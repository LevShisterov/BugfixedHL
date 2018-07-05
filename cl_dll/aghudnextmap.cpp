// Martin Webrant (BulliT)

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

DECLARE_MESSAGE(m_Nextmap, Nextmap)

int AgHudNextmap::Init(void)
{
	HOOK_MESSAGE(Nextmap);

	gHUD.AddHudElem(this);

	m_iFlags = 0;
	m_szNextmap[0] = '\0';
	m_flTurnoff = 0;

	return 1;
}

int AgHudNextmap::VidInit(void)
{
	return 1;
}

void AgHudNextmap::Reset(void)
{
	m_iFlags &= ~HUD_ACTIVE;
}

int AgHudNextmap::Draw(float fTime)
{
	if (gHUD.m_flTime > m_flTurnoff)
	{
		Reset();
		return 1;
	}

	char szText[32];

	int r, g, b, a;
	a = 255 * gHUD.GetHudTransparency();
	gHUD.GetHudColor(0, 0, r, g, b);
	ScaleColors(r, g, b, a);

	sprintf(szText, "Nextmap is %s", m_szNextmap);
	AgDrawHudStringCentered(ScreenWidth / 2, gHUD.m_scrinfo.iCharHeight * 5, ScreenWidth, szText, r, g, b);

	return 0;
}

int AgHudNextmap::MsgFunc_Nextmap(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);
	strcpy(m_szNextmap, READ_STRING());

	gHUD.m_Timer.SetNextmap(m_szNextmap);

	const int hud_nextmap = (int)gHUD.m_Timer.GetHudNextmap();
	if (hud_nextmap != 2 && hud_nextmap != 1)
	{
		m_flTurnoff = gHUD.m_flTime + 10; // Display for 10 seconds.
		m_iFlags |= HUD_ACTIVE;
	}

	return 1;
}
