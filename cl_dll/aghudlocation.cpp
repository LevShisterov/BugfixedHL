// Martin Webrant (BulliT)

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

DECLARE_MESSAGE(m_Location, Location)
DECLARE_MESSAGE(m_Location, InitLoc)

int AgHudLocation::Init(void)
{
	m_fAt = 0;
	m_fNear = 0;
	m_iFlags = 0;
	m_szMap[0] = '\0';

	for (int i = 1; i <= MAX_PLAYERS; i++)
		m_vPlayerLocations[i] = Vector(0, 0, 0);

	gHUD.AddHudElem(this);

	HOOK_MESSAGE(Location);
	HOOK_MESSAGE(InitLoc);

	return 1;
}

int AgHudLocation::VidInit(void)
{
	return 1;
}

void AgHudLocation::Reset(void)
{
	m_iFlags &= ~HUD_ACTIVE;
}

int AgHudLocation::Draw(float fTime)
{
	return 0;
}

int AgHudLocation::MsgFunc_Location(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);
	int iPlayer = READ_BYTE();
	for (int i = 0; i < 3; i++)
		m_vPlayerLocations[iPlayer][i] = READ_COORD();

	return 1;
}

int AgHudLocation::MsgFunc_InitLoc(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);
	strcpy(m_szMap, READ_STRING());

	return 1;
}
