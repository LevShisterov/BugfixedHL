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

size_t engineModuleBase = 0, engineModuleSize = 0, engineModuleEnd = 0;
size_t g_svc_messages_table = 0;
void **g_EngineBuf;
int *g_EngineBufSize;
int *g_EngineReadPos;


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

uint32_t HookDWord(size_t *origAddr, uint32_t newDWord)
{
	DWORD oldProtect;
	uint32_t origDWord = *origAddr;
	VirtualProtect(origAddr, 8, PAGE_EXECUTE_READWRITE, &oldProtect);
	*origAddr = newDWord;
	VirtualProtect(origAddr, 8, oldProtect, &oldProtect);
	return origDWord;
}

void GetEngineModuleAddress(void)
{
	if (!GetModuleAddress("hw.dll", engineModuleBase, engineModuleSize) &&	// Try Hardware engine
		!GetModuleAddress("sw.dll", engineModuleBase, engineModuleSize) &&	// Try Software engine
		!GetModuleAddress("hl.exe", engineModuleBase, engineModuleSize))	// Try Encrypted engine
		return;
	engineModuleEnd = engineModuleBase + engineModuleSize - 1;
}

void FindSvcMessagesTable(void)
{
	if (!engineModuleBase) GetEngineModuleAddress();
	if (!engineModuleBase) return;

	// Search for "svc_print" and futher engine messages strings
	size_t svc_bad, svc_nop, svc_disconnect;
	const char data1[] = "svc_bad";
	svc_bad = MemoryFindForward(engineModuleBase, engineModuleEnd, (unsigned char*)data1, NULL, sizeof(data1) - 1);
	if (!svc_bad) return;
	const char data2[] = "svc_nop";
	svc_nop = MemoryFindForward(svc_bad, engineModuleEnd, (unsigned char*)data2, NULL, sizeof(data2) - 1);
	if (!svc_nop) return;
	const char data3[] = "svc_disconnect";
	svc_disconnect = MemoryFindForward(svc_nop, engineModuleEnd, (unsigned char*)data3, NULL, sizeof(data3) - 1);
	if (!svc_disconnect) return;

	// Form pattern to search for engine messages functions table
	char data4[12 * 3];
	*((uint32_t*)data4 + 0) = svc_bad;
	*((uint32_t*)data4 + 2) = 1;
	*((uint32_t*)data4 + 3) = svc_nop;
	*((uint32_t*)data4 + 5) = 2;
	*((uint32_t*)data4 + 6) = svc_disconnect;
	*((uint32_t*)data4 + 8) = 3;
	const char mask4[] = "\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF" "\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF" "\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF";
	g_svc_messages_table = MemoryFindBackward(svc_bad, engineModuleBase, (unsigned char*)data4, (unsigned char*)mask4, sizeof(data4) - 1);
	if (g_svc_messages_table) return;
	g_svc_messages_table = MemoryFindForward(svc_bad, engineModuleEnd, (unsigned char*)data4, (unsigned char*)mask4, sizeof(data4) - 1);
}

bool HookSvcMessages(cl_enginemessages_t *pEngineMessages)
{
	if (!g_svc_messages_table) FindSvcMessagesTable();
	if (!g_svc_messages_table) return false;

	// Find and get engine messages buffer variables
	const char data2[] = "\x8B\x0D\x30\xE6\xF8\x03\x8B\x15\x28\xE6\xF8\x03\x8B\xC5\x83\xC1\xF8\x25\xFF\x00\x00\x00\x83\xC2\x08\x50\x51\x52\xE8";
	const char mask2[] = "\xFF\xFF\x00\x00\x00\x00\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF";
	size_t addr2 = MemoryFindForward(engineModuleBase, engineModuleEnd, (unsigned char*)data2, (unsigned char*)mask2, sizeof(data2) - 1);
	if (!addr2) return false;

	g_EngineBufSize = (int *)*(size_t *)(((uint8_t *)addr2) + 2);
	g_EngineBuf = (void **)*(size_t *)(((uint8_t *)addr2) + 8);

	const char data3[] = "\x8B\x0D\x30\xE6\xF8\x03\x8B\x15\x28\x3D\x4F\x04\x8B\xE8\x2B\xCA\xB8\xAB\xAA\xAA\x2A\xF7\xE9\x8B\xCA\xC1\xE9\x1F\x03\xD1\x8B\xC2";
	const char mask3[] = "\xFF\xFF\x00\x00\x00\x00\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF";
	size_t addr3 = MemoryFindForward(engineModuleBase, engineModuleEnd, (unsigned char*)data3, (unsigned char*)mask3, sizeof(data3) - 1);
	if (!addr3) return false;

	//g_EngineBufSize == (int *)*(size_t *)(((uint8_t *)addr3) + 2);
	g_EngineReadPos = (int *)*(size_t *)(((uint8_t *)addr3) + 8);

	// Hook requested functions
	int len = sizeof(cl_enginemessages_t) / 4;
	for (int i = 0; i < len; i++)
	{
		if (((uint32_t *)pEngineMessages)[i] == NULL) continue;
		size_t funcAddr = ((uint32_t *)pEngineMessages)[i];
		size_t addr = g_svc_messages_table  + (i * 3 + 1) * 4;
		size_t oldAddr = HookDWord((size_t*)addr, funcAddr);
		((uint32_t *)pEngineMessages)[i] = oldAddr;
	}

	return true;
}

bool UnHookSvcMessages(cl_enginemessages_t *pEngineMessages)
{
	if (!g_svc_messages_table) FindSvcMessagesTable();
	if (!g_svc_messages_table) return false;

	// UnHook requested functions
	int len = sizeof(cl_enginemessages_t) / 4;
	for (int i = 0; i < len; i++)
	{
		if (((uint32_t *)pEngineMessages)[i] == NULL) continue;
		size_t funcAddr = ((uint32_t *)pEngineMessages)[i];
		size_t addr = g_svc_messages_table  + (i * 3 + 1) * 4;
		size_t oldAddr = HookDWord((size_t*)addr, funcAddr);
		((uint32_t *)pEngineMessages)[i] = oldAddr;
	}

	return true;
}
