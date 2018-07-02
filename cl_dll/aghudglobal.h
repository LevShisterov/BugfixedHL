// Martin Webrant (BulliT)

#ifndef __AGHUDGLOBAL_H__
#define __AGHUDGLOBAL_H__

class AgHudGlobal : public CHudBase
{
public:
	int Init(void) override;
	int VidInit(void) override;
	int Draw(float flTime) override;
	void Reset(void) override;

	static int MsgFunc_PlaySound(const char *pszName, int iSize, void *pbuf);
	static int MsgFunc_CheatCheck(const char *pszName, int iSize, void *pbuf);
	static int MsgFunc_WhString(const char *pszName, int iSize, void *pbuf);
	static int MsgFunc_SpikeCheck(const char *pszName, int iSize, void *pbuf);
	static int MsgFunc_Gametype(const char *pszName, int iSize, void *pbuf);
	static int MsgFunc_AuthID(const char *pszName, int iSize, void *pbuf);
	static int MsgFunc_MapList(const char *pszName, int iSize, void *pbuf);
	static int MsgFunc_CRC32(const char *pszName, int iSize, void *pbuf);
	static int MsgFunc_Splash(const char* pszName, int iSize, void* pbuf);
};

enum GameTypes { GT_STANDARD = 0, GT_ARENA = 1, GT_LMS = 2, GT_CTF = 3, GT_ARCADE = 4, GT_SGBOW = 5, GT_INSTAGIB = 6 };
extern int g_GameType;
inline int AgGametype()
{
	return g_GameType;
};

int AgDrawHudString(int xpos, int ypos, int iMaxX, const char *szIt, int r, int g, int b);
int AgDrawHudStringCentered(int xpos, int ypos, int iMaxX, const char *szIt, int r, int g, int b);

#endif // __AGHUDGLOBAL_H__
