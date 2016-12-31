// Martin Webrant (BulliT)

#ifndef __AGHUDSETTINGS_H__
#define __AGHUDSETTINGS_H__

class AgHudSettings : public CHudBase
{
public:
	AgHudSettings() : m_flTurnoff(0), m_iMatch(0), m_iTimeLimit(0), m_iFragLimit(0), m_iFriendlyFire(0), m_iWeaponstay(0), m_pCvarHudSettings(NULL) { }

	int Init(void) override;
	int VidInit(void) override;
	int Draw(float flTime) override;
	void Reset(void) override;

	int MsgFunc_Settings(const char *pszName, int iSize, void *pbuf);

private:
	float m_flTurnoff;
	int m_iMatch;
	char m_szGamemode[16];
	int m_iTimeLimit;
	int m_iFragLimit;
	int m_iFriendlyFire;
	int m_iWeaponstay;
	char m_szVersion[8];

	char m_szWallgauss[8];
	char m_szHeadShot[8];
	char m_szBlastRadius[8];

	cvar_t *m_pCvarHudSettings;
};

bool AgIsMatch();

#endif // __AGHUDSETTINGS_H__
