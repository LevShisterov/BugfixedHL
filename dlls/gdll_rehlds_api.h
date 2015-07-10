#pragma once

#include "extdll.h"

#include "rehlds_api.h"


extern IRehldsApi* g_RehldsApi;
extern const RehldsFuncs_t* g_RehldsFuncs;
extern IRehldsHookchains* g_RehldsHookchains;
extern IRehldsServerStatic* g_RehldsSvs;
extern IRehldsServerData* g_RehldsSv;

extern bool GGll_Rehlds_API_Init();
