/***
*
*	Copyright (c) 2012, AGHL.RU. All rights reserved.
*
****/
//
// Memory.cpp
//
// Runtime memory searching/patching
//

#include <windows.h>
#include <psapi.h>

#include "memory.h"
#include "wrect.h"
#include "cl_dll.h"
#include "cvardef.h"

#define MAX_PATTERN 64

size_t g_EngineModuleBase = 0, g_EngineModuleSize = 0, g_EngineModuleEnd = 0;
size_t g_SvcMessagesTable = 0;
size_t g_FpsBugPlace = 0;
uint8_t g_FpsBugPlaceBackup[16];
double *g_flFrameTime;
double g_flFrameTimeReminder = 0;
cvar_t *m_pCvarEngineFixFpsBug = 0;

void **g_EngineBuf = 0;
int *g_EngineBufSize = 0;
int *g_EngineReadPos = 0;
UserMessage **g_pUserMessages = 0;


bool GetModuleAddress(const char *moduleName, size_t &moduleBase, size_t &moduleSize)
{
	HANDLE hProcess = GetCurrentProcess();
	HMODULE hModuleDll = GetModuleHandle(moduleName);
	if (!hProcess || !hModuleDll) return false;
	MODULEINFO moduleInfo;
	GetModuleInformation(hProcess, hModuleDll, &moduleInfo, sizeof(moduleInfo));
	moduleBase = (size_t)moduleInfo.lpBaseOfDll;
	moduleSize = (size_t)moduleInfo.SizeOfImage;
	return true;
}

void GetEngineModuleAddress(void)
{
	if (!GetModuleAddress("hw.dll", g_EngineModuleBase, g_EngineModuleSize) &&	// Try Hardware engine
		!GetModuleAddress("sw.dll", g_EngineModuleBase, g_EngineModuleSize) &&	// Try Software engine
		!GetModuleAddress("hl.exe", g_EngineModuleBase, g_EngineModuleSize))	// Try Encrypted engine
		return;
	g_EngineModuleEnd = g_EngineModuleBase + g_EngineModuleSize - 1;
}

size_t ConvertHexString(const char* srcHexString, unsigned char *outBuffer, size_t bufferSize)
{
	unsigned char *in = (unsigned char *)srcHexString;
	unsigned char *out = outBuffer;
	unsigned char *end = outBuffer + bufferSize;
	bool low = false;
	uint8_t byte = 0;
	while (*in && out < end)
	{
		if (*in >= '0' && *in <= '9') { byte |= *in - '0'; }
		else if (*in >= 'A' && *in <= 'F') { byte |= *in - 'A' + 10; }
		else if (*in >= 'a' && *in <= 'f') { byte |= *in - 'a' + 10; }
		else if (*in == ' ') { in++; continue; }

		if (!low)
		{
			byte = byte << 4;
			in++;
			low = true;
			continue;
		}
		low = false;

		*out = byte;
		byte = 0;

		in++;
		out++;
	}
	return out - outBuffer;
}

size_t MemoryFindForward(size_t start, size_t end, const unsigned char* pattern, const unsigned char *mask, size_t pattern_len)
{
	unsigned char *cend = (unsigned char*)(end - pattern_len + 1);
	unsigned char *current = (unsigned char*)(start);

	size_t i;
	if (mask)
	{
		while (current < cend)
		{
			for (i = 0; i < pattern_len; i++)
			{
				if ((current[i] & mask[i]) != (pattern[i] & mask[i]))
					break;
			}

			if (i == pattern_len)
				return (size_t)(void*)current;

			current++;
		}
	}
	else
	{
		while (current < cend)
		{
			for (i = 0; i < pattern_len; i++)
			{
				if (current[i] != pattern[i])
					break;
			}

			if (i == pattern_len)
				return (size_t)(void*)current;

			current++;
		}
	}

	return NULL;
}

size_t MemoryFindForward(size_t start, size_t end, const char* pattern, const char *mask)
{
	unsigned char p[MAX_PATTERN];
	unsigned char m[MAX_PATTERN];
	size_t pl = ConvertHexString(pattern, p, sizeof(p));
	size_t ml = ConvertHexString(mask, m, sizeof(m));
	return MemoryFindForward(start, end, p, m, pl >= ml ? pl : ml);
}

size_t MemoryFindBackward(size_t start, size_t end, const unsigned char* pattern, const unsigned char *mask, size_t pattern_len)
{
	// Ensure start is high then the end
	if (start < end)
	{
		size_t reverse = end;
		end = start;
		start = reverse;
	}

	unsigned char *cend = (unsigned char*)(end);
	unsigned char *current = (unsigned char*)(start - pattern_len);

	size_t i;
	if (mask)
	{
		while (current >= cend)
		{
			for (i = 0; i < pattern_len; i++)
			{
				if ((current[i] & mask[i]) != (pattern[i] & mask[i]))
					break;
			}

			if (i == pattern_len)
				return (size_t)(void*)current;

			current--;
		}
	}
	else
	{
		while (current >= cend)
		{
			for (i = 0; i < pattern_len; i++)
			{
				if (current[i] != pattern[i])
					break;
			}

			if (i == pattern_len)
				return (size_t)(void*)current;

			current--;
		}
	}

	return NULL;
}

size_t MemoryFindBackward(size_t start, size_t end, const char* pattern, const char *mask)
{
	unsigned char p[MAX_PATTERN];
	unsigned char m[MAX_PATTERN];
	size_t pl = ConvertHexString(pattern, p, sizeof(p));
	size_t ml = ConvertHexString(mask, m, sizeof(m));
	return MemoryFindBackward(start, end, p, m, pl >= ml ? pl : ml);
}

uint32_t HookDWord(size_t *origAddr, uint32_t newDWord)
{
	DWORD oldProtect;
	uint32_t origDWord = *origAddr;
	VirtualProtect(origAddr, 4, PAGE_EXECUTE_READWRITE, &oldProtect);
	*origAddr = newDWord;
	VirtualProtect(origAddr, 4, oldProtect, &oldProtect);
	return origDWord;
}

void ExchangeMemoryBytes(size_t *origAddr, size_t *dataAddr, uint32_t size)
{
	DWORD oldProtect;
	VirtualProtect(origAddr, size, PAGE_EXECUTE_READWRITE, &oldProtect);
	unsigned char data[MAX_PATTERN];
	int32_t iSize = size;
	while (iSize > 0)
	{
		size_t s = iSize <= MAX_PATTERN ? iSize : MAX_PATTERN;
		memcpy(data, origAddr, s);
		memcpy((void *)origAddr, (void *)dataAddr, s);
		memcpy((void *)dataAddr, data, s);
		iSize -= MAX_PATTERN;
	}
	VirtualProtect(origAddr, size, oldProtect, &oldProtect);
}

void FindSvcMessagesTable(void)
{
	// Search for "svc_bad" and futher engine messages strings
	size_t svc_bad, svc_nop, svc_disconnect;
	const unsigned char data1[] = "svc_bad";
	svc_bad = MemoryFindForward(g_EngineModuleBase, g_EngineModuleEnd, data1, NULL, sizeof(data1) - 1);
	if (!svc_bad) return;
	const unsigned char data2[] = "svc_nop";
	svc_nop = MemoryFindForward(svc_bad, g_EngineModuleEnd, data2, NULL, sizeof(data2) - 1);
	if (!svc_nop) return;
	const unsigned char data3[] = "svc_disconnect";
	svc_disconnect = MemoryFindForward(svc_nop, g_EngineModuleEnd, data3, NULL, sizeof(data3) - 1);
	if (!svc_disconnect) return;

	// Form pattern to search for engine messages functions table
	char data4[12 * 3 + 4];
	*((uint32_t*)data4 + 0) = 0;
	*((uint32_t*)data4 + 1) = svc_bad;
	*((uint32_t*)data4 + 3) = 1;
	*((uint32_t*)data4 + 4) = svc_nop;
	*((uint32_t*)data4 + 6) = 2;
	*((uint32_t*)data4 + 7) = svc_disconnect;
	*((uint32_t*)data4 + 9) = 3;
	const char mask4[] = "FFFFFFFFFFFFFFFF00000000 FFFFFFFFFFFFFFFF00000000 FFFFFFFFFFFFFFFF00000000 FFFFFFFF";
	unsigned char m[MAX_PATTERN];
	ConvertHexString(mask4, m, sizeof(m));
	// We search backward first - it should be there and near
	g_SvcMessagesTable = MemoryFindBackward(svc_bad, g_EngineModuleBase, (unsigned char*)data4, m, sizeof(data4) - 1);
	if (!g_SvcMessagesTable)
		g_SvcMessagesTable = MemoryFindForward(svc_bad, g_EngineModuleEnd, (unsigned char*)data4, (unsigned char*)mask4, sizeof(data4) - 1);
}
void FindEngineMessagesBufferVariables(void)
{
	// Find and get engine messages buffer variables
	const char data2[] = "8B0D30E6F8038B1528E6F8038BC583C1F825FF00000083C208505152E8";
	const char mask2[] = "FFFF00000000FFFF00000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF";
	size_t addr2 = MemoryFindForward(g_EngineModuleBase, g_EngineModuleEnd, data2, mask2);
	if (!addr2) return;

	g_EngineBufSize = (int *)*(size_t *)(((uint8_t *)addr2) + 2);
	g_EngineBuf = (void **)*(size_t *)(((uint8_t *)addr2) + 8);

	const char data3[] = "8B0D30E6F8038B15283D4F048BE82BCAB8ABAAAA2AF7E98BCAC1E91F03D18BC2";
	const char mask3[] = "FFFF00000000FFFF00000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF";
	size_t addr3 = MemoryFindForward(g_EngineModuleBase, g_EngineModuleEnd, data3, mask3);
	if (!addr3) return;

	//g_EngineBufSize == (int *)*(size_t *)(((uint8_t *)addr3) + 2);
	g_EngineReadPos = (int *)*(size_t *)(((uint8_t *)addr3) + 8);
}
void FindUserMessagesEntry(void)
{
	// Find and get engine messages buffer variables
	const char data1[] = "81FB000100000F8D1B0100008B3574FF6C0385F6740B";
	const char mask1[] = "FFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFF";
	size_t addr1 = MemoryFindForward(g_EngineModuleBase, g_EngineModuleEnd, data1, mask1);
	if (!addr1) return;

	g_pUserMessages = (UserMessage **)*(size_t *)(((uint8_t *)addr1) + 14);
}

// Hooks requested functions
bool HookSvcMessages(cl_enginemessages_t *pEngineMessages)
{
	if (!g_EngineModuleBase) GetEngineModuleAddress();
	if (!g_EngineModuleBase) return false;

	if (!g_SvcMessagesTable) FindSvcMessagesTable();
	if (!g_SvcMessagesTable) return false;

	if (!g_EngineBufSize || !g_EngineModuleSize || !g_EngineModuleEnd) FindEngineMessagesBufferVariables();
	if (!g_EngineBufSize || !g_EngineModuleSize || !g_EngineModuleEnd) return false;

	if (!g_pUserMessages) FindUserMessagesEntry();
	if (!g_pUserMessages) return false;

	int len = sizeof(cl_enginemessages_t) / 4;
	for (int i = 0; i < len; i++)
	{
		if (((uint32_t *)pEngineMessages)[i] == NULL) continue;
		size_t funcAddr = ((uint32_t *)pEngineMessages)[i];
		size_t addr = g_SvcMessagesTable + i * 12 + 8;
		size_t oldAddr = HookDWord((size_t*)addr, funcAddr);
		((uint32_t *)pEngineMessages)[i] = oldAddr;
	}

	return true;
}

// Unhooks requested functions
bool UnHookSvcMessages(cl_enginemessages_t *pEngineMessages)
{
	// We just do same exchange for functions addresses
	return HookSvcMessages(pEngineMessages);
}

void __stdcall FpsBugFix(int a1, int64_t *a2)
{
	if (!m_pCvarEngineFixFpsBug || m_pCvarEngineFixFpsBug->value)
	{
		g_flFrameTimeReminder += *g_flFrameTime * 1000 - a1;
		if (g_flFrameTimeReminder > 1.0)
		{
			g_flFrameTimeReminder--;
			a1++;
		}
	}
	*a2 = a1;
	*((double *)(a2 + 1)) = a1;
}

void PatchEngine(void)
{
	if (!g_EngineModuleBase) GetEngineModuleAddress();
	if (!g_EngineModuleBase) return;

	// Find place where FPS bug happen
	const char data1[] = "DD052834FA03 DC0DE8986603 83C408 E8D87A1000 89442424DB442424 DD5C242C DD05";
	const char mask1[] = "FFFF00000000 FFFF00000000 FFFFFF FF00000000 FFFFFFFFFFFFFFFF FFFFFFFF FFFF";
	size_t addr1 = MemoryFindForward(g_EngineModuleBase, g_EngineModuleEnd, data1, mask1);
	if (addr1)
	{
		g_FpsBugPlace = addr1;
		g_flFrameTime = (double *)*(size_t *)(((uint8_t *)addr1) + 2);

		// Patch FPS bug: hook correcting function there
		const char data2[] = "8D542424 52 50 E8FFFFFFFF 90";
		ConvertHexString(data2, g_FpsBugPlaceBackup, sizeof(g_FpsBugPlaceBackup));
		size_t offset = (size_t)FpsBugFix - (g_FpsBugPlace + 20 + 11);
		*(size_t*)(&(g_FpsBugPlaceBackup[7])) = offset;
		ExchangeMemoryBytes((size_t *)(g_FpsBugPlace + 20), (size_t *)g_FpsBugPlaceBackup, 12);
	}
}

void UnPatchEngine(void)
{
	if (!g_EngineModuleBase) GetEngineModuleAddress();
	if (!g_EngineModuleBase) return;

	// Restore FPS engine block
	if (g_FpsBugPlace)
	{
		ExchangeMemoryBytes((size_t *)(g_FpsBugPlace + 20), (size_t *)g_FpsBugPlaceBackup, 12);
		g_FpsBugPlace = 0;
	}
}

void MemoryPatcherInit(void)
{
	m_pCvarEngineFixFpsBug = gEngfuncs.pfnRegisterVariable("engine_fix_fpsbug", "1", FCVAR_ARCHIVE);
};
