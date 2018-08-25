#ifndef CHUDSTATUSBAR_H
#define CHUDSTATUSBAR_H

#include "CHudBase.h"

class CHudStatusBar : public CHudBase
{
public:
	int Init(void);
	int VidInit(void);
	int Draw(float flTime);
	void Reset(void);
	void ParseStatusString(int line_num);

	int MsgFunc_StatusText(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_StatusValue(const char *pszName, int iSize, void *pbuf);

protected:
	enum {
		MAX_STATUSTEXT_LENGTH = 128,
		MAX_STATUSBAR_VALUES = 8,
		MAX_STATUSBAR_LINES = 2,
	};

	char m_szStatusText[MAX_STATUSBAR_LINES][MAX_STATUSTEXT_LENGTH];  // a text string describing how the status bar is to be drawn
	char m_szStatusBar[MAX_STATUSBAR_LINES][MAX_STATUSTEXT_LENGTH];	// the constructed bar that is drawn
	int m_iStatusValues[MAX_STATUSBAR_VALUES];  // an array of values for use in the status bar

	int m_bReparseString; // set to TRUE whenever the m_szStatusBar needs to be recalculated

						  // an array of colors...one color for each line
	float *m_pflNameColors[MAX_STATUSBAR_LINES];
};

#endif