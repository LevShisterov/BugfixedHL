#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

#include <cmath>

int CHudSpeedometer::Init(void)
{
	m_iFlags = HUD_ACTIVE;
	
	m_pCvarSpeedometer = CVAR_CREATE("hud_speedometer", "0", FCVAR_ARCHIVE);

	gHUD.AddHudElem(this);

	return 0;
}

int CHudSpeedometer::VidInit()
{
	return 1;
}

int CHudSpeedometer::Draw(float time)
{
	if ((gHUD.m_iHideHUDDisplay & HIDEHUD_HEALTH) || gEngfuncs.IsSpectateOnly())
		return 0;

	if (m_pCvarSpeedometer->value == 0.0f)
		return 0;

	int r, g, b;
	gHUD.GetHudColor(0, 0, r, g, b);
	gHUD.DrawHudNumberCentered(ScreenWidth / 2, ScreenHeight - gHUD.m_iFontHeight - gHUD.m_iFontHeight / 2, m_iSpeed, r, g, b);

	return 0;
}

void CHudSpeedometer::UpdateSpeed(const float velocity[2])
{
	m_iSpeed = std::round(std::hypot(velocity[0], velocity[1]));
}