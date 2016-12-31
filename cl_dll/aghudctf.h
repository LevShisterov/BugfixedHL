// Martin Webrant (BulliT)

#ifndef __AGHUDCTF_H__
#define __AGHUDCTF_H__

class AgHudCTF : public CHudBase
{
public:
	AgHudCTF(): m_iFlagStatus1(0), m_iFlagStatus2(0), m_pCvarClCtfVolume(NULL) { }

	int Init(void) override;
	int VidInit(void) override;
	int Draw(float flTime) override;
	void Reset(void) override;

	int MsgFunc_CTF(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_CTFSound(const char *pszName, int iSize, void *pbuf) const;
	static int MsgFunc_CTFFlag(const char *pszName, int iSize, void *pbuf);

private:
	enum enumFlagStatus { Off = -1, Home = 0, Stolen = 1, Lost = 2, Carry = 3 };

	typedef struct
	{
		HLHSPRITE spr;
		wrect_t rc;
	} icon_flagstatus_t;

	icon_flagstatus_t m_IconFlagStatus[4];
	int m_iFlagStatus1;
	int m_iFlagStatus2;

	cvar_t *m_pCvarClCtfVolume;
};

extern int g_iPlayerFlag1;
extern int g_iPlayerFlag2;

#endif // __AGHUDCTF_H__
