// Martin Webrant (BulliT)

#ifndef __AGHUDNEXTMAP_H__
#define __AGHUDNEXTMAP_H__

class AgHudNextmap : public CHudBase
{
public:
	AgHudNextmap() : m_flTurnoff(0) { }

	int Init(void) override;
	int VidInit(void) override;
	int Draw(float flTime) override;
	void Reset(void) override;

	int MsgFunc_Nextmap(const char *pszName, int iSize, void *pbuf);

private:
	float m_flTurnoff;
	char m_szNextmap[32];
};

#endif // __AGHUDNEXTMAP_H__
