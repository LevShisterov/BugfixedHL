/***
*
*	Copyright (c) 2012, AGHL.RU. All rights reserved.
*
****/
//
// Common_utils.h
//
// Functions that can be used on both: client and server.
//

#ifdef _WIN32

/*============
String functions
strrepl: replaces substrings in a string
============*/
bool strrepl(char *str, int size, const char *find, const char *repl);

#endif
