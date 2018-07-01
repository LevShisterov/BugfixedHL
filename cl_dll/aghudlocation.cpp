// Martin Webrant (BulliT)

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

#ifdef _WIN32
#include <windows.h>
#endif

DECLARE_MESSAGE(m_Location, Location)
DECLARE_MESSAGE(m_Location, InitLoc)

DECLARE_COMMAND(m_Location, AddLocation);
DECLARE_COMMAND(m_Location, DeleteLocation);
DECLARE_COMMAND(m_Location, ShowLocations);

int AgHudLocation::Init(void)
{
	m_fAt = 0;
	m_fNear = 0;
	m_iFlags = 0;
	m_szMap[0] = '\0';

	for (int i = 1; i <= MAX_PLAYERS; i++)
		m_vPlayerLocations[i] = Vector(0, 0, 0);

	gHUD.AddHudElem(this);

	HOOK_MESSAGE(Location);
	HOOK_MESSAGE(InitLoc);

	HOOK_COMMAND("agaddloc", AddLocation);
	HOOK_COMMAND("agdelloc", DeleteLocation);
	HOOK_COMMAND("aglistloc", ShowLocations);

	m_pCvarLocationKeywords = gEngfuncs.pfnRegisterVariable("cl_location_keywords", "0", FCVAR_ARCHIVE);

	return 1;
}

int AgHudLocation::VidInit(void)
{
	return 1;
}

void AgHudLocation::Reset(void)
{
	m_iFlags &= ~HUD_ACTIVE;
}

int AgHudLocation::Draw(float fTime)
{
	return 0;
}

void AgHudLocation::UserCmd_AddLocation()
{
	if (gEngfuncs.Cmd_Argc() != 2)
		return;

	char* locationName = gEngfuncs.Cmd_Argv(1);
	if (locationName[0] == 0)
		return;

	if (m_freeLocation == NULL)
	{
		ConsolePrint("Locations limit reached. Can't add new location.");
		return;
	}

	AgLocation* pLocation = m_firstLocation;
	while (pLocation->m_nextLocation != NULL)
		pLocation = pLocation->m_nextLocation;
	pLocation->m_nextLocation = m_freeLocation;
	m_freeLocation = m_freeLocation->m_nextLocation;
	pLocation = pLocation->m_nextLocation;
	pLocation->m_nextLocation = NULL;

	strncpy(pLocation->m_sLocation, locationName, AG_MAX_LOCATION_NAME - 1);
	pLocation->m_sLocation[AG_MAX_LOCATION_NAME - 1] = 0;
	//pLocation->m_vPosition = m_vPlayerLocations[gEngfuncs.GetLocalPlayer()->index];
	pLocation->m_vPosition = gEngfuncs.GetLocalPlayer()->attachment[0];
	pLocation->Show();

	Save();
	InitDistances();

	char szMsg[128];
	sprintf(szMsg, "Added Location %s.\n", (const char*)locationName);
	ConsolePrint(szMsg);
}

void AgHudLocation::UserCmd_DeleteLocation()
{
	if (gEngfuncs.Cmd_Argc() != 2)
		return;

	char* locationName = gEngfuncs.Cmd_Argv(1);
	if (locationName[0] == 0)
		return;

	AgLocation** prevNextLocPointer = &m_firstLocation;
	for (AgLocation* pLocation = m_firstLocation; pLocation != NULL; pLocation = pLocation->m_nextLocation)
	{
		if (_stricmp(pLocation->m_sLocation, locationName) != 0)
		{
			prevNextLocPointer = &pLocation->m_nextLocation;
			continue;
		}

		*prevNextLocPointer = pLocation->m_nextLocation;

		pLocation->m_nextLocation = m_freeLocation;
		m_freeLocation = pLocation;

		char szMsg[128];
		sprintf(szMsg, "Deleted Location %s.\n", (const char*)locationName);
		ConsolePrint(szMsg);

		Save();
		InitDistances();
		break;
	}
}

void AgHudLocation::UserCmd_ShowLocations()
{
	for (AgLocation* pLocation = m_firstLocation; pLocation != NULL; pLocation = pLocation->m_nextLocation)
	{
		char szMsg[128];
		sprintf(szMsg, "%s\n", (const char*)pLocation->m_sLocation);
		ConsolePrint(szMsg);
		pLocation->Show();
	}
}

void AgHudLocation::InitDistances()
{
	float fMinDistance = -1;
	float fMaxdistance = 0;

	// Calculate max/min distance between all locations.
	for (AgLocation* pLocation1 = m_firstLocation; pLocation1 != NULL; pLocation1 = pLocation1->m_nextLocation)
	{
		for (AgLocation* pLocation2 = m_firstLocation; pLocation2 != NULL; pLocation2 = pLocation2->m_nextLocation)
		{
			if (pLocation1 == pLocation2)
				continue;

			const float distance = (pLocation2->m_vPosition - pLocation1->m_vPosition).Length();

			if (distance < fMinDistance || fMinDistance == -1)
				fMinDistance = distance;

			if (distance > fMaxdistance)
				fMaxdistance = distance;
		}
	}

	// Now calculate when you are at/near a location or at atleast when its closest.
	m_fAt = fMinDistance / 2; // You are at a location if you are one fourth between to locations.
	m_fNear = fMinDistance / 1.1; // Over halfway of the mindistance you are at the "side".
}

void AgHudLocation::Load()
{
	m_firstLocation = NULL;
	m_freeLocation = &m_locations[0];
	for (unsigned int i = 0; i < HLARRAYSIZE(m_locations) - 1; i++)
	{
		m_locations[i].m_nextLocation = &m_locations[i + 1];
	}
	m_locations[HLARRAYSIZE(m_locations) - 1].m_nextLocation = NULL;

	char szData[8196];
	char szFile[MAX_PATH];
	const char *gameDirectory = gEngfuncs.pfnGetGameDirectory();
	sprintf(szFile, "%s/locs/%s.loc", gameDirectory, m_szMap);
	FILE* pFile = fopen(szFile, "r");
	if (!pFile)
	{
		// file error
		char szMsg[128];
		sprintf(szMsg, "Couldn't open location file %s.\n", szFile);
		ConsolePrint(szMsg);
		return;
	}

	const int iRead = fread(szData, sizeof(char), sizeof(szData) - 2, pFile);
	fclose(pFile);
	if (iRead <= 0)
		return;
	szData[iRead] = '\0';

	enum enumParseState { Location, X, Y, Z };
	enumParseState ParseState = Location;
	AgLocation* lastLocation = NULL;
	m_firstLocation = m_freeLocation;
	char* pszParse = strtok(szData, "#");
	if (pszParse)
	{
		while (pszParse)
		{
			switch (ParseState)
			{
			case Location:
				strncpy(m_freeLocation->m_sLocation, pszParse, AG_MAX_LOCATION_NAME - 1);
				m_freeLocation->m_sLocation[AG_MAX_LOCATION_NAME - 1] = 0;
				ParseState = X;
				break;
			case X:
				m_freeLocation->m_vPosition.x = atof(pszParse);
				ParseState = Y;
				break;
			case Y:
				m_freeLocation->m_vPosition.y = atof(pszParse);
				ParseState = Z;
				break;
			case Z:
				m_freeLocation->m_vPosition.z = atof(pszParse);
				ParseState = Location;
				lastLocation = m_freeLocation;
				m_freeLocation = m_freeLocation->m_nextLocation;
				break;
			}
			if (m_freeLocation == NULL)
				break;
			pszParse = strtok(NULL, "#");
		}
	}

	if (lastLocation == NULL)
		m_firstLocation = NULL;
	else
		lastLocation->m_nextLocation = NULL;

	InitDistances();
}

void AgHudLocation::Save()
{
	if (m_firstLocation == NULL)
		return;

	char szFile[MAX_PATH];
	const char *gameDirectory = gEngfuncs.pfnGetGameDirectory();
	sprintf(szFile, "%s/locs/%s.loc", gameDirectory, m_szMap);
	FILE* pFile = fopen(szFile, "wb");
	if (!pFile)
	{
		// file error
		char szMsg[128];
		sprintf(szMsg, "Couldn't create/save location file %s.\n", szFile);
		ConsolePrint(szMsg);
		return;
	}

	// Loop and write the file.
	for (AgLocation* pLocation = m_firstLocation; pLocation != NULL; pLocation = pLocation->m_nextLocation)
	{
		fprintf(pFile, "%s#%f#%f#%f#", (const char*)pLocation->m_sLocation, pLocation->m_vPosition.x, pLocation->m_vPosition.y, pLocation->m_vPosition.z);
	}

	fclose(pFile);
}

AgLocation* AgHudLocation::NearestLocation(const Vector& vPosition, float& fNearestDistance)
{
	fNearestDistance = -1;
	AgLocation* pLocation = NULL;

	for (AgLocation* pLocation1 = m_firstLocation; pLocation1 != NULL; pLocation1 = pLocation1->m_nextLocation)
	{
		float fDistance = (vPosition - pLocation1->m_vPosition).Length();

		if (fDistance < fNearestDistance || fNearestDistance == -1)
		{
			fNearestDistance = fDistance;
			pLocation = pLocation1;
		}
	}

	return pLocation;
}

char* AgHudLocation::FillLocation(const Vector& vPosition, char* pszSay, int pszSaySize)
{
	if (m_firstLocation == NULL)
		return pszSay;

	float fNearestDistance = 0;
	AgLocation* pLocation = NearestLocation(vPosition, fNearestDistance);

	if (pLocation == NULL)
		return pszSay;

#ifdef _DEBUG
	//pLocation->Show();
#endif

	if (fNearestDistance < m_fAt || m_pCvarLocationKeywords->value < 1)
	{
		return pszSay + snprintf(pszSay, pszSaySize, "%s", pLocation->m_sLocation);
	}
	if (fNearestDistance < m_fNear)
	{
		return pszSay + snprintf(pszSay, pszSaySize, "Near %s", pLocation->m_sLocation);
	}
	return pszSay + snprintf(pszSay, pszSaySize, "%s Side", pLocation->m_sLocation);
}

void AgHudLocation::ParseAndEditSayString(int iPlayer, char* pszSay, int pszSaySize)
{
	// Make backup
	char* pszText = (char*)malloc(strlen(pszSay) + 1);
	char* pszSayTemp = pszText;
	strcpy(pszText, pszSay);

	// Now parse for %L and edit it.
	char* pszSayEnd = pszSay + pszSaySize - 1;
	while (*pszSayTemp && pszSay < pszSayEnd) // Dont overflow the string.
	{
		if (*pszSayTemp == '%')
		{
			pszSayTemp++;
			if (*pszSayTemp == 'l' || *pszSayTemp == 'L' || *pszSayTemp == 'd' || *pszSayTemp == 'D')
			{
				pszSay = FillLocation(m_vPlayerLocations[iPlayer], pszSay, pszSayEnd - pszSay);
				pszSayTemp++;
				continue;
			}

			pszSay[0] = '%';
			pszSay++;
			continue;
		}

		*pszSay = *pszSayTemp;
		pszSay++;
		pszSayTemp++;
	}
	*pszSay = '\0';

	free(pszText);
}

int AgHudLocation::MsgFunc_Location(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);
	int iPlayer = READ_BYTE();
	for (int i = 0; i < 3; i++)
		m_vPlayerLocations[iPlayer][i] = READ_COORD();

	return 1;
}

int AgHudLocation::MsgFunc_InitLoc(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);
	strcpy(m_szMap, READ_STRING());
	Load();

	return 1;
}
