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

cl_enginemessages_t pEngineMessages;

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
		// Supress status output
		*g_EngineReadPos += strlen(str) + 1;
		return;
	}
	else
	{
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

	// Fix engine bug that leads to duplicate user message ids in user messages chain
	UserMessage *current = *g_pUserMessages;
	while (current != 0)
	{
		if (current->messageId == id && strcmp(current->messageName, name))
			current->messageId = 0;
		current = current->nextMessage;
	}
}

bool HookSvcMessages(void)
{
	memset(&pEngineMessages, 0, sizeof(cl_enginemessages_t));
	pEngineMessages.pfnSvcPrint = SvcPrint;
	pEngineMessages.pfnSvcNewUserMsg = SvcNewUserMsg;
	return HookSvcMessages(&pEngineMessages);
}

bool UnHookSvcMessages(void)
{
	return UnHookSvcMessages(&pEngineMessages);
}
