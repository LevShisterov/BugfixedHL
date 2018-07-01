// Martin Webrant (BulliT)

#ifndef __AGHUDTIMEOUT_H__
#define __AGHUDTIMEOUT_H__

class AgHudTimeout : public CHudBase
{
public:
	AgHudTimeout() : m_State(0), m_iTime(0) { }

	int Init(void) override;
	int VidInit(void) override;
	int Draw(float flTime) override;
	void Reset(void) override;

	int MsgFunc_Timeout(const char *pszName, int iSize, void *pbuf);

private:
	enum enumState { Inactive = 0, Called = 1, Pause = 2, Countdown = 3 };

	int m_State;
	int m_iTime;
};

#endif // __AGHUDTIMEOUT_H__
