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
// text_message.cpp
//
// implementation of CHudTextMessage class
//
// this class routes messages through titles.txt for localisation
//

#include <string.h>
#include <stdio.h>

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "vgui_TeamFortressViewport.h"

DECLARE_MESSAGE( m_TextMessage, TextMsg );

int CHudTextMessage::Init(void)
{
	HOOK_MESSAGE( TextMsg );

	gHUD.AddHudElem( this );

	Reset();

	return 1;
};

// Searches through the string for any msg names (indicated by a '#')
// any found are looked up in titles.txt and the new message substituted
// the new value is pushed into dst_buffer
char *CHudTextMessage::LocaliseTextString( const char *msg, char *dst_buffer, int buffer_size )
{
	int size = buffer_size;
	char *dst = dst_buffer;
	for ( char *src = (char*)msg; *src != 0 && size > 1; )
	{
		if ( *src == '#' )
		{
			// cut msg name out of string
			static char word_buf[255];
			char *word_buff_end = word_buf + sizeof(word_buf) - 1;
			char *wdst = word_buf, *word_start = src;
			for ( ++src ; ((*src >= 'A' && *src <= 'z') || (*src >= '0' && *src <= '9')) && wdst < word_buff_end; wdst++, src++ )
			{
				*wdst = *src;
			}
			*wdst = 0;

			// lookup msg name in titles.txt
			client_textmessage_t *clmsg = TextMessageGet( word_buf );
			if ( !clmsg || !(clmsg->pMessage) )
			{
				src = word_start;
				*dst = *src;
				dst++, src++, size--;
				continue;
			}

			// copy string into message instead of msg name
			for ( char *wsrc = (char*)clmsg->pMessage; *wsrc != 0 && size > 1; )
			{
				*dst = *wsrc;
				dst++, wsrc++, size--;
			}
		}
		else
		{
			*dst = *src;
			dst++, src++, size--;
		}
	}
	*dst = 0; // terminate with null
	return dst_buffer;
}

// As above, but with a local static buffer
char *CHudTextMessage::BufferedLocaliseTextString( const char *msg )
{
	static char dst_buffer[1024];
	LocaliseTextString( msg, dst_buffer, 1024 );
	return dst_buffer;
}

// Simplified version of LocaliseTextString;  assumes string is only one word
char *CHudTextMessage::LookupString( const char *msg, int *msg_dest )
{
	if ( !msg )
		return "";

	// '#' character indicates this is a reference to a string in titles.txt, and not the string itself
	if ( msg[0] == '#' ) 
	{
		// this is a message name, so look up the real message
		client_textmessage_t *clmsg = TextMessageGet( msg+1 );

		if ( !clmsg || !(clmsg->pMessage) )
			return (char*)msg; // lookup failed, so return the original string
		
		if ( msg_dest )
		{
			// check to see if titles.txt info overrides msg destination
			// if clmsg->effect is less than 0, then clmsg->effect holds -1 * message_destination
			if ( clmsg->effect < 0 )  // 
				*msg_dest = -clmsg->effect;
		}

		return (char*)clmsg->pMessage;
	}
	else
	{  // nothing special about this message, so just return the same string
		return (char*)msg;
	}
}

void StripEndNewlineFromString( char *str )
{
	int s = strlen( str ) - 1;
	if ( str[s] == '\n' || str[s] == '\r' )
		str[s] = 0;
}

// converts all '\r' characters to '\n', so that the engine can deal with the properly
// returns a pointer to str
char* ConvertCRtoNL( char *str )
{
	for ( char *ch = str; *ch != 0; ch++ )
		if ( *ch == '\r' )
			*ch = '\n';
	return str;
}

// Message handler for text messages
// displays a string, looking them up from the titles.txt file, which can be localised
// parameters:
//   byte:   message direction  ( HUD_PRINTCONSOLE, HUD_PRINTNOTIFY, HUD_PRINTCENTER, HUD_PRINTTALK )
//   string: message
// optional parameters:
//   string: message parameter 1
//   string: message parameter 2
//   string: message parameter 3
//   string: message parameter 4
// any string that starts with the character '#' is a message name, and is used to look up the real message in titles.txt
// the next (optional) one to four strings are parameters for that string (which can also be message names if they begin with '#')
int CHudTextMessage::MsgFunc_TextMsg( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	int msg_dest = READ_BYTE();

	const int MAX_PARAMS = 4;
	const int MAX_TEXT = 127;
	static char szBuf[MAX_PARAMS + 2][MAX_TEXT + 1];

	char *msg_text = LookupString(READ_STRING(), &msg_dest);
	msg_text = strncpy(szBuf[1], msg_text, MAX_TEXT);
	szBuf[1][MAX_TEXT] = 0;

	// keep reading strings and using C format strings for subsituting the strings into the localised text string
	for (int i = 2; i < MAX_PARAMS + 2; i++)
	{
		char *sstr = LookupString(READ_STRING());
		strncpy(szBuf[i], sstr, MAX_TEXT);
		szBuf[i][MAX_TEXT] = 0;
		// these strings are meant for subsitution into the main string, so cull the automatic end newlines
		StripEndNewlineFromString(szBuf[i]);
	}

	char *dst = szBuf[0];

	if (msg_dest == HUD_PRINTNOTIFY)
	{
		dst[0] = 1;  // mark this message to go into the notify buffer
		dst++;
	}

	// Substitute substrings
	//sprintf_s(psz, MAX_TEXT, msg_text, sstr1, sstr2, sstr3, sstr4);
	int next_param = 0;
	char *src = msg_text;
	char *end = szBuf[0] + MAX_TEXT;
	bool inParam = false;
	while (*src && dst < end)
	{
		if (inParam)
		{
			inParam = false;
			if (*src == 's' && next_param < MAX_PARAMS)
			{
				strncpy(dst, szBuf[next_param + 2], MAX_TEXT - (dst - szBuf[0]));
				szBuf[0][MAX_TEXT] = 0;
				dst += strlen(dst);
				src++;
				next_param++;
				continue;
			}
		}
		else if (*src == '%')
		{
			inParam = true;
			src++;
			continue;
		}
		*dst = *src;
		dst++;
		src++;
	}
	*dst = 0;

	ConvertCRtoNL(szBuf[0]);

	switch (msg_dest)
	{
	case HUD_PRINTNOTIFY:
	case HUD_PRINTCONSOLE:
		ConsolePrint(szBuf[0]);
		break;
	case HUD_PRINTTALK:
		gHUD.m_SayText.SayTextPrint(szBuf[0], 128);
		break;
	case HUD_PRINTCENTER:
		if (gViewPort && gViewPort->AllowedToPrintText() == FALSE)
			return 1;
		CenterPrint(szBuf[0]);
		break;
	}

	return 1;
}
