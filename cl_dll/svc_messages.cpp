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
bool processingUserRow = false;

void SvcPrint(void)
{
	BEGIN_READ(*g_EngineBuf, *g_EngineBufSize, *g_EngineReadPos);
	char *str = READ_STRING();

	if (!strncmp(str, "\"mp_timelimit\" changed to \"", 27))
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
							char *colon = strchr(steamid, ':');
							if (!strncmp(steamid, "STEAM_", 6) &&
								(colon = strchr(steamid + 6, ':')) != NULL &&
								strchr(colon + 1, ':') != NULL) // lazy test for steamId
								strncpy(g_PlayerSteamId[slot], steamid + 6, MAX_STEAMID); // normal steamId - omit "STEAM_" start string
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

bool HookSvcMessages(void)
{
	memset(&pEngineMessages, 0, sizeof(cl_enginemessages_t));
	pEngineMessages.pfnSvcPrint = SvcPrint;
	return HookSvcMessages(&pEngineMessages);
}

bool UnHookSvcMessages(void)
{
	return UnHookSvcMessages(&pEngineMessages);
}
