#include "extdll.h"
#include	"util.h"
#include "gdll_rehlds_api.h"
#include "interface.h"

IRehldsApi* g_RehldsApi;
const RehldsFuncs_t* g_RehldsFuncs;
IRehldsHookchains* g_RehldsHookchains;
IRehldsServerStatic* g_RehldsSvs;
IRehldsServerData* g_RehldsSv;

bool GGll_Rehlds_API_Init() {
#ifdef WIN32
	CSysModule* engineModule = Sys_LoadModule("swds.dll");
#else
	CSysModule* engineModule = Sys_LoadModule("engine_i486.so");
#endif

	if (!engineModule) {
		ALERT(at_error, "Failed to locate engine module\n");
		return false;
	}

	CreateInterfaceFn ifaceFactory = Sys_GetFactory(engineModule);
	if (!ifaceFactory) {
		ALERT(at_error, "Failed to locate interface factory in engine module\n");
		return false;
	}

	int retCode = 0;
	g_RehldsApi = (IRehldsApi*)ifaceFactory(VREHLDS_HLDS_API_VERSION, &retCode);
	if (!g_RehldsApi) {
		ALERT(at_error, "Failed to locate retrieve rehlds api interface from engine module\n");
		return false;
	}

	int majorVersion = g_RehldsApi->GetMajorVersion();
	int minorVersion = g_RehldsApi->GetMinorVersion();

	if (majorVersion != REHLDS_API_VERSION_MAJOR) {
		ALERT(at_error, "REHLDS Api major version mismatch\n");
		return false;
	}

	if (minorVersion < REHLDS_API_VERSION_MINOR) {
		ALERT(at_error, "REHLDS Api minor version mismatch\n");
		return false;
	}

	g_RehldsFuncs = g_RehldsApi->GetFuncs();
	g_RehldsHookchains = g_RehldsApi->GetHookchains();
	g_RehldsSvs = g_RehldsApi->GetServerStatic();
	g_RehldsSv = g_RehldsApi->GetServerData();

	return true;

}