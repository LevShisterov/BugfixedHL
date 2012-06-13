/***
*
*	Copyright (c) 2012, AGHL.RU. All rights reserved.
*
****/
//
// Hud_timer.cpp
//
// implementation of CHudTimer class
//

#include <string.h>
#include <stdio.h>
#include <windows.h>
#include <time.h>

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "net_api.h"
#include "net.h"

#define NET_API gEngfuncs.pNetAPI

#define TIMER_Y 0.04
#define TIMERS_Y_OFFSET 0.04
#define TIMER_RED_R 255
#define TIMER_RED_G 16
#define TIMER_RED_B 16
#define CUSTOM_TIMER_R 0
#define CUSTOM_TIMER_G 160
#define CUSTOM_TIMER_B 0


int CHudTimer::Init(void)
{
	gHUD.AddHudElem(this);

	m_iFlags |= HUD_ACTIVE;

	m_HUD_timer = gEngfuncs.pfnRegisterVariable("hud_timer", "1", FCVAR_ARCHIVE);

	return 1;
};

int CHudTimer::VidInit(void)
{
	m_iNextSyncTime = 0;
	m_iEndtime = 0;
	memset(m_iCustomTimes, 0, sizeof(m_iCustomTimes));
	memset(m_bCustomTimerNeedSound, 0, sizeof(m_bCustomTimerNeedSound));
	m_bAgVersion = SV_AG_UNKNOWN;

	return 1;
};

int CHudTimer::SyncTimer(float fTime)
{
	// Make sure networking system has started.
	NET_API->InitNetworking();
	// Get net status
	net_status_t status;
	NET_API->Status(&status);
	if (status.connected)
	{
		if (status.remote_address.type != NA_LOOPBACK)
		{
			// Retrieve timer settings from the server
			char buffer[2048];
			int len = NetSendReceiveUdp(*((unsigned int*)status.remote_address.ip), status.remote_address.port, "\xFF\xFF\xFF\xFFV\xFF\xFF\xFF\xFF", 9, buffer, sizeof(buffer));
			if (len > 0)
			{
				char *value = NetGetRuleValueFromBuffer(buffer, len, "mp_timelimit");
				if (value != NULL && value[0])
				{
					m_iEndtime = atof(value) * 60;
				}
				else
				{
					m_iEndtime = 0;
				}
				value = NetGetRuleValueFromBuffer(buffer, len, "mp_timeleft");
				if (value != NULL && value[0])
				{
					float timeleft = atof(value);
					if (timeleft > 0)
					{
						float endtime = timeleft + (int)(fTime - status.latency + 0.5);
						if (abs(m_iEndtime - endtime) > 1.5)
							m_iEndtime = endtime;
					}
				}
				if (m_bAgVersion == SV_AG_UNKNOWN)
				{
					value = NetGetRuleValueFromBuffer(buffer, len, "sv_ag_version");
					if (value != NULL && value[0])
					{
						if (!strcmp(value, "6.6") || !strcmp(value, "6.3"))
						{
							m_bAgVersion = SV_AG_FULL;
						}
						else // We will assume its miniAG server, which will be true in almost all cases
						{
							m_bAgVersion = SV_AG_MINI;
						}
					}
					else
					{
						m_bAgVersion = SV_AG_NONE;
					}
				}
			}

			m_iNextSyncTime = fTime + 30;
		}
		else
		{
			// Get timer settings directly from cvars
			m_iEndtime = CVAR_GET_FLOAT("mp_timelimit");
			float timeleft = CVAR_GET_FLOAT("mp_timeleft");
			if (timeleft > 0)
			{
				float endtime = timeleft + fTime;
				if (abs(m_iEndtime - endtime) > 1.5)
					m_iEndtime = endtime;
			}

			m_iNextSyncTime = fTime + 5;
		}
	}

	return 1;
};

void CHudTimer::CustomTimerCommand(void)
{
	if (gEngfuncs.Cmd_Argc() <= 1)
	{
		gEngfuncs.Con_Printf( "usage:  customtimer <interval in seconds> [<timer number 1|2>]\n" );
		return;
	}

	// Get interval value
	int interval;
	char *intervalString = gEngfuncs.Cmd_Argv(1);
	if (!intervalString || !intervalString[0]) return;
	interval = atoi(intervalString);
	if (interval < 0) return;
	if (interval > 86400) interval = 86400;

	// Get timer number
	int number = 0;
	char *numberString = gEngfuncs.Cmd_Argv(2);
	if (numberString && numberString[0])
	{
		number = atoi(numberString) - 1;
		if (number < 0 || number >= MAX_CUSTOM_TIMERS) return;
	}

	// Set custom timer
	m_iCustomTimes[number] = gHUD.m_flTime + interval;
	m_bCustomTimerNeedSound[number] = true;
}

int CHudTimer::Draw(float fTime)
{
	char text[64];
	float timeleft;
	int ypos = ScreenHeight * TIMER_Y;

	// Do sync. We do it always, so message hud can hide miniAG timer, and timer could work just as it is enabled
	if (m_iNextSyncTime <= fTime) SyncTimer(fTime);

	if (gHUD.m_iHideHUDDisplay & HIDEHUD_ALL)
		return 1;

	// Get the paint color
	int r, g, b;
	float a = 255 * gHUD.GetHudTransparency();
	gHUD.GetHudColor(0, 0, r, g, b);
	ScaleColors(r, g, b, a);

	// Draw timer
	int hud_timer = (int)m_HUD_timer->value;
	switch(hud_timer)
	{
	case 1:	// time left
		timeleft = (int)(m_iEndtime - fTime) + 1;
		if (timeleft > 0)
			DrawTimerInternal(timeleft, ypos, r, g, b, true);
		break;
	case 2:	// time passed
		DrawTimerInternal((int)fTime, ypos, r, g, b, false);
		break;
	case 3:	// local PC time
		time_t rawtime;
		struct tm *timeinfo;
		time(&rawtime);
		timeinfo = localtime(&rawtime);
		sprintf(text, "Clock %ld:%02ld:%02ld", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
		// Output to screen
		int width = TextMessageDrawString(ScreenWidth + 1, ypos, text, 0, 0, 0);
		TextMessageDrawString((ScreenWidth - width) / 2, ypos, text, r, g, b);
		break;
	}

	// Draw custom timers
	for (int i = 0; i < MAX_CUSTOM_TIMERS; i++)
	{
		if (m_iCustomTimes[i] > fTime)
		{
			timeleft = (int)(m_iCustomTimes[i] - fTime) + 1;
			sprintf(text, "Timer %ld", (int)timeleft);
			// Output to screen
			ypos = ScreenHeight * (TIMER_Y + TIMERS_Y_OFFSET * (i + 1));
			int width = TextMessageDrawString(ScreenWidth + 1, ypos, text, 0, 0, 0);
			float a = 255 * gHUD.GetHudTransparency();
			r = CUSTOM_TIMER_R, g = CUSTOM_TIMER_G, b = CUSTOM_TIMER_B;
			ScaleColors(r, g, b, a);
			TextMessageDrawString((ScreenWidth - width) / 2, ypos, text, r, g, b);
		}
		else if (m_bCustomTimerNeedSound[i])
		{
			m_bCustomTimerNeedSound[i] = false;
			PlaySound("fvox/bell.wav", 1);
		}
	}

	return 1;
}

void CHudTimer::DrawTimerInternal(float time, float ypos, int r, int g, int b, bool redOnLow)
{
	div_t q;
	char text[64];

	// Calculate time parts and format into a text
	if (time >= 86400)
	{
		q = div(time, 86400);
		int d = q.quot;
		q = div(q.rem, 3600);
		int h = q.quot;
		q = div(q.rem, 60);
		int m = q.quot;
		int s = q.rem;
		sprintf(text, "%ldd %ldh %02ldm %02lds", d, h, m, s);
	}
	else if (time >= 3600)
	{
		q = div(time, 3600);
		int h = q.quot;
		q = div(q.rem, 60);
		int m = q.quot;
		int s = q.rem;
		sprintf(text, "%ldh %02ldm %02lds", h, m, s);
	}
	else if (time >= 60)
	{
		q = div(time, 60);
		int m = q.quot;
		int s = q.rem;
		sprintf(text, "%ld:%02ld", m, s);
	}
	else
	{
		sprintf(text, "%ld", (int)time);
		if (redOnLow)
		{
			float a = 255 * gHUD.GetHudTransparency();
			r = TIMER_RED_R, g = TIMER_RED_G, b = TIMER_RED_B;
			ScaleColors(r, g, b, a);
		}
	}

	// Output to screen
	int width = TextMessageDrawString(ScreenWidth + 1, ypos, text, 0, 0, 0);
	TextMessageDrawString((ScreenWidth - width) / 2, ypos, text, r, g, b);
}
