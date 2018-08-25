#ifndef CHUDMENU
#define CHUDMENU

#include "CHudBase.h"

class CHudMenu : public CHudBase
{
public:
	int Init(void);
	void InitHUDData(void);
	int VidInit(void);
	void Reset(void);
	int Draw(float flTime);
	int MsgFunc_ShowMenu(const char *pszName, int iSize, void *pbuf);

	void SelectMenuItem(int menu_item);

	int m_fMenuDisplayed;
	int m_bitsValidSlots;
	float m_flShutoffTime;
	int m_fWaitingForMore;
};

#endif