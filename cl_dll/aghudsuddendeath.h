// Martin Webrant (BulliT)

#ifndef __AGHUDSUDDENDEATH_H__
#define __AGHUDSUDDENDEATH_H__

class AgHudSuddenDeath : public CHudBase
{
public:
	AgHudSuddenDeath(): m_iSuddenDeath(0) { }

	int Init(void) override;
	int VidInit(void) override;
	int Draw(float flTime) override;
	void Reset(void) override;

	int MsgFunc_SuddenDeath(const char *pszName, int iSize, void *pbuf);

private:
	int m_iSuddenDeath;
};

#endif // __AGHUDSUDDENDEATH_H__
