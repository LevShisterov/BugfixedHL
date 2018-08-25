#ifndef CHUDSTATUSICONS_H
#define CHUDSTATUSICONS_H

#include "CHudBase.h"

#define MAX_SPRITE_NAME_LENGTH		24
#define RESERVE_SPRITES_FOR_WEAPONS	32

class CHudStatusIcons : public CHudBase
{
public:
	int Init(void);
	int VidInit(void);
	void Reset(void);
	int Draw(float flTime);
	int MsgFunc_StatusIcon(const char *pszName, int iSize, void *pbuf);

	enum {
		MAX_ICONSPRITENAME_LENGTH = MAX_SPRITE_NAME_LENGTH,
		MAX_ICONSPRITES = 4,
	};


	//had to make these public so CHud could access them (to enable concussion icon)
	//could use a friend declaration instead...
	void EnableIcon(char *pszIconName, unsigned char red, unsigned char green, unsigned char blue);
	void DisableIcon(char *pszIconName);

private:

	typedef struct
	{
		char szSpriteName[MAX_ICONSPRITENAME_LENGTH];
		HLHSPRITE spr;
		wrect_t rc;
		unsigned char r, g, b;
	} icon_sprite_t;

	icon_sprite_t m_IconList[MAX_ICONSPRITES];

};

#endif