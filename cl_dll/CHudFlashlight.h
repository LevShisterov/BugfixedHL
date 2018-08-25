#ifndef CHUDFLASHLIGHT_H
#define CHUDFLASHLIGHT_H

#include "CHudBase.h"

class CHudFlashlight : public CHudBase
{
public:
	int Init(void);
	int VidInit(void);
	int Draw(float flTime);
	void Reset(void);
	int MsgFunc_Flashlight(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_FlashBat(const char *pszName, int iSize, void *pbuf);

private:
	HLHSPRITE m_hSprite1;
	HLHSPRITE m_hSprite2;
	HLHSPRITE m_hBeam;
	wrect_t *m_prc1;
	wrect_t *m_prc2;
	wrect_t *m_prcBeam;
	float m_flBat;
	int	  m_iBat;
	int	  m_fOn;
	float m_fFade;
	int	  m_iWidth;		// width of the battery innards
};

#endif