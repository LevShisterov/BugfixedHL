// Martin Webrant (BulliT)

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

DECLARE_MESSAGE(m_Settings, Settings)

int g_iMatch = 0;

int AgHudSettings::Init(void)
{
	HOOK_MESSAGE(Settings);

	gHUD.AddHudElem(this);

	g_iMatch = 0;

	m_iFlags = 0;
	m_szGamemode[0] = '\0';
	m_iTimeLimit = 0;
	m_iFragLimit = 0;
	m_iFriendlyFire = 0;
	m_iWeaponstay = 0;
	m_szVersion[0] = '\0';
	m_szWallgauss[0] = '\0';
	m_szHeadShot[0] = '\0';
	m_szBlastRadius[0] = '\0';
	m_flTurnoff = 0.0;

	m_pCvarHudSettings = gEngfuncs.pfnRegisterVariable("hud_settings", "1", FCVAR_ARCHIVE);

	return 1;
}

int AgHudSettings::VidInit(void)
{
	return 1;
}

void AgHudSettings::Reset(void)
{
	m_iFlags &= ~HUD_ACTIVE;
	m_flTurnoff = 0;
}

int AgHudSettings::Draw(float fTime)
{
	if (gHUD.m_flTime > m_flTurnoff || m_pCvarHudSettings->value == 0)
	{
		Reset();
		return 1;
	}

	char szText[128];

	int r, g, b, a;
	a = 255 * gHUD.GetHudTransparency();
	gHUD.GetHudColor(0, 0, r, g, b);
	ScaleColors(r, g, b, a);

	int x = ScreenWidth - (ScreenWidth / 4);
	int y = 10;

	sprintf(szText, "Adrenaline Gamer Mod %s", m_szVersion);
	gHUD.DrawHudString(ScreenWidth / 20, gHUD.m_scrinfo.iCharHeight, szText, r, g, b);
	gHUD.DrawHudString(ScreenWidth / 20, gHUD.m_scrinfo.iCharHeight * 2, "www.planethalflife.com/agmod", r, g, b);
	gHUD.DrawHudString(ScreenWidth / 20, gHUD.m_scrinfo.iCharHeight * 3, "Martin Webrant", r, g, b);

	sprintf(szText, "%s", m_szGamemode);
	gHUD.DrawHudString(x, y, szText, r, g, b);
	y += gHUD.m_scrinfo.iCharHeight * 2;

	sprintf(szText, "Time limit: %d", m_iTimeLimit);
	gHUD.DrawHudString(x, y, szText, r, g, b);
	y += gHUD.m_scrinfo.iCharHeight;

	sprintf(szText, "Frag limit: %d", m_iFragLimit);
	gHUD.DrawHudString(x, y, szText, r, g, b);
	y += gHUD.m_scrinfo.iCharHeight;

	sprintf(szText, "Friendly fire: %s", m_iFriendlyFire ? "On" : "Off");
	gHUD.DrawHudString(x, y, szText, r, g, b);
	y += gHUD.m_scrinfo.iCharHeight;

	sprintf(szText, "Weaponstay: %s", m_iWeaponstay ? "On" : "Off");
	gHUD.DrawHudString(x, y, szText, r, g, b);
	y += gHUD.m_scrinfo.iCharHeight;

	if (strlen(m_szWallgauss))
	{
		sprintf(szText, "Wallgauss: %sX (1)", m_szWallgauss);
		gHUD.DrawHudString(x, y, szText, r, g, b);
		y += gHUD.m_scrinfo.iCharHeight;
	}
	if (strlen(m_szHeadShot))
	{
		sprintf(szText, "Headshot: %sX (3)", m_szHeadShot);
		gHUD.DrawHudString(x, y, szText, r, g, b);
		y += gHUD.m_scrinfo.iCharHeight;
	}
	if (strlen(m_szBlastRadius))
	{
		sprintf(szText, "BlastRadius: %sX (1)", m_szBlastRadius);
		gHUD.DrawHudString(x, y, szText, r, g, b);
	}
	if (g_iMatch)
	{
		strcpy(szText, "Match is on!");
		AgDrawHudStringCentered(ScreenWidth / 2, gHUD.m_scrinfo.iCharHeight * 2, ScreenWidth, szText, r, g, b);
	}

	AgDrawHudStringCentered(ScreenWidth / 2, gHUD.m_scrinfo.iCharHeight * 3, ScreenWidth, gHUD.m_Location.m_szMap, r, g, b);

	return 0;
}

int AgHudSettings::MsgFunc_Settings(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	g_iMatch = READ_BYTE();
	strcpy(m_szGamemode, READ_STRING());
	m_iTimeLimit = READ_BYTE();
	m_iFragLimit = READ_BYTE();
	m_iFriendlyFire = READ_BYTE();
	m_iWeaponstay = READ_BYTE();
	strcpy(m_szVersion, READ_STRING());
	strcpy(m_szWallgauss, READ_STRING());
	strcpy(m_szHeadShot, READ_STRING());
	strcpy(m_szBlastRadius, READ_STRING());

	if (strcmp(m_szWallgauss, "1") == 0)
		m_szWallgauss[0] = '\0';
	if (strcmp(m_szHeadShot, "3") == 0)
		m_szHeadShot[0] = '\0';
	if (strcmp(m_szBlastRadius, "1") == 0)
		m_szBlastRadius[0] = '\0';

	if (m_pCvarHudSettings->value == 0)
		m_iFlags &= ~HUD_ACTIVE;
	else
		m_iFlags |= HUD_ACTIVE;

	m_flTurnoff = gHUD.m_flTime + 10; // Hold for 10 seconds.

	return 1;
}

bool AgIsMatch()
{
	return g_iMatch == 1;
}
