// Martin Webrant (BulliT)

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

DECLARE_MESSAGE(m_Longjump, Longjump)

int AgHudLongjump::Init(void)
{
	HOOK_MESSAGE(Longjump);

	gHUD.AddHudElem(this);

	m_iFlags = 0;
	m_flTurnoff = 0;

	return 1;
}

int AgHudLongjump::VidInit(void)
{
	return 1;
}

void AgHudLongjump::Reset(void)
{
	m_iFlags &= ~HUD_ACTIVE;
}

int AgHudLongjump::Draw(float fTime)
{
	if (gHUD.m_flTime > m_flTurnoff || gHUD.m_iIntermission)
	{
		Reset();
		return 1;
	}

	char szText[32];

	int r, g, b, a;
	UnpackRGB(r, g, b, RGB_GREENISH);
	a = 255 * gHUD.GetHudTransparency();
	ScaleColors(r, g, b, a);

	sprintf(szText, "Longjump %d", (int)(m_flTurnoff - gHUD.m_flTime));
	AgDrawHudStringCentered(ScreenWidth / 2, gHUD.m_scrinfo.iCharHeight * 2, ScreenWidth, szText, r, g, b);

	return 0;
}

int AgHudLongjump::MsgFunc_Longjump(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);
	int iTime = READ_BYTE();

	m_flTurnoff = gHUD.m_flTime + iTime;
	m_iFlags |= HUD_ACTIVE;

	return 1;
}
