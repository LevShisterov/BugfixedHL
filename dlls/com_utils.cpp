/***
*
*	Copyright (c) 2012, AGHL.RU. All rights reserved.
*
****/
//
// Common_utils.cpp
//
// Functions that can be used on both: client and server.
//

#ifdef _WIN32
#include <windows.h>
#endif

#include "com_utils.h"


#ifdef _WIN32

/*============
String functions
strrepl: replaces substrings in a string
============*/
bool strrepl(char *str, int size, const char *find, const char *repl)
{
	if (size < 1) return true;
	if (size < 2)
	{
		str[0] = 0;
		return true;
	}

	char *buffer = (char *)malloc(size);
	if (buffer == NULL) return false;

	char *c = str;
	char *buf = buffer;
	char *buf_end = buffer + size - 1;
	const char *match = find;
	int len_find = strlen(find);
	while(*c && buf < buf_end)
	{
		if (*match == *c)
		{
			match++;
			if (!*match)
			{
				// Match found: rewind buffer and copy relpacement there
				buf -= len_find - 1;
				int len = buf_end - buf;
				if (len < 1) break;
				strncpy(buf, repl, len);
				match = find;
				buf += min(strlen(repl), len);
				c++;
				continue;
			}
		}
		else
		{
			// Reset match
			match = find;
		}

		*buf = *c;
		buf++;
		c++;
	}
	*buf = 0;
	strcpy(str, buffer);
	free(buffer);
	return true;
}

#endif
