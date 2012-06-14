/***
*
*	Copyright (c) 2012, AGHL.RU. All rights reserved.
*
****/
//
// Memory.h
//
// Runtime memory searching/patching
//

#ifndef MEMORY_H
#define MEMORY_H

#ifdef _WIN32	//WINDOWS

typedef unsigned __int32 uint32_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int8 uint8_t;

#elif defined(linux)	//LINUX

typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;

#endif


extern void **g_EngineBuf;
extern int *g_EngineBufSize;
extern int *g_EngineReadPos;

typedef struct cl_enginemessages_s
{
	void				(*pnfSvcBad)			(void);
	void				(*pnfSvcNop)			(void);
	int		pfnUnks001[6];
	void				(*pfnSvcPrint)			(void);
	int		pfnUnks002[49];
} cl_enginemessages_t;


bool HookSvcMessages(cl_enginemessages_t *pEngineMessages);
bool UnHookSvcMessages(cl_enginemessages_t *pEngineMessages);

#endif MEMORY_H
