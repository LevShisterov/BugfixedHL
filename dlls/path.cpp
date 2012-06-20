/***
*
*	Copyright (c) 2012, AGHL.RU. All rights reserved.
*
****/
//
// Path.cpp
//
// File path manipulation routines.
//

void RemoveInvalidFilenameChars(char *path)
{
	char *c = path;
	char *d = path;
	while (*c)
	{
		if (*c <= 31  || *c == '<' || *c == '>' || *c == '"' ||
			*c == '/' || *c == '|' || *c == '?' || *c == '*' ||
			*c == ':' || *c == '\\')
		{
			c++;
			continue;
		}

		*d = *c;
		c++;
		d++;
	}
	*d = 0;
}

void RemoveInvalidPathChars(char *path, bool isRoted)
{
	char *c = path;
	char *d = path;
	if (isRoted)
		while (*c)
		{
			if (*c <= 31  || *c == '<' || *c == '>' || *c == '"' ||
				*c == '/' || *c == '|' || *c == '?' || *c == '*' ||
				(*c == ':' && d - path != 1))
			{
				c++;
				continue;
			}

			*d = *c;
			c++;
			d++;
		}
	else
		while (*c)
		{
			if (*c <= 31  || *c == '<' || *c == '>' || *c == '"' ||
				*c == '/' || *c == '|' || *c == '?' || *c == '*' ||
				*c == ':')
			{
				c++;
				continue;
			}

			*d = *c;
			c++;
			d++;
		}
	*d = 0;
}
