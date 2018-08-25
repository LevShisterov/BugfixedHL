#ifndef CHUDSCORES_H
#define CHUDSCORES_H

#include "CHudBase.h"

struct HudScoresData
{
	char szScore[64];
	int r, g, b;
};

class CHudScores : public CHudBase
{
public:
	int Init(void);
	int VidInit(void);
	int Draw(float flTime);

private:
	HudScoresData m_ScoresData[MAX_PLAYERS] = {};
	int m_iLines = 0;
	int m_iOverLay = 0;
	float m_flScoreBoardLastUpdated = 0;
	cvar_t* m_pCvarHudScores = nullptr;
	cvar_t* m_pCvarHudScoresPos = nullptr;
};

#endif