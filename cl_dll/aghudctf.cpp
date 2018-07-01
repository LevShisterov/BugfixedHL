// Martin Webrant (BulliT)

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "vgui_TeamFortressViewport.h"

DECLARE_MESSAGE(m_CTF, CTF)
DECLARE_MESSAGE(m_CTF, CTFSound)
DECLARE_MESSAGE(m_CTF, CTFFlag)

int g_iPlayerFlag1 = 0;
int g_iPlayerFlag2 = 0;

static char* s_szSounds[] =
{
	"ctf/youhaveflag.wav",
	"ctf/teamhaveflag.wav",
	"ctf/enemyhaveflag.wav",
	"ctf/blueflagreturned.wav",
	"ctf/redflagreturned.wav",
	"ctf/bluescores.wav",
	"ctf/redscores.wav",
	"ctf/blueflagstolen.wav",
	"ctf/redflagstolen.wav",
	// Not used but can be good to have...
	"ctf/blueleads",
	"ctf/redleads",
	"ctf/teamstied",
	"ctf/suddendeath",
	"ctf/stolen"
	"ctf/capture"
};

int AgHudCTF::Init(void)
{
	HOOK_MESSAGE(CTF);
	HOOK_MESSAGE(CTFSound);
	HOOK_MESSAGE(CTFFlag);

	gHUD.AddHudElem(this);

	m_iFlags = 0;
	m_iFlagStatus1 = 0;
	m_iFlagStatus2 = 0;

	g_iPlayerFlag1 = 0;
	g_iPlayerFlag2 = 0;

	m_pCvarClCtfVolume = gEngfuncs.pfnRegisterVariable("cl_ctf_volume", "1", FCVAR_ARCHIVE);

	return 1;
}

int AgHudCTF::VidInit(void)
{
	int iSprite;

	iSprite = gHUD.GetSpriteIndex("icon_ctf_home");
	m_IconFlagStatus[Home].spr = gHUD.GetSprite(iSprite);
	m_IconFlagStatus[Home].rc = gHUD.GetSpriteRect(iSprite);

	iSprite = gHUD.GetSpriteIndex("icon_ctf_stolen");
	m_IconFlagStatus[Stolen].spr = gHUD.GetSprite(iSprite);
	m_IconFlagStatus[Stolen].rc = gHUD.GetSpriteRect(iSprite);

	iSprite = gHUD.GetSpriteIndex("icon_ctf_lost");
	m_IconFlagStatus[Lost].spr = gHUD.GetSprite(iSprite);
	m_IconFlagStatus[Lost].rc = gHUD.GetSpriteRect(iSprite);

	iSprite = gHUD.GetSpriteIndex("icon_ctf_carry");
	m_IconFlagStatus[Carry].spr = gHUD.GetSprite(iSprite);
	m_IconFlagStatus[Carry].rc = gHUD.GetSpriteRect(iSprite);

	m_iFlagStatus1 = 0;
	m_iFlagStatus2 = 0;

	g_iPlayerFlag1 = 0;
	g_iPlayerFlag2 = 0;

	return 1;
}

void AgHudCTF::Reset(void)
{
	if (AgGametype() != GT_CTF || gHUD.m_iIntermission)
		m_iFlags &= ~HUD_ACTIVE;
}

int AgHudCTF::Draw(float fTime)
{
	if (m_iFlagStatus1 == Off || m_iFlagStatus2 == Off || gHUD.m_iIntermission)
	{
		Reset();
		return 0;
	}

	int x = 30;
	int y = ScreenHeight / 2;

	// Draw Blue
	if (m_IconFlagStatus[m_iFlagStatus1].spr)
	{
		int yBlue = y - ((m_IconFlagStatus[m_iFlagStatus2].rc.bottom - m_IconFlagStatus[m_iFlagStatus2].rc.top) + 5);
		SPR_Set(m_IconFlagStatus[m_iFlagStatus1].spr, iTeamColors[1][0], iTeamColors[1][1], iTeamColors[1][2]);
		SPR_DrawAdditive(0, x, yBlue, &m_IconFlagStatus[m_iFlagStatus1].rc);
	}
	// Draw Red
	if (m_IconFlagStatus[m_iFlagStatus2].spr)
	{
		//y += (m_IconFlagStatus[m_iFlagStatus2].rc.bottom - m_IconFlagStatus[m_iFlagStatus2].rc.top) + 5;
		SPR_Set(m_IconFlagStatus[m_iFlagStatus2].spr, iTeamColors[2][0], iTeamColors[2][1], iTeamColors[2][2]);
		SPR_DrawAdditive(0, x, y, &m_IconFlagStatus[m_iFlagStatus2].rc);
	}

	return 0;
}

int AgHudCTF::MsgFunc_CTF(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);
	m_iFlagStatus1 = READ_BYTE();
	m_iFlagStatus2 = READ_BYTE();

	if (m_iFlagStatus1 != Off && m_iFlagStatus2 != Off)
		m_iFlags |= HUD_ACTIVE;
	else
		m_iFlags &= ~HUD_ACTIVE;

	return 1;
}

int AgHudCTF::MsgFunc_CTFSound(const char *pszName, int iSize, void *pbuf) const
{
	BEGIN_READ(pbuf, iSize);

	int iSound = READ_BYTE();
	if (iSound >= 0 && iSound < sizeof(s_szSounds) / sizeof(s_szSounds[0]))
		gEngfuncs.pfnPlaySoundByName(s_szSounds[iSound], m_pCvarClCtfVolume->value);

	return 1;
}

int AgHudCTF::MsgFunc_CTFFlag(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	g_iPlayerFlag1 = READ_BYTE();
	g_iPlayerFlag2 = READ_BYTE();

	return 1;
}
