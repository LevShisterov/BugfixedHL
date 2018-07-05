/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
//
// Hud_scores.cpp
//
// implementation of CHudScores class
//

#include "hud.h"
#include "cl_util.h"
#include "vgui_TeamFortressViewport.h"
#include "vgui_ScorePanel.h"


int CHudScores::Init(void)
{
	m_pCvarHudScores = CVAR_CREATE("hud_scores", "0", FCVAR_ARCHIVE);
	m_pCvarHudScoresPos = CVAR_CREATE("hud_scores_pos", "30 50", FCVAR_ARCHIVE);

	m_iFlags |= HUD_ACTIVE;

	gHUD.AddHudElem(this);

	return 1;
}

int CHudScores::VidInit(void)
{
	m_iOverLay = 0;
	m_flScoreBoardLastUpdated = 0;

	return 1;
}

int CHudScores::Draw (float flTime)
{
	if (m_pCvarHudScores->value < 1)
		return 1;

	// No Scoreboard in single-player
	if (gEngfuncs.GetMaxClients() <= 1)
		return 1;

	if (gViewPort && gViewPort->m_pScoreBoard)
	{
		// Update the Scoreboard
		if (m_flScoreBoardLastUpdated < gHUD.m_flTime)
		{
			gViewPort->m_pScoreBoard->Update();
			m_flScoreBoardLastUpdated = gHUD.m_flTime + 0.5;

			// Build list
			m_iLines = 0;
			for (int iRow = 0; iRow < gViewPort->m_pScoreBoard->m_iRows && m_iLines < m_pCvarHudScores->value; iRow++)
			{
				if (gViewPort->m_pScoreBoard->m_iIsATeam[iRow] == 1 && gHUD.m_Teamplay)
				{
					team_info_t* team_info = &g_TeamInfo[gViewPort->m_pScoreBoard->m_iSortedRows[iRow]];
					HudScoresData* data = &m_ScoresData[m_iLines];
					sprintf(data->szScore, "%-5i %s", team_info->frags, team_info->name);

					data->r = iTeamColors[team_info->teamnumber % iNumberOfTeamColors][0];
					data->g = iTeamColors[team_info->teamnumber % iNumberOfTeamColors][1];
					data->b = iTeamColors[team_info->teamnumber % iNumberOfTeamColors][2];

					m_iLines++;
				}
				else if (gViewPort->m_pScoreBoard->m_iIsATeam[iRow] == 0 && !gHUD.m_Teamplay)
				{
					hud_player_info_t* pl_info = &g_PlayerInfoList[gViewPort->m_pScoreBoard->m_iSortedRows[iRow]];
					extra_player_info_t* pl_info_extra = &g_PlayerExtraInfo[gViewPort->m_pScoreBoard->m_iSortedRows[iRow]];
					if (pl_info->spectator && pl_info_extra->frags == 0)
						continue;
					HudScoresData* data = &m_ScoresData[m_iLines];
					sprintf(data->szScore, "%-5i %s", pl_info_extra->frags, pl_info->name);

					data->r = iTeamColors[pl_info_extra->teamnumber % iNumberOfTeamColors][0];
					data->g = iTeamColors[pl_info_extra->teamnumber % iNumberOfTeamColors][1];
					data->b = iTeamColors[pl_info_extra->teamnumber % iNumberOfTeamColors][2];

					m_iLines++;
				}
			}
		}

		int xpos, ypos;
		xpos = 30;
		ypos = 50;
		sscanf(m_pCvarHudScoresPos->string, "%i %i", &xpos, &ypos);

		for (int iLine = 0; iLine < m_iLines && iLine < m_pCvarHudScores->value; iLine++)
		{
			HudScoresData* data = &m_ScoresData[iLine];

			int r, g, b;
			r = data->r;
			g = data->g;
			b = data->b;
			FillRGBA(xpos - 10, ypos + 2, m_iOverLay, gHUD.m_scrinfo.iCharHeight * 0.9, r, g, b, gHUD.m_Teamplay ? 20 : 10);

			ScaleColors(r, g, b, 135);

			const int ixposnew = AgDrawHudString(xpos, ypos, ScreenWidth, data->szScore, r, g, b);
			m_iOverLay = max(ixposnew - xpos + 20, m_iOverLay);
			ypos += gHUD.m_scrinfo.iCharHeight * 0.9;
		}
	}

	return 1;
}
