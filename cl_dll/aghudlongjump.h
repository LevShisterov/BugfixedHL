// Martin Webrant (BulliT)

#ifndef __AGHUDLONGJUMP_H__
#define __AGHUDLONGJUMP_H__

class AgHudLongjump : public CHudBase
{
public:
	AgHudLongjump() : m_flTurnoff(0) { }

	int Init(void) override;
	int VidInit(void) override;
	int Draw(float flTime) override;
	void Reset(void) override;

	int MsgFunc_Longjump(const char *pszName, int iSize, void *pbuf);

private:
	float m_flTurnoff;
};

#endif // __AGHUDLONGJUMP_H__
