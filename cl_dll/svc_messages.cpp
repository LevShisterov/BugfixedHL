/***
*
*	Copyright (c) 2012, AGHL.RU. All rights reserved.
*
****/
//
// Svc_messages.cpp
//
// Engine messages handlers
//

#include <string.h>

#include "hud.h"
#include "memory.h"
#include "parsemsg.h"
#include "cl_util.h"
#include "svc_messages.h"
#include "vgui_TeamFortressViewport.h"
#include "vgui_ScorePanel.h"
#include "pcre/pcre.h"

#define MAX_CMD_LINE	1024

cl_enginemessages_t pEngineMessages;
cvar_t *m_pCvarClLogMessages = 0;
cvar_t *m_pCvarClProtectBlock = 0;
cvar_t *m_pCvarClProtectAllow = 0;
cvar_t *m_pCvarClProtectBlockCvar = 0;
char com_token[1024];
const char *blockList = "^(exit|quit|bind|unbind|unbindall|kill|exec|alias|clear|motdfile|motd_write|writecfg|cd|developer|fps.+|rcon.*)$";
const char *blockListCvar = "^(rcon.*)$";

bool RegexMatch(const char *str, const char *regex)
{
	if (regex[0] == 0)
		return false;

	const char *error;
	int erroffset;
	int ovector[30];

	// Prepare regex
	pcre *re = pcre_compile(regex, PCRE_CASELESS, &error, &erroffset, NULL);
	if (!re)
	{
		if (gEngfuncs.Con_Printf)
		{
			gEngfuncs.Con_Printf("PCRE compilation failed at offset %d: %s\n", erroffset, error);
			gEngfuncs.Con_Printf("in regex: %s\n", regex);
		}
		return false;
	}

	// Try to match
	int rc = pcre_exec(re, NULL, str, strlen(str), 0, 0, ovector, sizeof(ovector));
	if (rc < 0)
		return false;	// No match

	return true;
}
bool IsCommandGood(const char *str)
{
	// Parse command into token
	char *ret = gEngfuncs.COM_ParseFile((char *)str, com_token);
	if (ret == NULL || com_token[0] == 0)
		return true;	// no tokens

	// Block our filter from hacking
	if (!_stricmp(com_token, m_pCvarClProtectBlock->name)) return false;
	if (!_stricmp(com_token, m_pCvarClProtectAllow->name)) return false;

	// Check command name against block lists and whole command line against allow list
	if ((RegexMatch(com_token, blockList) ||
		 RegexMatch(com_token, m_pCvarClProtectBlock->string)) &&
		!RegexMatch(str, m_pCvarClProtectAllow->string))
		return false;

	return true;
}
bool IsCvarGood(const char *str)
{
	if (str[0] == 0)
		return true;	// no cvar

	// Block our filter from getting
	if (!_stricmp(com_token, m_pCvarClProtectBlock->name)) return false;
	if (!_stricmp(com_token, m_pCvarClProtectAllow->name)) return false;

	// Check cvar name against block lists
	if (RegexMatch(str, blockListCvar) ||
		RegexMatch(str, m_pCvarClProtectBlockCvar->string))
		return false;

	return true;
}
bool SanitizeCommands(char *str)
{
	bool changed = false;
	char *text = str;
	char command[MAX_CMD_LINE];
	int i, quotes;
	int len = strlen(str);

	// Split string into commands and check them separately
	while (text[0] != 0)
	{
		// Find \n or ; splitter
		quotes = 0;
		for (i = 0; i < len; i++)
		{
			if (text[i] == '"') quotes++;
			if (!(quotes & 1) && text[i] == ';')
				break;
			if (text[i] == '\n')
				break;
		}
		if (i >= MAX_CMD_LINE)
			i = MAX_CMD_LINE;	// game engine behaviour
		strncpy(command, text, i);
		command[i] = 0;

		// Check command
		bool isGood = IsCommandGood(command);

		len -= i;
		if (!isGood)
		{
			gEngfuncs.Con_DPrintf("Server tried to execute bad command: %s\n", com_token);

			// Trash command, but leave the splitter
			strncpy(text, text + i, len);
			text[len] = 0;
			text++;
			changed = true;
		}
		else
		{
			text += i + 1;
		}
	}
	return changed;
}

void SvcPrint(void)
{
	static bool processingUserRow = false;

	BEGIN_READ(*g_EngineBuf, *g_EngineBufSize, *g_EngineReadPos);
	char *str = READ_STRING();

	if (!strncmp(str, "\"mp_timelimit\" changed to \"", 27) ||
		!strncmp(str, "\"amx_nextmap\" changed to \"", 26))
	{
		gHUD.m_Timer.DoResync();
	}
	else if (gViewPort && gViewPort->m_pScoreBoard && gViewPort->m_pScoreBoard->m_iStatusRequestState != STATUS_REQUEST_IDLE)
	{
		// Proccess status command answer
		if (str[0] == '#' && str[1] != 0 && str[2] != 0 && str[3] == ' ' ) // start of new player info row (or table header)
		{
			gViewPort->m_pScoreBoard->m_iStatusRequestState = STATUS_REQUEST_PROCESSING; // players info table started
			int slot = atoi(str + 1);
			if (slot > 0)
			{
				char *name = strchr(strchr(str + 2, ' '), '"');
				if (name != NULL)
				{
					name ++; // space
					char *userid = strchr(name, '"');
					if (userid != NULL)
					{
						userid += 2; // quote and space
						char *steamid = strchr(userid, ' ');
						if (steamid != NULL)
						{
							steamid++; // space
							char *steamidend = strchr(steamid, ' ');
							if (steamidend != NULL)
								*steamidend = 0;
							if (!strncmp(steamid, "STEAM_", 6) ||
								!strncmp(steamid, "VALVE_", 6))
								strncpy(g_PlayerSteamId[slot], steamid + 6, MAX_STEAMID); // cutout "STEAM_" or "VALVE_" start of the string
							else
								strncpy(g_PlayerSteamId[slot], steamid, MAX_STEAMID);
							g_PlayerSteamId[slot][MAX_STEAMID] = 0;
						}
					}
				}
			}
			if (strchr(str, '\n') == NULL)
				processingUserRow = true;
		}
		else if (processingUserRow)
		{
			if (strchr(str, '\n') != NULL) // skip till the end of the row (new line)
				processingUserRow = false;
		}
		else if (gViewPort->m_pScoreBoard->m_iStatusRequestState == STATUS_REQUEST_PROCESSING)
		{
			gViewPort->m_pScoreBoard->m_iStatusRequestState = STATUS_REQUEST_IDLE;
		}
		// Suppress status output
		*g_EngineReadPos += strlen(str) + 1;
		return;
	}
	else
	{
		// Clear cached steam id for left player
		int len = strlen(str);
		if (!strcmp(str + len - 9, " dropped\n"))
		{
			str[len - 9] = 0;
			gViewPort->GetAllPlayersInfo();
			for (int i = 1; i < MAX_PLAYERS; i++)
			{
				if (g_PlayerInfoList[i].name != NULL &&
					!strcmp(g_PlayerInfoList[i].name, str))
				{
					g_PlayerSteamId[i][0] = 0;
					break;
				}
			}
		}
	}

	pEngineMessages.pfnSvcPrint();
}
void SvcNewUserMsg(void)
{
	BEGIN_READ(*g_EngineBuf, *g_EngineBufSize, *g_EngineReadPos);
	int id = READ_BYTE();
	int len = READ_BYTE();
	char name[16];
	uint32_t value = READ_LONG();
	memcpy(name, &(value), 4);
	value = READ_LONG();
	memcpy(name + 4, &(value), 4);
	value = READ_LONG();
	memcpy(name + 8, &(value), 4);
	value = READ_LONG();
	memcpy(name + 12, &(value), 4);
	name[15] = 0;

	pEngineMessages.pfnSvcNewUserMsg();

	// Log user message to console
	if (m_pCvarClLogMessages && m_pCvarClLogMessages->value)
		gEngfuncs.Con_Printf("User Message: %d, %s, %d\n", id, name, len == 255 ? -1 : len);

	// Fix engine bug that leads to duplicate user message ids in user messages chain
	UserMessage *current = *g_pUserMessages;
	while (current != 0)
	{
		if (current->messageId == id && strcmp(current->messageName, name))
			current->messageId = 0;
		current = current->nextMessage;
	}
}
void SvcStuffText(void)
{
	BEGIN_READ(*g_EngineBuf, *g_EngineBufSize, *g_EngineReadPos);
	char *commands = READ_STRING();

	char str[1024];
	strncpy(str, commands, sizeof(str));
	str[sizeof(str) - 1] = 0;

	if (SanitizeCommands(str))
	{
		// Some commands were removed, put cleaned command line back to stream
		int l1 = strlen(commands);
		int l2 = strlen(str);
		if (l2 == 0)
		{
			// Suppress commands if they are all removed
			*g_EngineReadPos += l1 + 1;
			return;
		}
		int diff = l1 - l2;
		strncpy(commands + diff, str, l2);
		*g_EngineReadPos += diff;
	}

	pEngineMessages.pfnSvcStuffText();
}
void SvcSendCvarValue(void)
{
	BEGIN_READ(*g_EngineBuf, *g_EngineBufSize, *g_EngineReadPos);
	char *cvar = READ_STRING();

	// Check cvar
	bool isGood = IsCvarGood(cvar);
	if (!isGood)
	{
		gEngfuncs.Con_DPrintf("Server tried to request blocked cvar: %s\n", cvar);
		*g_EngineReadPos += strlen(cvar) + 1;
		// At now we will silently block the request
		return;
	}

	pEngineMessages.pfnSvcSendCvarValue();
}
void SvcSendCvarValue2(void)
{
	BEGIN_READ(*g_EngineBuf, *g_EngineBufSize, *g_EngineReadPos);
	long l = READ_LONG();
	char *cvar = READ_STRING();

	// Check cvar
	bool isGood = IsCvarGood(cvar);
	if (!isGood)
	{
		gEngfuncs.Con_DPrintf("Server tried to request blocked cvar: %s\n", cvar);
		*g_EngineReadPos += 4 + strlen(cvar) + 1;
		// At now we will silently block the request
		return;
	}

	pEngineMessages.pfnSvcSendCvarValue2();
}

void HookSvcMessages(void)
{
	memset(&pEngineMessages, 0, sizeof(cl_enginemessages_t));
	pEngineMessages.pfnSvcPrint = SvcPrint;
	pEngineMessages.pfnSvcNewUserMsg = SvcNewUserMsg;
	pEngineMessages.pfnSvcStuffText = SvcStuffText;
	pEngineMessages.pfnSvcSendCvarValue = SvcSendCvarValue;
	pEngineMessages.pfnSvcSendCvarValue2 = SvcSendCvarValue2;
	HookSvcMessages(&pEngineMessages);
}
void UnHookSvcMessages(void)
{
	UnHookSvcMessages(&pEngineMessages);
}

void DumpUserMessages(void)
{
	// Dump all user messages to console
	UserMessage *current = *g_pUserMessages;
	while (current != 0)
	{
		gEngfuncs.Con_Printf("User Message: %d, %s, %d\n", current->messageId, current->messageName, current->messageLen);
		current = current->nextMessage;
	}
}
void ProtectHelp(void)
{
	// Show protect info
	gEngfuncs.Con_Printf("cl_protect_* cvars are used to protect from slowhacking.\n");
	gEngfuncs.Con_Printf("By default following is blocked:\n");
	gEngfuncs.Con_Printf("Commands: %s\n", blockList);
	gEngfuncs.Con_Printf("Cvar requests: %s\n", blockListCvar);
}

// Registers cvars and commands
void SvcMessagesInit(void)
{
	m_pCvarClLogMessages = gEngfuncs.pfnRegisterVariable("cl_messages_log", "0", FCVAR_ARCHIVE);
	gEngfuncs.pfnAddCommand("cl_messages_dump", DumpUserMessages);

	m_pCvarClProtectBlock = gEngfuncs.pfnRegisterVariable("cl_protect_block", "", FCVAR_ARCHIVE);
	m_pCvarClProtectAllow = gEngfuncs.pfnRegisterVariable("cl_protect_allow", "", FCVAR_ARCHIVE);
	m_pCvarClProtectBlockCvar = gEngfuncs.pfnRegisterVariable("cl_protect_block_cvar", "", FCVAR_ARCHIVE);
	gEngfuncs.pfnAddCommand("cl_protect", ProtectHelp);
}
