// Martin Webrant (BulliT)

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

DECLARE_MESSAGE(m_Countdown, Countdown)

int AgHudCountdown::Init(void)
{
	HOOK_MESSAGE(Countdown);

	gHUD.AddHudElem(this);

	m_iFlags = 0;
	m_btCountdown = -1;

	return 1;
}

int AgHudCountdown::VidInit(void)
{
	return 1;
}

void AgHudCountdown::Reset(void)
{
	m_iFlags &= ~HUD_ACTIVE;
	m_btCountdown = -1;
}

int AgHudCountdown::Draw(float fTime)
{
	if (gHUD.m_iIntermission)
		return 0;

	char szText[128];

	int r, g, b, a;
	a = 255 * gHUD.GetHudTransparency();
	gHUD.GetHudColor(0, 0, r, g, b);
	ScaleColors(r, g, b, a);

	if (m_btCountdown != 50)
	{
		int iWidth = gHUD.GetSpriteRect(gHUD.m_HUD_number_0).right - gHUD.GetSpriteRect(gHUD.m_HUD_number_0).left;
		//int iHeight = gHUD.GetSpriteRect(gHUD.m_HUD_number_0).bottom - gHUD.GetSpriteRect(gHUD.m_HUD_number_0).top;

		gHUD.DrawHudNumber(ScreenWidth / 2 - iWidth / 2, gHUD.m_scrinfo.iCharHeight * 10, DHN_DRAWZERO, m_btCountdown, r, g, b);
		if (0 != strlen(m_szPlayer1) && 0 != strlen(m_szPlayer2))
		{
			// Write arena text.
			sprintf(szText, "%s vs %s", m_szPlayer1, m_szPlayer2);
			AgDrawHudStringCentered(ScreenWidth / 2, gHUD.m_scrinfo.iCharHeight * 7, ScreenWidth, szText, r, g, b);
		}
		else
		{
			// Write match text.
			strcpy(szText, "Match about to start");
			AgDrawHudStringCentered(ScreenWidth / 2, gHUD.m_scrinfo.iCharHeight * 7, ScreenWidth, szText, r, g, b);
		}
	}
	else
	{
		if (strlen(m_szPlayer1) != 0)
		{
			sprintf(szText, "Last round won by %s", m_szPlayer1);
			AgDrawHudStringCentered(ScreenWidth / 2, gHUD.m_scrinfo.iCharHeight * 7, ScreenWidth, szText, r, g, b);
		}
		else
		{
			strcpy(szText, "Waiting for players to get ready");
			AgDrawHudStringCentered(ScreenWidth / 2, gHUD.m_scrinfo.iCharHeight * 7, ScreenWidth, szText, r, g, b);
		}
	}

	return 0;
}

int AgHudCountdown::MsgFunc_Countdown(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	// Update data
	m_btCountdown = READ_BYTE();
	char btSound = READ_BYTE();
	strcpy(m_szPlayer1, READ_STRING());
	strcpy(m_szPlayer2, READ_STRING());

	if (m_btCountdown >= 0)
	{
		m_iFlags |= HUD_ACTIVE;

		if (btSound)
		{
			// Play countdown sound
			switch (m_btCountdown)
			{
			case 0: PlaySound("barney/ba_bring.wav", 1); break;
			case 1: PlaySound("fvox/one.wav", 1); break;
			case 2: PlaySound("fvox/two.wav", 1); break;
			case 3: PlaySound("fvox/three.wav", 1); break;
			case 4: PlaySound("fvox/four.wav", 1); break;
			case 5: PlaySound("fvox/five.wav", 1); break;
			case 6: PlaySound("fvox/six.wav", 1); break;
			case 7: PlaySound("fvox/seven.wav", 1); break;
			case 8: PlaySound("fvox/eight.wav", 1); break;
			case 9: PlaySound("fvox/nine.wav", 1); break;
			case 10: PlaySound("fvox/ten.wav", 1); break;
			default: break;
			}
		}
	}
	else
		m_iFlags &= ~HUD_ACTIVE;

	return 1;
}
