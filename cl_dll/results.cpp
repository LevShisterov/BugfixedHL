/***
*
*	Copyright (c) 2012, AGHL.RU. All rights reserved.
*
****/
//
// Results.cpp
//
// Functions for storing game results files.
//

#include <crtdbg.h>
#include <time.h>
#include <io.h>

#include "hud.h"
#include "cl_util.h"
#include "cvardef.h"
#include "com_utils.h"
#include "path.h"
#include "demo_api.h"
#include "vgui_TeamFortressViewport.h"
#include "memory.h"


bool g_bFormatError = false;
cvar_t *m_pCvarResultsFileFormat, *m_pCvarResultsCounterFormat;
cvar_t *m_pCvarResultsDemoAutorecord, *m_pCvarResultsDemoKeepDays;
cvar_t *m_pCvarResultsLogChat, *m_pCvarResultsLogOther;
char g_szModDirectory[MAX_PATH];
char g_szTempDemoList[MAX_PATH];
char g_szCurrentMap[MAX_PATH];
char g_szCurrentResultsDemo[MAX_PATH];
char g_szCurrentResultsLog[MAX_PATH];
char g_szCurrentResultsStats[MAX_PATH];
char g_szDefaultFilenameFormat[] = "results\\%Y-%m\\%l-%Y%m%d-%H%M%S";
char g_szDefaultCounterFormat[] = "-%03d";
bool g_bDemoRecording = false;
bool g_bDemoRecordingStartIssued = false;
bool g_bResultsStarted = false;
FILE *g_pLogFile = NULL;
int g_iLastMaxClients = 0;
int g_bDemoRecordingFrame = 0;


void InvalidParameterHandler(const wchar_t *expression, const wchar_t *function, const wchar_t *file, unsigned int line, uintptr_t pReserved)
{
	g_bFormatError = true;
}

bool DoSubstitutions(char filename[MAX_PATH], char fullpath[MAX_PATH], const char *format, const char *map, tm *pTm)
{
	// Substitute map name
	char frmt[MAX_PATH];
	strncpy(frmt, format, MAX_PATH);
	frmt[MAX_PATH - 1] = 0;
	strrepl(frmt, MAX_PATH, "%l", map);

	// Substitute datetime parts
	g_bFormatError = false;
	_invalid_parameter_handler oldHandler = _set_invalid_parameter_handler(InvalidParameterHandler);
	int oldMode = _CrtSetReportMode(_CRT_ASSERT, 0);
	strftime(filename, MAX_PATH, frmt, pTm);
	_CrtSetReportMode(_CRT_ASSERT, oldMode);
	_set_invalid_parameter_handler(oldHandler);

	// Use default pattern on format error
	if (g_bFormatError)
	{
		g_bFormatError = false;
		return false;
	}

	// Perform filename validation
	RemoveInvalidPathChars(filename, false);

	// Construct full path
	sprintf_s(fullpath, MAX_PATH, "%s\\%s", g_szModDirectory, filename);
	GetFullPathName(fullpath, MAX_PATH, fullpath, NULL);

	// Check that path lie within mod directory for security reasons
	if (strncmp(fullpath, g_szModDirectory, strlen(g_szModDirectory)))
		return false;
	return true;
}

// Returns new results file name in relative to mod directory and rooted formats.
bool GetResultsFilename(const char *extension, char filename[MAX_PATH], char fullpath[MAX_PATH])
{
	// Get map name
	if (g_szCurrentMap[0] == 0)
	{
		strncpy(g_szCurrentMap, gEngfuncs.pfnGetLevelName(), MAX_PATH);
		g_szCurrentMap[MAX_PATH - 1] = 0;
		if (!g_szCurrentMap[0])
		{
			strcpy(g_szCurrentMap, "none");
		}
		else
		{
			char *s = strrchr(g_szCurrentMap, '\\');
			if (!s)
				s = strrchr(g_szCurrentMap, '/');
			else
				s++;
			if (!s)
				s = g_szCurrentMap;
			else
				s++;
			strrepl(s, MAX_PATH, ".bsp", "");
			strcpy(g_szCurrentMap, s);
		}
		strrepl(g_szCurrentMap, MAX_PATH, "%", "");
	}

	// Get time
	time_t t = time(NULL);
	tm *pTm = localtime(&t);

	// Substitute values
	if (!DoSubstitutions(filename, fullpath, m_pCvarResultsFileFormat->string, g_szCurrentMap, pTm))
	{
		gEngfuncs.Con_Printf("results_file_format contains invalid path, using default format instead.\n");
		if (!DoSubstitutions(filename, fullpath, g_szDefaultFilenameFormat, g_szCurrentMap, pTm))
		{
			gEngfuncs.Con_Printf("Couldn't construct filepath for game results file using default file format.\n");
			filename[0] = 0;
			fullpath[0] = 0;
			return false;
		}
	}

	// Get directory path
	char path[MAX_PATH];
	strcpy(path, fullpath);
	*strrchr(path, '\\') = 0;

	// Ensure directory
	if (GetFileAttributes(path) == INVALID_FILE_ATTRIBUTES)
	{
		CreateDirectoryFull(path);
	}

	// Construct file name without counter
	g_bFormatError = false;
	_invalid_parameter_handler oldHandler = _set_invalid_parameter_handler(InvalidParameterHandler);
	int oldMode = _CrtSetReportMode(_CRT_ASSERT, 0);
	char file[MAX_PATH];
	sprintf_s(file, MAX_PATH, "%s.%s", filename, extension);
	sprintf_s(path, MAX_PATH, "%s.%s", fullpath, extension);
	if (g_bFormatError)
	{
		g_bFormatError = false;
		gEngfuncs.Con_Printf("Couldn't construct filepath for game results file: check results_file_format, may be it is too long.\n");
		filename[0] = 0;
		fullpath[0] = 0;
		_CrtSetReportMode(_CRT_ASSERT, oldMode);
		_set_invalid_parameter_handler(oldHandler);
		return false;
	}

	// Do not return the same file name twice
	static char prevPath[MAX_PATH];
	static int prevCounter = 0;
	bool matchedPrev = false;
	if (strcmp(path, prevPath) == 0)
	{
		matchedPrev = true;
	}
	else
	{
		strncpy(prevPath, path, MAX_PATH - 1);
		prevPath[MAX_PATH - 1] = 0;
		prevCounter = 0;
	}

	// Check for existing file and apply counter
	if (matchedPrev || GetFileAttributes(path) != INVALID_FILE_ATTRIBUTES)
	{
		char counter[MAX_PATH];
		strcpy(counter, m_pCvarResultsCounterFormat->string);
		char *c = counter;
		bool parameter = false;
		while (*c)
		{
			if (*c == '%' && parameter)
				break;
			if (*c == '%')
			{
				parameter = true;
				if (!(c[1] == '0' && c[2] >= '1' && c[2] <= '3' && c[3] == 'd' ||
					c[1] >= '1' && c[1] <= '3' && c[2] == 'd' ||
					c[1] == 'd'))
					break;
			}
			c++;
		}
		char frmt[MAX_PATH];
		if (*c)
			sprintf_s(frmt, MAX_PATH, "%%s%s.%%s", g_szDefaultCounterFormat);
		else
			sprintf_s(frmt, MAX_PATH, "%%s%s.%%s", m_pCvarResultsCounterFormat->string);
		int i = matchedPrev ? prevCounter + 1 : 1;
		for (; i < 1000; i++)
		{
			sprintf_s(file, MAX_PATH, frmt, filename, i, extension);
			sprintf_s(path, MAX_PATH, frmt, fullpath, i, extension);
			// Use default pattern on format error
			if (g_bFormatError)
			{
				g_bFormatError = false;
				gEngfuncs.Con_Printf("Couldn't construct filepath for game results file using results_counter_format.\n");
				sprintf_s(frmt, MAX_PATH, "%%s%s.%%s", g_szDefaultCounterFormat);
				sprintf_s(file, MAX_PATH, frmt, filename, i, extension);
				sprintf_s(path, MAX_PATH, frmt, fullpath, i, extension);
				if (g_bFormatError)
				{
					g_bFormatError = false;
					gEngfuncs.Con_Printf("Couldn't construct filepath for game results file using default counter format.\n");
					filename[0] = 0;
					fullpath[0] = 0;
					_CrtSetReportMode(_CRT_ASSERT, oldMode);
					_set_invalid_parameter_handler(oldHandler);
					return false;
				}
			}
			if (GetFileAttributes(path) == INVALID_FILE_ATTRIBUTES)
				break;
		}
		if (i == 1000)
		{
			gEngfuncs.Con_Printf("Couldn't construct filepath for game results file: counter exausted, fix results_file_format.\n");
			filename[0] = 0;
			fullpath[0] = 0;
			_CrtSetReportMode(_CRT_ASSERT, oldMode);
			_set_invalid_parameter_handler(oldHandler);
			return false;
		}
		prevCounter = i;
	}
	_CrtSetReportMode(_CRT_ASSERT, oldMode);
	_set_invalid_parameter_handler(oldHandler);

	strcpy(filename, file);
	strcpy(fullpath, path);
	return true;
}

// Opens log file and adds text there if results are started.
void ResultsAddLog(const char *text, bool chat)
{
	// Check that we need to log this type of event
	if (chat && m_pCvarResultsLogChat->value == 0 ||
		!chat && m_pCvarResultsLogOther->value == 0)
		return;

	// Check that results are started
	if (!g_bResultsStarted)
		return;

	// Open log file in case it is not already
	if (!g_pLogFile)
		g_pLogFile = fopen(g_szCurrentResultsLog, "a+b");
	if (!g_pLogFile)
		return;

	int len = strlen(text);
	if (text[len - 1] == '\n')
	{
		*((char *)&text[len - 1]) = 0;
		fprintf(g_pLogFile, "%s\r\n", text);
	}
	else
	{
		fprintf(g_pLogFile, "%s", text);
	}
	fflush(g_pLogFile);
}

// Closes opened results files.
void ResultsCloseFiles()
{
	if (g_pLogFile)
	{
		fclose(g_pLogFile);
		g_pLogFile = NULL;
	}
}

// Starts demo recording.
void ResultsStartDemoRecord()
{
	if (g_bDemoRecording)
	{
		gEngfuncs.Con_Printf("Already recording.\n");
		return;
	}

	char temp[MAX_PATH];
	GetResultsFilename("dem", g_szCurrentResultsDemo, temp);
	g_bFormatError = false;
	_invalid_parameter_handler oldHandler = _set_invalid_parameter_handler(InvalidParameterHandler);
	int oldMode = _CrtSetReportMode(_CRT_ASSERT, 0);
	sprintf_s(temp, MAX_PATH, "record %s\n", g_szCurrentResultsDemo);
	if (!g_bFormatError)
	{
		ClientCmd(temp);
		g_bDemoRecording = true;
		g_bDemoRecordingFrame = 0;
		// Write autodemo file name to the temporary list
		FILE *file = fopen(g_szTempDemoList, "a+b");
		if (file)
		{
			time_t now;
			time(&now);
			struct tm *current = localtime(&now);
			char time_buf[32];
			sprintf(time_buf, "[%04i-%02i-%02i %02i:%02i:%02i] ", current->tm_year + 1900, current->tm_mon + 1, current->tm_mday, current->tm_hour, current->tm_min, current->tm_sec);
			fprintf(file, "%s%s\r\n", time_buf, g_szCurrentResultsDemo);
			fclose(file);
		}
	}
	_CrtSetReportMode(_CRT_ASSERT, oldMode);
	_set_invalid_parameter_handler(oldHandler);
}

// Purges old demos.
void ResultsPurgeDemos()
{
	char buf[512], fileName[MAX_PATH];
	int readPos = 0, writePos = 0;
	bool deleteRow = true;

	time_t now;
	time(&now);
	now -= m_pCvarResultsDemoKeepDays->value * 24 * 60 * 60;	// days to keep

	FILE *file = fopen(g_szTempDemoList, "r+b");
	if (file)
	{
		while (fgets(buf, sizeof(buf), file) != NULL)
		{
			deleteRow = true;
			if (buf[0] == '[' && buf[20] == ']')
			{
				buf[20] = 0;
				struct tm inTm;
				memset(&inTm, 0, sizeof(inTm));
				g_bFormatError = false;
				_invalid_parameter_handler oldHandler = _set_invalid_parameter_handler(InvalidParameterHandler);
				int oldMode = _CrtSetReportMode(_CRT_ASSERT, 0);
				sscanf(buf + 1, "%4u-%2u-%2u %2u:%2u:%2u", &(inTm.tm_year), &(inTm.tm_mon), &(inTm.tm_mday), &(inTm.tm_hour), &(inTm.tm_min), &(inTm.tm_sec));
				_CrtSetReportMode(_CRT_ASSERT, oldMode);
				_set_invalid_parameter_handler(oldHandler);
				if (!g_bFormatError)
				{
					inTm.tm_year -= 1900;
					inTm.tm_mon -= 1;
					time_t inTime = mktime(&inTm);
					if (inTime < now)
					{
						char *fname = &buf[22];
						int len = strlen(fname) - 1;
						while (len >= 0 && fname[len] == '\r' || fname[len] == '\n')
						{
							fname[len--] = 0;
						}
						sprintf_s(fileName, MAX_PATH, "%s\\%s", g_szModDirectory, fname);
						if (_access(fileName, 0) != -1)
						{
							remove(fileName);
						}
					}
					else
					{
						deleteRow = false;
					}
				}
			}
			// Remove row
			if (deleteRow)
			{
				readPos = ftell(file);
				continue;
			}
			// Place string back if we keep it
			if (readPos != writePos)
			{
				buf[20] = ']';
				readPos = ftell(file);
				fseek(file, writePos, SEEK_SET);
				fputs(buf, file);
				writePos = ftell(file);
				fseek(file, readPos, SEEK_SET);
			}
			else
			{
				readPos = ftell(file);
				writePos = readPos;
			}
		}
		fseek(file, writePos, SEEK_SET);
		_chsize(_fileno(file), writePos);
		fclose(file);
	}
}

// Start results.
void ResultsStart(void)
{
	g_bResultsStarted = true;

	// Prepare file names
	char temp[MAX_PATH];
	GetResultsFilename("log", temp, g_szCurrentResultsLog);
	GetResultsFilename("txt", temp, g_szCurrentResultsStats);
}

// Stops log and demo recordings.
void ResultsStop(void)
{
	g_bResultsStarted = false;

	// Clear map name
	g_szCurrentMap[0] = 0;

	// Close chat log and other files we may have opened
	ResultsCloseFiles();

	// Stop demo recording if we started it
	if (g_bDemoRecording && gEngfuncs.pDemoAPI->IsRecording())
	{
		ClientCmd("stop\n");
	}
	g_bDemoRecording = false;
	g_bDemoRecordingStartIssued = false;

	// Clear file names
	g_szCurrentResultsDemo[0] = 0;
	g_szCurrentResultsLog[0] = 0;
	g_szCurrentResultsStats[0] = 0;
}

// Monitors MaxClients changes to do results stopping, also monitors demo recording.
void ResultsFrame(double time)
{
	int maxClients = gEngfuncs.GetMaxClients();
	if (maxClients != g_iLastMaxClients)
	{
		// Store maxClients value for later usages in other functions
		g_iLastMaxClients = maxClients;

		// Connection state changed, do results stop, besides if they are started (this is needed due to agrecord command)
		ResultsStop();
		// Startup results here, but without demo recording
		if (maxClients > 1)
		{
			ResultsStart();
		}
	}

	// Demo watchdog: If user stopped demo that was autorecorded, clean mark so we don't stop the demo later
	if (g_bDemoRecording)
	{
		if (g_bDemoRecordingFrame < 10)
		{
			g_bDemoRecordingFrame++;
		}
		else if (!gEngfuncs.pDemoAPI->IsRecording())
		{
			g_bDemoRecording = false;
		}
	}
}

// Starts demo recording after map initialized.
void ResultsThink(void)
{
	// Do start, but not in single-player and only once
	if (g_iLastMaxClients > 1 && !g_bDemoRecordingStartIssued && !gHUD.m_iIntermission)
	{
		g_bDemoRecordingStartIssued = true;

		// Start demo auto-recording if engine doesn't record a demo already by some direct user request.
		if (m_pCvarResultsDemoAutorecord->value != 0 && !gEngfuncs.pDemoAPI->IsRecording())
		{
			ResultsStartDemoRecord();
		}
	}
}

// Registers cvars and commands.
void ResultsInit(void)
{
	m_pCvarResultsFileFormat = gEngfuncs.pfnRegisterVariable("results_file_format", g_szDefaultFilenameFormat, FCVAR_ARCHIVE);
	m_pCvarResultsCounterFormat = gEngfuncs.pfnRegisterVariable("results_counter_format", g_szDefaultCounterFormat, FCVAR_ARCHIVE);
	m_pCvarResultsDemoAutorecord = gEngfuncs.pfnRegisterVariable("results_demo_autorecord", "0", FCVAR_ARCHIVE);
	m_pCvarResultsDemoKeepDays = gEngfuncs.pfnRegisterVariable("results_demo_keepdays", "14", FCVAR_ARCHIVE);
	m_pCvarResultsLogChat = gEngfuncs.pfnRegisterVariable("results_log_chat", "0", FCVAR_ARCHIVE);
	m_pCvarResultsLogOther = gEngfuncs.pfnRegisterVariable("results_log_other", "0", FCVAR_ARCHIVE);

	gEngfuncs.pfnAddCommand("agrecord", ResultsStartDemoRecord);

	// Get mod directory
	const char *gameDirectory = gEngfuncs.pfnGetGameDirectory();
	strncpy(g_szModDirectory, gameDirectory, MAX_PATH - 1);

	const char *info = gEngfuncs.ServerInfo_ValueForKey("*gamedir");

	if (!g_bNewerBuild)
	{
		// Get language name from the registry, because older engines write demos in languaged directory
		HKEY rKey;
		char value[MAX_PATH];
		DWORD valueSize = sizeof(value);
		if (RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Valve\\Steam", 0, KEY_QUERY_VALUE, &rKey) == ERROR_SUCCESS)
		{
			if (RegQueryValueEx(rKey, "language", NULL, NULL, (unsigned char *)value, &valueSize) == ERROR_SUCCESS &&
				value[0] && strcmp(value, "english") != 0 &&
				strlen(g_szModDirectory) + strlen(value) + 1 < MAX_PATH - 1)
			{
				strcat(g_szModDirectory, "_");
				strcat(g_szModDirectory, value);
			}
			RegCloseKey(rKey);
		}
	}

	// Get full path to mod directory
	GetFullPathName(g_szModDirectory, MAX_PATH, g_szModDirectory, NULL);

	// Get temporary demos file list name and purge old demos
	sprintf_s(g_szTempDemoList, MAX_PATH, "%s\\tempdemolist.txt", g_szModDirectory);
	ResultsPurgeDemos();
}
