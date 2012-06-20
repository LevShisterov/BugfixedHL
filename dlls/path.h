/***
*
*	Copyright (c) 2012, AGHL.RU. All rights reserved.
*
****/
//
// Path.h
//
// File path manipulation routines.
//

#include <windows.h>


void RemoveInvalidFilenameChars(char *path);
void RemoveInvalidPathChars(char *path, bool isRoted);
// Creates directory with all intermediate directories.
bool CreateDirectoryFull(char *path);
