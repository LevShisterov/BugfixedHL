// Martin Webrant (BulliT)

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

DECLARE_MESSAGE(m_PlayerId, PlayerId)

int AgHudPlayerId::Init(void)
{
	HOOK_MESSAGE(PlayerId);

	gHUD.AddHudElem(this);

	m_iFlags = 0;
	m_flTurnoff = 0.0;
	m_iPlayer = 0;
	m_bTeam = false;
	m_iHealth = 0;
	m_iArmour = 0;

	m_pCvarHudPlayerId = gEngfuncs.pfnRegisterVariable("hud_playerid", "1", FCVAR_ARCHIVE);

	return 1;
}

int AgHudPlayerId::VidInit(void)
{
	return 1;
}

void AgHudPlayerId::Reset(void)
{
	m_iFlags &= ~HUD_ACTIVE;
	m_iPlayer = 0;
}

int AgHudPlayerId::Draw(float fTime)
{
	if (m_iPlayer <= 0 || m_pCvarHudPlayerId->value == 0)
		return 1;

	if (gHUD.m_flTime > m_flTurnoff)
	{
		Reset();
		return 1;
	}

	if (g_PlayerInfoList[m_iPlayer].name)
	{
		char szText[64];
		if (m_bTeam)
			sprintf(szText, "%s %d/%d", g_PlayerInfoList[m_iPlayer].name, m_iHealth, m_iArmour);
		else
			sprintf(szText, "%s", g_PlayerInfoList[m_iPlayer].name);

		int r, g, b, a;
		if (m_bTeam)
			UnpackRGB(r, g, b, RGB_GREENISH);
		else
			UnpackRGB(r, g, b, RGB_REDISH);
		a = 255 * gHUD.GetHudTransparency();
		ScaleColors(r, g, b, a);

		if (CVAR_GET_FLOAT("hud_centerid"))
		{
			AgDrawHudStringCentered(ScreenWidth / 2, ScreenHeight - ScreenHeight / 4, ScreenWidth, szText, r, g, b);
		}
		else
		{
			if (gHUD.m_pCvarColorText->value != 0)
				RemoveColorCodes(szText, true);
			gHUD.DrawHudString(10, ScreenHeight - ScreenHeight / 8, szText, r, g, b);
		}
	}

	return 0;
}

int AgHudPlayerId::MsgFunc_PlayerId(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	m_iPlayer = READ_BYTE();
	m_bTeam = READ_BYTE() == 1;
	m_iHealth = READ_SHORT();
	m_iArmour = READ_SHORT();

	if (m_pCvarHudPlayerId->value == 0)
		m_iFlags &= ~HUD_ACTIVE;
	else
		m_iFlags |= HUD_ACTIVE;

	GetPlayerInfo(m_iPlayer, &g_PlayerInfoList[m_iPlayer]);

	m_flTurnoff = gHUD.m_flTime + 2; // Hold for 2 seconds.

	return 1;
}
