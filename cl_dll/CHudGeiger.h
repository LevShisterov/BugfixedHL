#ifndef CHUDGEIGER_H
#define CHUDGEIGER_H

#include "CHudBase.h"

class CHudGeiger : public CHudBase
{
public:
	int Init(void);
	int VidInit(void);
	int Draw(float flTime);
	int MsgFunc_Geiger(const char *pszName, int iSize, void *pbuf);

private:
	int m_iGeigerRange;

};

#endif