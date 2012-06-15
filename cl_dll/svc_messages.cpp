/***
*
*	Copyright (c) 2012, AGHL.RU. All rights reserved.
*
****/
//
// Svc_messages.cpp
//
// Engine messages handlers
//

#include <string.h>

#include "hud.h"
#include "memory.h"
#include "parsemsg.h"
#include "svc_messages.h"

cl_enginemessages_t pEngineMessages;


void SvcPrint(void)
{
	BEGIN_READ( *g_EngineBuf, *g_EngineBufSize, *g_EngineReadPos );
	char *str = READ_STRING();

	if (!strncmp(str, "\"mp_timelimit\" changed to \"", 27))
	{
		gHUD.m_Timer.DoReSync();
	}

	pEngineMessages.pfnSvcPrint();
}

bool HookSvcMessages(void)
{
	memset(&pEngineMessages, 0, sizeof(cl_enginemessages_t));
	pEngineMessages.pfnSvcPrint = SvcPrint;
	return HookSvcMessages(&pEngineMessages);
}

bool UnHookSvcMessages(void)
{
	return UnHookSvcMessages(&pEngineMessages);
}
