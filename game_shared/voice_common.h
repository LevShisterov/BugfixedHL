//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef VOICE_COMMON_H
#define VOICE_COMMON_H
#ifdef _WIN32
#ifndef __MINGW32__
#pragma once
#endif /* not __MINGW32__ */
#endif


#include "bitvec.h"

// Shared header between the client DLL and the game DLLs
#include "cdll_dll.h"


#define VOICE_MAX_PLAYERS_DW	((MAX_PLAYERS / 32) + !!(MAX_PLAYERS & 31))

typedef CBitVec<MAX_PLAYERS> CPlayerBitVec;


#endif // VOICE_COMMON_H
