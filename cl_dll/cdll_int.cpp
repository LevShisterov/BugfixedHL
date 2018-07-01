/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
//
//  cdll_int.c
//
// this implementation handles the linking of the engine to the DLL
//

#include <string.h>
#include <windows.h>
#include <psapi.h>
#include <dbghelp.h>

#include "hud.h"
#include "cl_util.h"
#include "netadr.h"
#include "vgui_schememanager.h"
#include "GameStudioModelRenderer.h"

extern "C"
{
#include "pm_shared.h"
}

#include "hud_servers.h"
#include "vgui_int.h"
#include "interface.h"
#include "svc_messages.h"
#include "memory.h"
#include "results.h"

#define DLLEXPORT __declspec( dllexport )


cl_enginefunc_t gEngfuncs;
CHud gHUD;
TeamFortressViewport *gViewPort = NULL;
PVOID hVehHandler = NULL;
bool g_bDllDetaching = false;

void InitInput (void);
void ShutdownInput (void);
void EV_HookEvents( void );
void IN_Commands( void );
int HUD_IsGame(const char *game);

/*
========================== 
	Initialize

Called when the DLL is first loaded.
==========================
*/
extern "C" 
{
int		DLLEXPORT Initialize( cl_enginefunc_t *pEnginefuncs, int iVersion );
int		DLLEXPORT HUD_VidInit( void );
void	DLLEXPORT HUD_Init( void );
int		DLLEXPORT HUD_Redraw( float flTime, int intermission );
int		DLLEXPORT HUD_UpdateClientData( client_data_t *cdata, float flTime );
void	DLLEXPORT HUD_Reset ( void );
void	DLLEXPORT HUD_Shutdown( void );
void	DLLEXPORT HUD_PlayerMove( struct playermove_s *ppmove, int server );
void	DLLEXPORT HUD_PlayerMoveInit( struct playermove_s *ppmove );
char	DLLEXPORT HUD_PlayerMoveTexture( char *name );
int		DLLEXPORT HUD_ConnectionlessPacket( const struct netadr_s *net_from, const char *args, char *response_buffer, int *response_buffer_size );
int		DLLEXPORT HUD_GetHullBounds( int hullnumber, float *mins, float *maxs );
void	DLLEXPORT HUD_Frame( double time );
void	DLLEXPORT HUD_VoiceStatus(int entindex, qboolean bTalking);
void	DLLEXPORT HUD_DirectorMessage( int iSize, void *pbuf );
}

/*
================================
HUD_GetHullBounds

  Engine calls this to enumerate player collision hulls, for prediction.  Return 0 if the hullnumber doesn't exist.
================================
*/
int DLLEXPORT HUD_GetHullBounds( int hullnumber, float *mins, float *maxs )
{
	int iret = 0;

	switch ( hullnumber )
	{
	case 0:				// Normal player
		mins = Vector(-16, -16, -36);
		maxs = Vector(16, 16, 36);
		iret = 1;
		break;
	case 1:				// Crouched player
		mins = Vector(-16, -16, -18 );
		maxs = Vector(16, 16, 18 );
		iret = 1;
		break;
	case 2:				// Point based hull
		mins = Vector( 0, 0, 0 );
		maxs = Vector( 0, 0, 0 );
		iret = 1;
		break;
	}

	return iret;
}

/*
================================
HUD_ConnectionlessPacket

 Return 1 if the packet is valid.  Set response_buffer_size if you want to send a response packet.  Incoming, it holds the max
  size of the response_buffer, so you must zero it out if you choose not to respond.
================================
*/
int	DLLEXPORT HUD_ConnectionlessPacket( const struct netadr_s *net_from, const char *args, char *response_buffer, int *response_buffer_size )
{
	// Parse stuff from args
	int max_buffer_size = *response_buffer_size;

	// Zero it out since we aren't going to respond.
	// If we wanted to response, we'd write data into response_buffer
	*response_buffer_size = 0;

	// Since we don't listen for anything here, just respond that it's a bogus message
	// If we didn't reject the message, we'd return 1 for success instead.
	return 0;
}

void DLLEXPORT HUD_PlayerMoveInit( struct playermove_s *ppmove )
{
	PM_Init( ppmove );
}

char DLLEXPORT HUD_PlayerMoveTexture( char *name )
{
	return PM_FindTextureType( name );
}

void DLLEXPORT HUD_PlayerMove( struct playermove_s *ppmove, int server )
{
	PM_Move( ppmove, server );
}


// Vectored Exceptions Handler
LONG NTAPI VectoredExceptionsHandler(PEXCEPTION_POINTERS pExceptionInfo)
{
	long exceptionCode = pExceptionInfo->ExceptionRecord->ExceptionCode;
	long exceptionAddress = (long)pExceptionInfo->ExceptionRecord->ExceptionAddress;

	if (exceptionCode == 0xE06D7363)	// SEH
		return EXCEPTION_CONTINUE_SEARCH;

	// We will handle all fatal unexpected exceptions, like STATUS_ACCESS_VIOLATION
	// But skip DLL Not Found exception, which happen on old non-steam when steam is running
	// Also skip while detach is in process, cos we can't write files (not sure about message boxes, but anyway...)
	if ((exceptionCode & 0xF0000000L) == 0xC0000000L && exceptionCode != 0xC0000139 && !g_bDllDetaching)
	{
		char buffer[1024];
		long moduleBase, moduleSize;

		HANDLE hProcess = GetCurrentProcess();
		MODULEINFO moduleInfo;

		// Get modules info
		HMODULE hMods[1024];
		DWORD cbNeeded;
		EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded);
		int count = cbNeeded / sizeof(HMODULE);

		// Write exception info to log
		FILE *file = fopen("crash.log", "a");
		if (file)
		{
			fputs("------------------------------------------------------------\n", file);
			sprintf(buffer, "Exception 0x%08X at address 0x%08X.\n", exceptionCode, exceptionAddress);
			fputs(buffer, file);

			// Dump modules info
			fputs("Modules:\n", file);
			fputs("  Base     Size     Path (Exception Offset)\n", file);
			for (int i = 0; i < count; i++)
			{
				GetModuleInformation(hProcess, hMods[i], &moduleInfo, sizeof(moduleInfo));
				moduleBase = (long)moduleInfo.lpBaseOfDll;
				moduleSize = (long)moduleInfo.SizeOfImage;
				// Get the full path to the module's file.
				TCHAR szModName[MAX_PATH];
				if (GetModuleFileNameEx(hProcess, hMods[i], szModName, sizeof(szModName)/sizeof(TCHAR)))
				{
					if (moduleBase <= exceptionAddress && exceptionAddress <= (moduleBase + moduleSize))
						sprintf(buffer, "=>%08X %08X %s  <==  %08X\n", moduleBase, moduleSize, szModName, exceptionAddress - moduleBase);
					else
						sprintf(buffer, "  %08X %08X %s\n", moduleBase, moduleSize, szModName);
				}
				else
				{
					if (moduleBase <= exceptionAddress && exceptionAddress <= (moduleBase + moduleSize))
						sprintf(buffer, "=>%08X %08X  <==  %08X\n", moduleBase, moduleSize, exceptionAddress - moduleBase);
					else
						sprintf(buffer, "  %08X %08X\n", moduleBase, moduleSize);
				}
				fputs(buffer, file);
			}

			fclose(file);
		}

		// Create mini-dump
		HANDLE hMiniDumpFile = CreateFile("crash.dmp", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH, NULL);
		if (hMiniDumpFile != INVALID_HANDLE_VALUE)
		{
			MINIDUMP_EXCEPTION_INFORMATION eInfo;
			eInfo.ThreadId = GetCurrentThreadId();
			eInfo.ExceptionPointers = pExceptionInfo;
			eInfo.ClientPointers = FALSE;
			MiniDumpWriteDump(hProcess, GetCurrentProcessId(), hMiniDumpFile, MiniDumpNormal, &eInfo, NULL, NULL);
			CloseHandle(hMiniDumpFile);
		}

		// Display a message
		HMODULE hModuleDll = GetModuleHandle("client.dll");
		GetModuleInformation(hProcess, hModuleDll, &moduleInfo, sizeof(moduleInfo));
		moduleBase = (long)moduleInfo.lpBaseOfDll;
		moduleSize = (long)moduleInfo.SizeOfImage;
		if (moduleBase <= exceptionAddress && exceptionAddress <= (moduleBase + moduleSize))
		{
			sprintf(buffer, "Exception in client.dll at offset 0x%08X.\n\nCrash dump and log files were created in game directory.\nPlease report them to http://aghl.ru/forum.", exceptionAddress - moduleBase);
		}
		else
		{
			sprintf(buffer, "Exception in the game.\n\nCrash dump and log files were created in game directory.\nPlease report them to http://aghl.ru/forum.");
		}
		MessageBox(GetActiveWindow(), buffer, "Error!", MB_OK | MB_ICONEXCLAMATION | MB_SETFOREGROUND | MB_TOPMOST);

		// Application will die anyway, so futher exceptions are not interesting to us
		RemoveVectoredExceptionHandler(hVehHandler);
	}

	return EXCEPTION_CONTINUE_SEARCH;
}

// DLL entry point
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		if (hVehHandler == NULL)
			hVehHandler = AddVectoredExceptionHandler(1, VectoredExceptionsHandler);

		PatchEngine();
		HookSvcMessages();
	}
	else if (fdwReason == DLL_PROCESS_DETACH)
	{
		g_bDllDetaching = true;

		UnHookSvcMessages();
		UnPatchEngine();
		StopServerBrowserThreads();

		if (hVehHandler != NULL)
		{
			RemoveVectoredExceptionHandler(hVehHandler);
			hVehHandler = NULL;
		}
	}
	return TRUE;
}

int DLLEXPORT Initialize( cl_enginefunc_t *pEnginefuncs, int iVersion )
{
	if (iVersion != CLDLL_INTERFACE_VERSION)
		return 0;

	gEngfuncs = *pEnginefuncs;

	EV_HookEvents();

	g_iIsAg = HUD_IsGame("ag");

	return 1;
}


/*
==========================
	HUD_VidInit

Called whenever the client connects to a server.
==========================
*/

int DLLEXPORT HUD_VidInit( void )
{
	gHUD.VidInit();
	VGui_Startup();
	g_StudioRenderer.InitOnConnect();
	ResultsStop();
	return 1;
}

/*
==========================
	HUD_Init

Called when the game initializes and whenever the vid_mode is changed so the HUD can reinitialize itself.
Reinitializes all the hud variables.
==========================
*/

void DLLEXPORT HUD_Init( void )
{
	InitInput();
	gHUD.Init();
	Scheme_Init();
	MemoryPatcherInit();
	SvcMessagesInit();
	ResultsInit();
	SetAffinity();
}


/*
==========================
	HUD_Redraw

called every screen frame to
redraw the HUD.
===========================
*/

int DLLEXPORT HUD_Redraw( float time, int intermission )
{
	gHUD.Redraw( time, intermission );

	return 1;
}


/*
==========================
	HUD_UpdateClientData

called every time shared client
dll/engine data gets changed,
and gives the cdll a chance
to modify the data.

returns 1 if anything has been changed, 0 otherwise.
==========================
*/

int DLLEXPORT HUD_UpdateClientData(client_data_t *pcldata, float flTime )
{
	IN_Commands();

	return gHUD.UpdateClientData(pcldata, flTime );
}

/*
==========================
	Obsolete: HUD_Reset

Obsolete: Called at start and end of demos to restore to "non"HUD state.
Obsolete: Doesn't called anymore from the engine.
==========================
*/

void DLLEXPORT HUD_Reset( void )
{
	gHUD.VidInit();
}

/*
==========================
	HUD_Shutdown

Called at game exit.
==========================
*/

void DLLEXPORT HUD_Shutdown( void )
{
	ShutdownInput();
}

/*
==========================
HUD_Frame

Called by engine every frame that client .dll is loaded
==========================
*/

void DLLEXPORT HUD_Frame( double time )
{
	MemoryPatcherHudFrame();
	ResultsFrame(time);

	ServersThink( time );

	GetClientVoiceMgr()->Frame(time);
}


/*
==========================
HUD_VoiceStatus

Called when a player starts or stops talking.
==========================
*/

void DLLEXPORT HUD_VoiceStatus(int entindex, qboolean bTalking)
{
	GetClientVoiceMgr()->UpdateSpeakerStatus(entindex, bTalking);
}

/*
==========================
HUD_DirectorEvent

Called when a director event message was received
==========================
*/

void DLLEXPORT HUD_DirectorMessage( int iSize, void *pbuf )
{
	 gHUD.m_Spectator.DirectorMessage( iSize, pbuf );
}


