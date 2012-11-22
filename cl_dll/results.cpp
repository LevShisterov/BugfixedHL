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

#include "wrect.h"
#include "cl_dll.h"
#include "cvardef.h"
#include "com_utils.h"
#include "path.h"


cvar_t *m_pCvarResultsFileFormat, *m_pCvarResultsCounterFormat;
char modDirectory[MAX_PATH];
char defaultFilenameFormat[] = "results\\%Y-%m\\%l-%Y%m%d-%H%M%S";
char defaultCounterFormat[] = "-%03d";
bool g_bFormatError = false;


void InvalidParameterHandler(const wchar_t * expression, const wchar_t * function, const wchar_t * file, unsigned int line, uintptr_t pReserved)
{
	g_bFormatError = true;
}

bool DoSubstitutions(char *filename, char *fullpath, const char *format, const char *map, tm *pTm)
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
	// Use default pattern on format error
	if (g_bFormatError)
	{
		g_bFormatError = false;
		return false;
	}
	_CrtSetReportMode(_CRT_ASSERT, oldMode);
	_set_invalid_parameter_handler(oldHandler);

	// Perform filename validation
	RemoveInvalidPathChars(filename, false);

	// Construct full path
	char *filepart;
	sprintf_s(fullpath, MAX_PATH, "%s\\%s", modDirectory, filename);
	GetFullPathName(fullpath, MAX_PATH, fullpath, &filepart);

	// Check that path lie within mod directory for security reasons
	if (strncmp(fullpath, modDirectory, strlen(modDirectory)))
		return false;
	return true;
}

bool GetResultsFilename(const char *extension, char filename[MAX_PATH], char fullpath[MAX_PATH])
{
	// Get map name
	char map[MAX_PATH];
	strncpy(map, gEngfuncs.pfnGetLevelName(), MAX_PATH);
	map[MAX_PATH - 1] = 0;
	if (!map[0])
		strcpy(map, "none");
	else
	{
		char *s = strrchr(map, '\\');
		if (!s)
			s = strrchr(map, '/');
		else
			s++;
		if (!s)
			s = map;
		else
			s++;
		strrepl(s, MAX_PATH, ".bsp", "");
		strcpy(map, s);
	}
	strrepl(map, MAX_PATH, "%", "");

	// Get time
	time_t t = time(NULL);
	tm *pTm = localtime(&t);

	// Substitute values
	if (!DoSubstitutions(filename, fullpath, m_pCvarResultsFileFormat->string, map, pTm))
	{
		gEngfuncs.Con_Printf("results_file_format contains invalid path, using default format instead.\n");
		if (!DoSubstitutions(filename, fullpath, defaultFilenameFormat, map, pTm))
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

	// Check for existing file and apply counter
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
		return false;
	}
	if (GetFileAttributes(path) != INVALID_FILE_ATTRIBUTES)
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
			sprintf_s(frmt, MAX_PATH, "%%s%s.%%s", defaultCounterFormat);
		else
			sprintf_s(frmt, MAX_PATH, "%%s%s.%%s", m_pCvarResultsCounterFormat->string);
		int i = 0;
		for (i = 1; i < 1000; i++)
		{
			sprintf_s(file, MAX_PATH, frmt, filename, i, extension);
			sprintf_s(path, MAX_PATH, frmt, fullpath, i, extension);
			// Use default pattern on format error
			if (g_bFormatError)
			{
				g_bFormatError = false;
				gEngfuncs.Con_Printf("Couldn't construct filepath for game results file using results_counter_format.\n");
				sprintf_s(frmt, MAX_PATH, "%%s%s.%%s", defaultCounterFormat);
				sprintf_s(file, MAX_PATH, frmt, filename, i, extension);
				sprintf_s(path, MAX_PATH, frmt, fullpath, i, extension);
				if (g_bFormatError)
				{
					g_bFormatError = false;
					gEngfuncs.Con_Printf("Couldn't construct filepath for game results file using default counter format.\n");
					filename[0] = 0;
					fullpath[0] = 0;
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
			return false;
		}
	}
	_CrtSetReportMode(_CRT_ASSERT, oldMode);
	_set_invalid_parameter_handler(oldHandler);

	strcpy(filename, file);
	strcpy(fullpath, path);
}

// Registers cvars
void ResultsInit(void)
{
	m_pCvarResultsFileFormat = gEngfuncs.pfnRegisterVariable("results_file_format", defaultFilenameFormat, FCVAR_ARCHIVE);
	m_pCvarResultsCounterFormat = gEngfuncs.pfnRegisterVariable("results_counter_format", defaultCounterFormat, FCVAR_ARCHIVE);

	// Get mod directory
	const char *gameDirectory = gEngfuncs.pfnGetGameDirectory();
	strncpy(modDirectory, gameDirectory, MAX_PATH - 1);
	// Get language name from the registry
	HKEY rKey;
	char value[MAX_PATH];
	DWORD valueSize = sizeof(value);
	if (RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Valve\\Steam", 0, KEY_QUERY_VALUE, &rKey) == ERROR_SUCCESS)
	{
		if (RegQueryValueEx(rKey, "language", NULL, NULL, (unsigned char *)value, &valueSize) == ERROR_SUCCESS &&
			value[0] && strcmp(value, "english") &&
			strlen(modDirectory) + strlen(value) + 1 < MAX_PATH - 1)
		{
			strcat(modDirectory, "_");
			strcat(modDirectory, value);
		}
		RegCloseKey(rKey);
	}
	// Get full path to mod directory
	char *file;
	GetFullPathName(modDirectory, MAX_PATH - 1, modDirectory, &file);
}
