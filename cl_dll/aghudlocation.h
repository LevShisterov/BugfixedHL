// Martin Webrant (BulliT)

#ifndef __AGHUDLOCATION_H__
#define __AGHUDLOCATION_H__

#include "aglocation.h"
#include <eiface.h>

class AgHudLocation : public CHudBase
{
public:
	AgHudLocation() : m_fAt(0), m_fNear(0), m_firstLocation(NULL),
					  m_freeLocation(m_locations), m_szMap{}, m_pCvarLocationKeywords(NULL)
	{
		for (unsigned int i = 0; i < HLARRAYSIZE(m_locations) - 1; i++)
		{
			m_locations[i].m_nextLocation = &m_locations[i + 1];
		}
		m_locations[HLARRAYSIZE(m_locations) - 1].m_nextLocation = NULL;
	}

	int Init(void) override;
	int VidInit(void) override;
	int Draw(float flTime) override;
	void Reset(void) override;

private:
	float m_fAt;
	float m_fNear;
	Vector m_vPlayerLocations[MAX_PLAYERS + 1];
	AgLocation m_locations[AG_MAX_LOCATIONS];
	AgLocation* m_firstLocation;
	AgLocation* m_freeLocation;

	AgLocation* NearestLocation(const Vector& vPosition, float& fNearestDistance);
	char* FillLocation(const Vector& vPosition, char* pszSay, int pszSaySize);

	void InitDistances();
	void Load();
	void Save();

public:
	char m_szMap[32];
	cvar_t *m_pCvarLocationKeywords;

	void ParseAndEditSayString(int iPlayer, char* pszSay, int pszSaySize);

	void UserCmd_AddLocation();
	void UserCmd_DeleteLocation();
	void UserCmd_ShowLocations();

	int MsgFunc_InitLoc(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_Location(const char *pszName, int iSize, void *pbuf);
};

#endif // __AGHUDLOCATION_H__
