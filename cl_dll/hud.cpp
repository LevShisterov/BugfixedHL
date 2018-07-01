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
// hud.cpp
//
// implementation of CHud class
//

#include "hud.h"
#include "cl_util.h"
#include <string.h>
#include <stdio.h>
#include "parsemsg.h"
#include "hud_servers.h"
#include "vgui_int.h"
#include "vgui_TeamFortressViewport.h"
#include "GameStudioModelRenderer.h"

#include "demo.h"
#include "demo_api.h"
#include "vgui_scorepanel.h"
#include "appversion.h"


float g_ColorBlue[3]	= { 0.6f, 0.8f, 1.0f };
float g_ColorRed[3]		= { 1.0f, 0.25f, 0.25f };
float g_ColorGreen[3]	= { 0.6f, 1.0f, 0.6f };
float g_ColorYellow[3]	= { 1.0f, 0.7f, 0.0f };
float g_ColorGrey[3]	= { 0.8f, 0.8f, 0.8f };

float *GetClientTeamColor(int clientIndex)
{
	switch (g_PlayerExtraInfo[clientIndex].teamnumber)
	{
		case 0: return NULL;
		case 1: return g_ColorBlue;
		case 2: return g_ColorRed;
		case 3: return g_ColorYellow;
		case 4: return g_ColorGreen;

		default: return g_ColorGrey;
	}
}

int g_iColorsCodes[10][3] = 
{
	{ 0xFF, 0xAA, 0x00 },	// ^0 orange
	{ 0xFF, 0x00, 0x00 },	// ^1 red
	{ 0x00, 0xFF, 0x00 },	// ^2 green
	{ 0xFF, 0xFF, 0x00 },	// ^3 yellow
	{ 0x00, 0x00, 0xFF },	// ^4 blue
	{ 0x00, 0xFF, 0xFF },	// ^5 cyan
	{ 0xFF, 0x00, 0xFF },	// ^6 magenta
	{ 0x88, 0x88, 0x88 },	// ^7 grey
	{ 0xFF, 0xFF, 0xFF },	// ^8 white
							// ^9 con_color
};

class CHLVoiceStatusHelper : public IVoiceStatusHelper
{
public:
	virtual void GetPlayerTextColor(int entindex, int color[3])
	{
		color[0] = color[1] = color[2] = 255;

		if( entindex >= 0 && entindex < sizeof(g_PlayerExtraInfo)/sizeof(g_PlayerExtraInfo[0]) )
		{
			int iTeam = g_PlayerExtraInfo[entindex].teamnumber;

			if ( iTeam < 0 )
			{
				iTeam = 0;
			}

			iTeam = iTeam % iNumberOfTeamColors;

			color[0] = iTeamColors[iTeam][0];
			color[1] = iTeamColors[iTeam][1];
			color[2] = iTeamColors[iTeam][2];
		}
	}

	virtual void UpdateCursorState()
	{
		gViewPort->UpdateCursorState();
	}

	virtual int	GetAckIconHeight()
	{
		return ScreenHeight - gHUD.m_iFontHeight*3 - 6;
	}

	virtual bool			CanShowSpeakerLabels()
	{
		if( gViewPort && gViewPort->m_pScoreBoard )
			return !gViewPort->m_pScoreBoard->isVisible();
		else
			return false;
	}
};
static CHLVoiceStatusHelper g_VoiceStatusHelper;


extern client_sprite_t *GetSpriteList(client_sprite_t *pList, const char *psz, int iRes, int iCount);

extern cvar_t *sensitivity;
cvar_t *cl_lw = NULL;

void ShutdownInput (void);

//DECLARE_MESSAGE(m_Logo, Logo)
int __MsgFunc_Logo(const char *pszName, int iSize, void *pbuf)
{
	return gHUD.MsgFunc_Logo(pszName, iSize, pbuf );
}

//DECLARE_MESSAGE(m_Logo, Logo)
int __MsgFunc_ResetHUD(const char *pszName, int iSize, void *pbuf)
{
	return gHUD.MsgFunc_ResetHUD(pszName, iSize, pbuf );
}

int __MsgFunc_InitHUD(const char *pszName, int iSize, void *pbuf)
{
	gHUD.MsgFunc_InitHUD( pszName, iSize, pbuf );
	return 1;
}

int __MsgFunc_ViewMode(const char *pszName, int iSize, void *pbuf)
{
	gHUD.MsgFunc_ViewMode( pszName, iSize, pbuf );
	return 1;
}

int __MsgFunc_SetFOV(const char *pszName, int iSize, void *pbuf)
{
	return gHUD.MsgFunc_SetFOV( pszName, iSize, pbuf );
}

int __MsgFunc_Concuss(const char *pszName, int iSize, void *pbuf)
{
	return gHUD.MsgFunc_Concuss( pszName, iSize, pbuf );
}

int __MsgFunc_GameMode(const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_GameMode( pszName, iSize, pbuf );
}

void __CmdFunc_About(void)
{
	char *ver = APP_VERSION;
	ConsolePrint("Fixed and improved HLSDK client.dll.\n");
	ConsolePrint("File version: ");
	ConsolePrint(ver);
	ConsolePrint(".\n");
	ConsolePrint("Visit http://aghl.ru/forum for more info on this dll.\n");
}

// TFFree Command Menu
void __CmdFunc_OpenCommandMenu(void)
{
	if ( gViewPort )
	{
		gViewPort->ShowCommandMenu( gViewPort->m_StandardMenu );
	}
}

// TFC "special" command
void __CmdFunc_InputPlayerSpecial(void)
{
	if ( gViewPort )
	{
		gViewPort->InputPlayerSpecial();
	}
}

void __CmdFunc_CloseCommandMenu(void)
{
	if ( gViewPort )
	{
		gViewPort->InputSignalHideCommandMenu();
	}
}

void __CmdFunc_ForceCloseCommandMenu( void )
{
	if ( gViewPort )
	{
		gViewPort->HideCommandMenu();
	}
}

void __CmdFunc_ToggleServerBrowser( void )
{
	if ( gViewPort )
	{
		gViewPort->ToggleServerBrowser();
	}
}

void __CmdFunc_ForceModel(void)
{
	g_StudioRenderer.ForceModelCommand();
}

void __CmdFunc_ForceColors(void)
{
	g_StudioRenderer.ForceColorsCommand();
}

void __CmdFunc_CustomTimer(void)
{
	gHUD.m_Timer.CustomTimerCommand();
}

// TFFree Command Menu Message Handlers
int __MsgFunc_ValClass(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_ValClass( pszName, iSize, pbuf );
	return 0;
}

int __MsgFunc_TeamNames(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_TeamNames( pszName, iSize, pbuf );
	return 0;
}

int __MsgFunc_Feign(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_Feign( pszName, iSize, pbuf );
	return 0;
}

int __MsgFunc_Detpack(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_Detpack( pszName, iSize, pbuf );
	return 0;
}

int __MsgFunc_VGUIMenu(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_VGUIMenu( pszName, iSize, pbuf );
	return 0;
}

int __MsgFunc_MOTD(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_MOTD( pszName, iSize, pbuf );
	return 0;
}

int __MsgFunc_BuildSt(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_BuildSt( pszName, iSize, pbuf );
	return 0;
}

int __MsgFunc_RandomPC(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_RandomPC( pszName, iSize, pbuf );
	return 0;
}
 
int __MsgFunc_ServerName(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_ServerName( pszName, iSize, pbuf );
	return 0;
}

int __MsgFunc_ScoreInfo(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_ScoreInfo( pszName, iSize, pbuf );
	return 0;
}

int __MsgFunc_TeamScore(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_TeamScore( pszName, iSize, pbuf );
	return 0;
}

int __MsgFunc_TeamInfo(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_TeamInfo( pszName, iSize, pbuf );
	return 0;
}

int __MsgFunc_Spectator(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_Spectator( pszName, iSize, pbuf );
	return 0;
}

int __MsgFunc_AllowSpec(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_AllowSpec( pszName, iSize, pbuf );
	return 0;
}

// This is called every time the DLL is loaded
void CHud :: Init( void )
{
	HOOK_MESSAGE( Logo );
	HOOK_MESSAGE( ResetHUD );
	HOOK_MESSAGE( GameMode );
	HOOK_MESSAGE( InitHUD );
	HOOK_MESSAGE( ViewMode );
	HOOK_MESSAGE( SetFOV );
	HOOK_MESSAGE( Concuss );

	HOOK_COMMAND( "about", About );

	// TFFree CommandMenu
	HOOK_COMMAND( "+commandmenu", OpenCommandMenu );
	HOOK_COMMAND( "-commandmenu", CloseCommandMenu );
	HOOK_COMMAND( "ForceCloseCommandMenu", ForceCloseCommandMenu );
	HOOK_COMMAND( "special", InputPlayerSpecial );
	HOOK_COMMAND( "togglebrowser", ToggleServerBrowser );
	HOOK_COMMAND( "forcemodel", ForceModel );
	HOOK_COMMAND( "forcecolors", ForceColors );
	HOOK_COMMAND( "customtimer", CustomTimer );

	HOOK_MESSAGE( ValClass );
	HOOK_MESSAGE( TeamNames );
	HOOK_MESSAGE( Feign );
	HOOK_MESSAGE( Detpack );
	HOOK_MESSAGE( MOTD );
	HOOK_MESSAGE( BuildSt );
	HOOK_MESSAGE( RandomPC );
	HOOK_MESSAGE( ServerName );
	HOOK_MESSAGE( ScoreInfo );
	HOOK_MESSAGE( TeamScore );
	HOOK_MESSAGE( TeamInfo );

	HOOK_MESSAGE( Spectator );
	HOOK_MESSAGE( AllowSpec );

	// VGUI Menus
	HOOK_MESSAGE( VGUIMenu );

	CVAR_CREATE( "hud_classautokill", "1", FCVAR_ARCHIVE );		// controls whether or not to suicide immediately on TF class switch
	CVAR_CREATE( "hud_takesshots", "0", FCVAR_ARCHIVE );		// controls whether or not to automatically take screenshots at the end of a round
	CVAR_CREATE( "zoom_sensitivity_ratio", "1.2", 0 );
	CVAR_CREATE( "cl_forceenemymodels", "", FCVAR_ARCHIVE );
	CVAR_CREATE( "cl_forceenemycolors", "", FCVAR_ARCHIVE );
	CVAR_CREATE( "cl_forceteammatesmodel", "", FCVAR_ARCHIVE );
	CVAR_CREATE( "cl_forceteammatescolors", "", FCVAR_ARCHIVE );
	m_pCvarBunnyHop = CVAR_CREATE( "cl_bunnyhop", "1", 0 );		// controls client-side bunnyhop enabling
	CVAR_CREATE( "cl_autowepswitch", "1", FCVAR_ARCHIVE | FCVAR_USERINFO );		// controls autoswitching to best weapon on pickup

	default_fov = CVAR_CREATE( "default_fov", "90", 0 );
	m_pCvarStealMouse = CVAR_CREATE( "hud_capturemouse", "1", FCVAR_ARCHIVE );
	m_pCvarDraw = CVAR_CREATE( "hud_draw", "1", FCVAR_ARCHIVE );
	m_pCvarDim = CVAR_CREATE( "hud_dim", "1", FCVAR_ARCHIVE );
	m_pCvarColor = CVAR_CREATE( "hud_color", "255 160 0", FCVAR_ARCHIVE );
	m_pCvarColor1 = CVAR_CREATE( "hud_color1", "0 255 0", FCVAR_ARCHIVE );
	m_pCvarColor2 = CVAR_CREATE( "hud_color2", "255 160 0", FCVAR_ARCHIVE );
	m_pCvarColor3 = CVAR_CREATE( "hud_color3", "255 96 0", FCVAR_ARCHIVE );
	m_pCvarShowNextmap = CVAR_CREATE( "hud_shownextmapinscore", "1", FCVAR_ARCHIVE );	// controls whether or not to show nextmap in scoreboard table
	m_pCvarShowLoss = CVAR_CREATE( "hud_showlossinscore", "1", FCVAR_ARCHIVE );	// controls whether or not to show loss in scoreboard table
	m_pCvarShowSteamId = CVAR_CREATE( "hud_showsteamidinscore", "1", FCVAR_ARCHIVE );	// controls whether or not to show SteamId in scoreboard table
	m_pCvarColorText = CVAR_CREATE( "hud_colortext", "1", FCVAR_ARCHIVE );
	m_pCvarRDynamicEntLight = CVAR_CREATE("r_dynamic_ent_light", "1", FCVAR_ARCHIVE);

	cl_lw = gEngfuncs.pfnGetCvarPointer( "cl_lw" );

	m_iLogo = 0;
	m_iFOV = 0;
	m_pSpriteList = NULL;

	// Clear any old HUD list
	if ( m_pHudList )
	{
		HUDLIST *pList;
		while ( m_pHudList )
		{
			pList = m_pHudList;
			m_pHudList = m_pHudList->pNext;
			free( pList );
		}
		m_pHudList = NULL;
	}

	// In case we get messages before the first update -- time will be valid
	m_flTime = 1.0;

	m_hudColor.Set(255, 160, 0);
	m_hudColor1.Set(0, 255, 0);
	m_hudColor2.Set(255, 160, 0);
	m_hudColor3.Set(255, 96, 0);

	m_Ammo.Init();
	m_Health.Init();
	m_SayText.Init();
	m_Spectator.Init();
	m_Geiger.Init();
	m_Train.Init();
	m_Battery.Init();
	m_Flash.Init();
	m_Message.Init();
	m_StatusBar.Init();
	m_DeathNotice.Init();
	m_AmmoSecondary.Init();
	m_TextMessage.Init();
	m_StatusIcons.Init();
	m_Timer.Init();
	m_Scores.Init();

	if (g_iIsAg)
	{
		m_Global.Init();
		m_Countdown.Init();
		m_CTF.Init();
		m_Location.Init();
		m_Longjump.Init();
		m_Nextmap.Init();
		m_PlayerId.Init();
		m_Settings.Init();
		m_SuddenDeath.Init();
		m_Timeout.Init();
		m_Vote.Init();
	}

	GetClientVoiceMgr()->Init(&g_VoiceStatusHelper, (vgui::Panel**)&gViewPort);

	m_Menu.Init();
	
	ServersInit();

	MsgFunc_ResetHUD(0, 0, NULL );
}

// CHud destructor
// cleans up memory allocated for m_rg* arrays
CHud :: ~CHud()
{
	delete [] m_rghSprites;
	delete [] m_rgrcRects;
	delete [] m_rgszSpriteNames;

	if ( m_pHudList )
	{
		HUDLIST *pList;
		while ( m_pHudList )
		{
			pList = m_pHudList;
			m_pHudList = m_pHudList->pNext;
			free( pList );
		}
		m_pHudList = NULL;
	}

	CharWidths* cur = m_CharWidths.next;
	CharWidths* next;
	while (cur != NULL)
	{
		next = cur->next;
		delete cur;
		cur = next;
	}

	ServersShutdown();
}

// GetSpriteIndex()
// searches through the sprite list loaded from hud.txt for a name matching SpriteName
// returns an index into the gHUD.m_rghSprites[] array
// returns 0 if sprite not found
int CHud :: GetSpriteIndex( const char *SpriteName )
{
	// look through the loaded sprite name list for SpriteName
	for ( int i = 0; i < m_iSpriteCount; i++ )
	{
		if ( strncmp( SpriteName, m_rgszSpriteNames + (i * MAX_SPRITE_NAME_LENGTH), MAX_SPRITE_NAME_LENGTH ) == 0 )
			return i;
	}

	return -1; // invalid sprite
}

void CHud :: VidInit( void )
{
	m_scrinfo.iSize = sizeof(m_scrinfo);
	GetScreenInfo(&m_scrinfo);

	CharWidths* cur = &m_CharWidths;
	while (cur != NULL)
	{
		cur->Reset();
		cur = cur->next;
	}

	// ----------
	// Load Sprites
	// ---------
	//	m_hsprFont = LoadSprite("sprites/%d_font.spr");
	m_hsprLogo = 0;
	m_hsprCursor = 0;

	if (ScreenWidth < 640)
		m_iRes = 320;
	else
		m_iRes = 640;

	client_sprite_t *p;
	m_iSpriteCount = 0;
	// Only load hud.txt once
	if ( !m_pSpriteList )
	{
		// we need to load the hud.txt, and all sprites within
		m_pSpriteList = SPR_GetList("sprites/hud.txt", &m_iSpriteCountAllRes);

		if (m_pSpriteList)
		{
			// count the number of sprites of the appropriate res
			m_iSpriteCountAlloc = 0;
			p = m_pSpriteList;
			for (int i = 0; i < m_iSpriteCountAllRes; i++)
			{
				if (p->iRes == m_iRes)
					m_iSpriteCountAlloc++;
				p++;
			}

			// allocated memory for sprite handle arrays
			m_iSpriteCountAlloc += RESERVE_SPRITES_FOR_WEAPONS;
			m_rghSprites = new HLHSPRITE[m_iSpriteCountAlloc];
			m_rgrcRects = new wrect_t[m_iSpriteCountAlloc];
			m_rgszSpriteNames = new char[m_iSpriteCountAlloc * MAX_SPRITE_NAME_LENGTH];
		}
	}
	// Load srpites on every VidInit
	// so make sure all the sprites have been loaded (may be we've gone through a transition, or loaded a save game)
	if (m_pSpriteList)
	{
		p = m_pSpriteList;
		for (int i = 0; i < m_iSpriteCountAllRes; i++)
		{
			if (p->iRes == m_iRes)
				AddSprite(p);
			p++;
		}
	}

	// assumption: number_1, number_2, etc, are all listed and loaded sequentially
	m_HUD_number_0 = GetSpriteIndex( "number_0" );

	m_iFontHeight = m_rgrcRects[m_HUD_number_0].bottom - m_rgrcRects[m_HUD_number_0].top;

	m_Ammo.VidInit();
	m_Health.VidInit();
	m_Spectator.VidInit();
	m_Geiger.VidInit();
	m_Train.VidInit();
	m_Battery.VidInit();
	m_Flash.VidInit();
	m_Message.VidInit();
	m_StatusBar.VidInit();
	m_DeathNotice.VidInit();
	m_SayText.VidInit();
	m_Menu.VidInit();
	m_AmmoSecondary.VidInit();
	m_TextMessage.VidInit();
	m_StatusIcons.VidInit();
	m_Timer.VidInit();
	m_Scores.VidInit();

	if (g_iIsAg)
	{
		m_Global.VidInit();
		m_Countdown.VidInit();
		m_CTF.VidInit();
		m_Location.VidInit();
		m_Longjump.VidInit();
		m_Nextmap.VidInit();
		m_PlayerId.VidInit();
		m_Settings.VidInit();
		m_SuddenDeath.VidInit();
		m_Timeout.VidInit();
		m_Vote.VidInit();
	}

	GetClientVoiceMgr()->VidInit();
}

void CHud::AddSprite(client_sprite_t *p)
{
	// Realloc arrays if needed
	if (m_iSpriteCount >= m_iSpriteCountAlloc)
	{
		int oldAllocCount = m_iSpriteCountAlloc;
		int newAllocCount = m_iSpriteCountAlloc + RESERVE_SPRITES_FOR_WEAPONS;
		m_iSpriteCountAlloc = newAllocCount;
		HLHSPRITE *new_rghSprites = new HLHSPRITE[newAllocCount];
		wrect_t *new_rgrcRects = new wrect_t[newAllocCount];
		char *new_rgszSpriteNames = new char[newAllocCount * MAX_SPRITE_NAME_LENGTH];
		memcpy(new_rghSprites, m_rghSprites, sizeof(HLHSPRITE) * oldAllocCount);
		memcpy(new_rgrcRects, m_rgrcRects, sizeof(wrect_t*) * oldAllocCount);
		memcpy(new_rgszSpriteNames, m_rgszSpriteNames, oldAllocCount * MAX_SPRITE_NAME_LENGTH);
		delete[] m_rghSprites;
		delete[] m_rgrcRects;
		delete[] m_rgszSpriteNames;
		m_rghSprites = new_rghSprites;
		m_rgrcRects = new_rgrcRects;
		m_rgszSpriteNames = new_rgszSpriteNames;
	}

	// Search for existing sprite
	int i = 0;
	for (i = 0; i < m_iSpriteCount; i++)
	{
		if (!_stricmp(&m_rgszSpriteNames[i * MAX_SPRITE_NAME_LENGTH], p->szName))
			return;
	}

	char sz[256];
	sprintf(sz, "sprites/%s.spr", p->szSprite);
	m_rghSprites[m_iSpriteCount] = SPR_Load(sz);
	m_rgrcRects[m_iSpriteCount] = p->rc;
	strncpy(&m_rgszSpriteNames[m_iSpriteCount * MAX_SPRITE_NAME_LENGTH], p->szName, MAX_SPRITE_NAME_LENGTH);
	m_rgszSpriteNames[m_iSpriteCount * MAX_SPRITE_NAME_LENGTH + MAX_SPRITE_NAME_LENGTH - 1] = 0;
	m_iSpriteCount++;
}

int CHud::MsgFunc_Logo(const char *pszName,  int iSize, void *pbuf)
{
	BEGIN_READ( pbuf, iSize );

	// update Train data
	m_iLogo = READ_BYTE();

	return 1;
}

float g_lastFOV = 0.0;

/*
============
COM_FileBase
============
*/
// Extracts the base name of a file (no path, no extension, assumes '/' as path separator)
void COM_FileBase ( const char *in, char *out)
{
	int len, start, end;

	len = strlen( in );
	
	// scan backward for '.'
	end = len - 1;
	while ( end && in[end] != '.' && in[end] != '/' && in[end] != '\\' )
		end--;
	
	if ( in[end] != '.' )		// no '.', copy to end
		end = len-1;
	else 
		end--;					// Found ',', copy to left of '.'


	// Scan backward for '/'
	start = len-1;
	while ( start >= 0 && in[start] != '/' && in[start] != '\\' )
		start--;

	if ( in[start] != '/' && in[start] != '\\' )
		start = 0;
	else 
		start++;

	// Length of new sting
	len = end - start + 1;

	// Copy partial string
	strncpy( out, &in[start], len );
	// Terminate it
	out[len] = 0;
}

/*
=================
HUD_IsGame

=================
*/
int HUD_IsGame( const char *game )
{
	const char *gamedir;
	char gd[ 1024 ];

	gamedir = gEngfuncs.pfnGetGameDirectory();
	if ( gamedir && gamedir[0] )
	{
		COM_FileBase( gamedir, gd );
		if ( !_stricmp( gd, game ) )
			return 1;
	}
	return 0;
}

/*
=====================
HUD_GetFOV

Returns last FOV
=====================
*/
float HUD_GetFOV( void )
{
	if ( gEngfuncs.pDemoAPI->IsRecording() )
	{
		// Write it
		int i = 0;
		unsigned char buf[ 100 ];

		// Active
		*( float * )&buf[ i ] = g_lastFOV;
		i += sizeof( float );

		Demo_WriteBuffer( TYPE_ZOOM, i, buf );
	}

	if ( gEngfuncs.pDemoAPI->IsPlayingback() )
	{
		g_lastFOV = g_demozoom;
	}
	return g_lastFOV;
}

int CHud::MsgFunc_SetFOV(const char *pszName,  int iSize, void *pbuf)
{
	BEGIN_READ( pbuf, iSize );

	int newfov = READ_BYTE();
	int def_fov = CVAR_GET_FLOAT( "default_fov" );

	//Weapon prediction already takes care of changing the fog. ( g_lastFOV ).
	if ( cl_lw && cl_lw->value )
		return 1;

	g_lastFOV = newfov;

	if ( newfov == 0 )
	{
		m_iFOV = def_fov;
	}
	else
	{
		m_iFOV = newfov;
	}

	// the clients fov is actually set in the client data update section of the hud

	// Set a new sensitivity
	if ( m_iFOV == def_fov )
	{  
		// reset to saved sensitivity
		m_flMouseSensitivity = 0;
	}
	else
	{  
		// set a new sensitivity that is proportional to the change from the FOV default
		m_flMouseSensitivity = sensitivity->value * ((float)newfov / (float)def_fov) * CVAR_GET_FLOAT("zoom_sensitivity_ratio");
	}

	return 1;
}


void CHud::AddHudElem(CHudBase *phudelem)
{
	HUDLIST *pdl, *ptemp;

//phudelem->Think();

	if (!phudelem)
		return;

	pdl = (HUDLIST *)malloc(sizeof(HUDLIST));
	if (!pdl)
		return;

	memset(pdl, 0, sizeof(HUDLIST));
	pdl->p = phudelem;

	if (!m_pHudList)
	{
		m_pHudList = pdl;
		return;
	}

	ptemp = m_pHudList;

	while (ptemp->pNext)
		ptemp = ptemp->pNext;

	ptemp->pNext = pdl;
}

float CHud::GetSensitivity( void )
{
	return m_flMouseSensitivity;
}

bool ParseColor( char *string, RGBA &rgba )
{
	unsigned char r,g,b;
	char *value = string;
	while (*value == ' ') value++;
	if (*value < '0' || *value > '9') return false;
	r = atoi(value);
	value = strchr(value, ' ');
	if (value == NULL) return false;
	while (*value == ' ') value++;
	if (*value < '0' || *value > '9') return false;
	g = atoi(value);
	value = strchr(value, ' ');
	if (value == NULL) return false;
	while (*value == ' ') value++;
	if (*value < '0' || *value > '9') return false;
	b = atoi(value);
	rgba.Set(r, g, b);
	return true;
}

// hudPart: 0 - common hud, 1 - health points, 2 - armor points
void CHud::GetHudColor( int hudPart, int value, int &r, int &g, int &b )
{
	RGBA *c;
	if (hudPart == 0) { ParseColor(m_pCvarColor->string, m_hudColor); c = &m_hudColor; }
	else if (value >= 90) { ParseColor(m_pCvarColor1->string, m_hudColor1); c = &m_hudColor1; }
	else if (value >= 50 && value <= 90) { ParseColor(m_pCvarColor2->string, m_hudColor2); c = &m_hudColor2; }
	else if ((value > 25 && value < 50) || hudPart == 2) { ParseColor(m_pCvarColor3->string, m_hudColor3); c = &m_hudColor3; }
	else { r = 255; g = 0; b = 0; return; }	// UnpackRGB(r, g, b, RGB_REDISH);
	r = c->r;
	g = c->g;
	b = c->b;
}

float CHud::GetHudTransparency()
{
	float hud_draw = m_pCvarDraw->value;

	if (hud_draw > 1) hud_draw = 1;
	if (hud_draw < 0) hud_draw = 0;

	return hud_draw;
}

void GetConsoleStringSize(const char *string, int *width, int *height)
{
	if (gHUD.m_pCvarColorText->value == 0)
		gEngfuncs.pfnDrawConsoleStringLen(string, width, height);
	else
		gEngfuncs.pfnDrawConsoleStringLen(RemoveColorCodes((char*)string), width, height);
}

int DrawConsoleString(int x, int y, const char *string, float *color)
{
	if (!string || !*string)
		return x;

	if (color != NULL)
		gEngfuncs.pfnDrawSetTextColor(color[0], color[1], color[2]);
	else
		gEngfuncs.pfnDrawConsoleString(x, y, " ");	// Reset color to con_color

	if (gHUD.m_pCvarColorText->value == 0)
		return gEngfuncs.pfnDrawConsoleString(x, y, (char*)string);

	char *c1 = (char*)string;
	char *c2 = (char*)string;
	float r, g, b;
	int colorIndex;
	while (true)
	{
		// Search for next color code
		colorIndex = -1;
		while(*c2 && *(c2 + 1) && !(*c2 == '^' && *(c2 + 1) >= '0' && *(c2 + 1) <= '9'))
			c2++;
		if (*c2 == '^' && *(c2 + 1) >= '0' && *(c2 + 1) <= '9')
		{
			colorIndex = *(c2 + 1) - '0';
			*c2 = 0;
		}
		// Draw current string
		x = gEngfuncs.pfnDrawConsoleString(x, y, c1);

		if (colorIndex >= 0)
		{
			// Revert change and advance
			*c2 = '^';
			c2 += 2;
			c1 = c2;

			// Return if next string is empty
			if (!*c1)
				return x;

			// Setup color
			if (color == NULL && colorIndex <= 8 && gHUD.m_pCvarColorText->value == 1)
			{
				r = g_iColorsCodes[colorIndex][0] / 256.0;
				g = g_iColorsCodes[colorIndex][1] / 256.0;
				b = g_iColorsCodes[colorIndex][2] / 256.0;
				gEngfuncs.pfnDrawSetTextColor(r, g, b);
			}
			else if (color != NULL)
				gEngfuncs.pfnDrawSetTextColor(color[0], color[1], color[2]);
			continue;
		}

		// Done
		break;
	}
	return x;
}

char *RemoveColorCodes(const char *string, bool inPlace)
{
	static char buffer[1024];

	char *c1 = inPlace ? (char *)string : buffer;
	char *c2 = (char *)string;
	char *end = inPlace ? 0 : buffer + sizeof(buffer) - 1;
	while(*c2 && (inPlace || c1 < end))
	{
		if (*c2 == '^' && *(c2 + 1) >= '0' && *(c2 + 1) <= '9')
		{
			c2 += 2;
			continue;
		}
		*c1 = *c2;
		c1++;
		c2++;
	}
	*c1 = 0;

	return buffer;
}

void ConsolePrint(const char *string)
{
	if (gHUD.m_pCvarColorText->value == 0)
		gEngfuncs.pfnConsolePrint(string);
	else
		gEngfuncs.pfnConsolePrint(RemoveColorCodes(string));
}

void ConsolePrintColor(const char *string, RGBA color)
{
	RGBA oldColor = SetConsoleColor(color);
	if (gHUD.m_pCvarColorText->value == 0)
		gEngfuncs.pfnConsolePrint(string);
	else
		gEngfuncs.pfnConsolePrint(RemoveColorCodes(string));
	SetConsoleColor(oldColor);
}

void CenterPrint( const char *string )
{
	if (gHUD.m_pCvarColorText->value == 0)
		gEngfuncs.pfnCenterPrint(string);
	else
		gEngfuncs.pfnCenterPrint(RemoveColorCodes(string));
}
