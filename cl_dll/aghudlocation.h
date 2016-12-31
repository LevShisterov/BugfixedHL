// Martin Webrant (BulliT)

#ifndef __AGHUDLOCATION_H__
#define __AGHUDLOCATION_H__

#include "aglocation.h"

class AgHudLocation : public CHudBase
{
public:
	AgHudLocation(): m_fAt(0), m_fNear(0) { }

	int Init(void) override;
	int VidInit(void) override;
	int Draw(float flTime) override;
	void Reset(void) override;

private:
	float m_fAt;
	float m_fNear;
	Vector m_vPlayerLocations[MAX_PLAYERS + 1];

public:
	char m_szMap[32];

	int MsgFunc_InitLoc(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_Location(const char *pszName, int iSize, void *pbuf);
};

#endif // __AGHUDLOCATION_H__
