// Martin Webrant (BulliT)

#include "hud.h"
#include "cl_util.h"
#include "com_weapons.h"
#include "parsemsg.h"
#include "aghudglobal.h"

DECLARE_MESSAGE(m_Global, PlaySound)
DECLARE_MESSAGE(m_Global, CheatCheck)
DECLARE_MESSAGE(m_Global, WhString)
DECLARE_MESSAGE(m_Global, SpikeCheck)
DECLARE_MESSAGE(m_Global, Gametype)
DECLARE_MESSAGE(m_Global, AuthID)
DECLARE_MESSAGE(m_Global, MapList)
DECLARE_MESSAGE(m_Global, CRC32)
DECLARE_MESSAGE(m_Global, Splash)

int g_GameType = GT_STANDARD;

int AgHudGlobal::Init(void)
{
	HOOK_MESSAGE(PlaySound);
	HOOK_MESSAGE(CheatCheck);
	HOOK_MESSAGE(WhString);
	HOOK_MESSAGE(SpikeCheck);
	HOOK_MESSAGE(Gametype);
	HOOK_MESSAGE(AuthID);
	HOOK_MESSAGE(MapList);
	HOOK_MESSAGE(CRC32);
	HOOK_MESSAGE(Splash);

	gHUD.AddHudElem(this);

	m_iFlags = 0;

	return 1;
}

int AgHudGlobal::VidInit(void)
{
	return 1;
}

void AgHudGlobal::Reset(void)
{
	m_iFlags |= HUD_ACTIVE;
}

int AgHudGlobal::Draw(float fTime)
{
	return 1;
}

int AgHudGlobal::MsgFunc_PlaySound(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	vec3_t origin;
	/*int iPlayer = */READ_BYTE();
	for (int i = 0; i < 3; i++)
		origin[i] = READ_COORD();
	char* pszSound = READ_STRING();

	gEngfuncs.pfnPlaySoundByName(pszSound, 1);

	return 1;
}

int AgHudGlobal::MsgFunc_CheatCheck(const char *pszName, int iSize, void *pbuf)
{
	return 1;
}

int AgHudGlobal::MsgFunc_WhString(const char *pszName, int iSize, void *pbuf)
{
	return 1;
}

int AgHudGlobal::MsgFunc_SpikeCheck(const char *pszName, int iSize, void *pbuf)
{
	return 1;
}

int AgHudGlobal::MsgFunc_Gametype(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);
	g_GameType = READ_BYTE();
	return 1;
}

int AgHudGlobal::MsgFunc_AuthID(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);
	const int slot = READ_BYTE();
	char* steamid = READ_STRING();

	if (!strncmp(steamid, "STEAM_", 6) ||
		!strncmp(steamid, "VALVE_", 6))
		strncpy(g_PlayerSteamId[slot], steamid + 6, MAX_STEAMID); // cutout "STEAM_" or "VALVE_" start of the string
	else
		strncpy(g_PlayerSteamId[slot], steamid, MAX_STEAMID);
	g_PlayerSteamId[slot][MAX_STEAMID] = 0;

	return 1;
}

int AgHudGlobal::MsgFunc_MapList(const char *pszName, int iSize, void *pbuf)
{
	return 1;
}

int AgHudGlobal::MsgFunc_CRC32(const char *pszName, int iSize, void *pbuf)
{
	return 1;
}

int AgHudGlobal::MsgFunc_Splash(const char *pszName, int iSize, void *pbuf)
{
	return 1;
}

// Colors
int iNumConsoleColors = 16;
int arrConsoleColors[16][3] =
{
	{ 255, 170, 0 },	// HL console (default)
	{ 255, 0,   0 },	// Red
	{ 0,   255, 0 },	// Green
	{ 255, 255, 0 },	// Yellow
	{ 0,   0,   255 },	// Blue
	{ 0,   255, 255 },	// Cyan
	{ 255, 0,   255 },	// Violet
	{ 136, 136, 136 },	// Q
	{ 255, 255, 255 },	// White
	{ 0,   0,   0 },	// Black
	{ 200, 90,  70 },	// Redb
	{ 145, 215, 140 },	// Green
	{ 225, 205, 45 },	// Yellow
	{ 125, 165, 210 },	// Blue
	{ 70,  70,  70 },
	{ 200, 200, 200 },
};
int GetColor(char cChar)
{
	int iColor = -1;
	if (cChar >= '0' && cChar <= '9')
		iColor = cChar - '0';
	return iColor;
}
int AgDrawHudString(int xpos, int ypos, int iMaxX, const char *szIt, int r, int g, int b)
{
	// calc center
	int iSizeX = 0;
	char* pszIt = (char*)szIt;
	for (; *pszIt != 0 && *pszIt != '\n'; pszIt++)
		iSizeX += gHUD.m_scrinfo.charWidths[*pszIt]; // variable-width fonts look cool

	int rx = r, gx = g, bx = b;

	pszIt = (char*)szIt;
	// draw the string until we hit the null character or a newline character
	for (; *pszIt != 0 && *pszIt != '\n'; pszIt++)
	{
		if (*pszIt == '^')
		{
			pszIt++;
			int iColor = GetColor(*pszIt);
			if (iColor < iNumConsoleColors && iColor >= 0)
			{
				if (0 >= iColor)// || 0 == g_pcl_show_colors->value)
				{
					rx = r;
					gx = g;
					bx = b;
				}
				else
				{
					rx = arrConsoleColors[iColor][0];
					gx = arrConsoleColors[iColor][1];
					bx = arrConsoleColors[iColor][2];
				}
				pszIt++;
				if (*pszIt == 0 || *pszIt == '\n')
					break;
			}
			else
				pszIt--;
		}

		int next = xpos + gHUD.m_scrinfo.charWidths[*pszIt]; // variable-width fonts look cool
		if (next > iMaxX)
			return xpos;
		TextMessageDrawChar(xpos, ypos, *pszIt, rx, gx, bx);
		xpos = next;
	}
	return xpos;
}
int AgDrawHudStringCentered(int xpos, int ypos, int iMaxX, const char *szIt, int r, int g, int b)
{
	// calc center
	int iSizeX = 0;
	char* pszIt = (char*)szIt;
	for (; *pszIt != 0 && *pszIt != '\n'; pszIt++)
		iSizeX += gHUD.m_scrinfo.charWidths[*pszIt]; // variable-width fonts look cool

	// Subtract half sizex from xpos to center it.
	xpos = xpos - iSizeX / 2;

	int rx = r, gx = g, bx = b;

	pszIt = (char*)szIt;
	// draw the string until we hit the null character or a newline character
	for (; *pszIt != 0 && *pszIt != '\n'; pszIt++)
	{
		if (*pszIt == '^')
		{
			pszIt++;
			int iColor = GetColor(*pszIt);
			if (iColor < iNumConsoleColors && iColor >= 0)
			{
				if (0 >= iColor)// || 0 == g_pcl_show_colors->value)
				{
					rx = r;
					gx = g;
					bx = b;
				}
				else
				{
					rx = arrConsoleColors[iColor][0];
					gx = arrConsoleColors[iColor][1];
					bx = arrConsoleColors[iColor][2];
				}
				pszIt++;
				if (*pszIt == 0 || *pszIt == '\n')
					break;
			}
			else
				pszIt--;
		}

		int next = xpos + gHUD.m_scrinfo.charWidths[*pszIt]; // variable-width fonts look cool
		if (next > iMaxX)
			return xpos;
		TextMessageDrawChar(xpos, ypos, *pszIt, rx, gx, bx);
		xpos = next;
	}
	return xpos;
}
