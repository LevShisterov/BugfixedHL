#ifndef CHUDSAYTEXT_H
#define CHUDSAYTEXT_H

#include "CHudBase.h"

class CHudSayText : public CHudBase
{
public:
	int Init(void);
	void InitHUDData(void);
	int VidInit(void);
	int Draw(float flTime);
	int MsgFunc_SayText(const char *pszName, int iSize, void *pbuf);
	void SayTextPrint(const char *pszBuf, int iBufSize, int clientIndex = -1);
	void EnsureTextFitsInOneLineAndWrapIfHaveTo(int line);
	friend class CHudSpectator;

private:
	struct cvar_s *	m_HUD_saytext;
	struct cvar_s *	m_HUD_saytext_time;
	cvar_t	*m_pCvarConSayColor;
};

#endif