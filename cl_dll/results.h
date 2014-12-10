/***
*
*	Copyright (c) 2012, AGHL.RU. All rights reserved.
*
****/
//
// Results.h
//
// Functions for storing game results files.
//

#include <windows.h>


bool GetResultsFilename(const char *extension, char fileName[MAX_PATH], char fullPath[MAX_PATH]);
void ResultsAddLog(const char *line, bool chat);
void ResultsStop(void);
void ResultsFrame(double time);
void ResultsThink(void);
void ResultsInit(void);
