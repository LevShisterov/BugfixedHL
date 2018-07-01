// Martin Webrant (BulliT)

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

DECLARE_MESSAGE(m_Timeout, Timeout)

int AgHudTimeout::Init(void)
{
	HOOK_MESSAGE(Timeout);

	gHUD.AddHudElem(this);

	m_iFlags = 0;
	m_State = Inactive;
	m_iTime = 0;

	return 1;
}

int AgHudTimeout::VidInit(void)
{
	m_State = Inactive;
	return 1;
}

void AgHudTimeout::Reset(void)
{
	m_iFlags &= ~HUD_ACTIVE;
}

int AgHudTimeout::Draw(float fTime)
{
	if (Inactive == m_State)
	{
		Reset();
		return 1;
	}

	char szText[64];
	szText[0] = '\0';

	int r, g, b, a;
	UnpackRGB(r, g, b, RGB_GREENISH);
	a = 255 * gHUD.GetHudTransparency();
	ScaleColors(r, g, b, a);

	if (Called == m_State)
		sprintf(szText, "Timeout called, stopping in %d seconds.", m_iTime);
	else if (Countdown == m_State)
		sprintf(szText, "Timeout, starting in %d seconds.", m_iTime);
	else
		return 0;

	AgDrawHudStringCentered(ScreenWidth / 2, gHUD.m_scrinfo.iCharHeight * 6, ScreenWidth, szText, r, g, b);

	return 0;
}

int AgHudTimeout::MsgFunc_Timeout(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);
	m_State = READ_BYTE();
	m_iTime = READ_BYTE();
	m_iFlags |= HUD_ACTIVE;

	return 1;
}
