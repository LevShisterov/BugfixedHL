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

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "net_api.h"
#include "net.h"

#include <string.h>
#include <stdio.h>
#include <windows.h>
#include <time.h>

#define NET_API gEngfuncs.pNetAPI

#define TIMER_Y 0.02
#define TIMER_R 255
#define TIMER_G 160
#define TIMER_B 0


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
				value = NetGetRuleValueFromBuffer(buffer, len, "mp_timeleft");
				if (value != NULL && value[0])
				{
					float endtime = atof(value) + (int)(fTime - status.latency + 0.5);
					if (abs(m_iEndtime - endtime) > 1.5)
						m_iEndtime = endtime;
				}
			}

			m_iNextSyncTime = fTime + 30;
		}
		else
		{
			// Get timer settings directly from cvars
			m_iEndtime = CVAR_GET_FLOAT("mp_timelimit");
			float endtime = CVAR_GET_FLOAT("mp_timeleft") + fTime;
			if (abs(m_iEndtime - endtime) > 1.5)
				m_iEndtime = endtime;

			m_iNextSyncTime = fTime + 5;
		}
	}

	return 1;
};

int CHudTimer::Draw(float fTime)
{
	if (m_HUD_timer->value <= 0 || m_HUD_timer->value > 3) return 1;

	char text[64];
	int r = TIMER_R, g = TIMER_G, b = TIMER_B;
	float drawTime;

	if (m_HUD_timer->value == 1)	// time left
	{
		if (m_iNextSyncTime <= fTime)
			SyncTimer(fTime);

		float timeleft = (int)(m_iEndtime - fTime) + 1;
		drawTime = timeleft;
	}
	else if (m_HUD_timer->value == 2)	// time passed
	{
		drawTime = (int)fTime;
	}
	else if (m_HUD_timer->value == 3)	// local PC time
	{
		time_t rawtime;
		struct tm *timeinfo;
		time(&rawtime);
		timeinfo = localtime(&rawtime);
		sprintf(text, "Clock %ld:%02ld:%02ld", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
	}

	if (m_HUD_timer->value == 1 || m_HUD_timer->value == 2)
	{
		// Calculate time parts
		if (drawTime <= 0) return 1;
		div_t q;
		if (drawTime > 86400)
		{
			q = div(drawTime, 86400);
			int d = q.quot;
			q = div(q.rem, 3600);
			int h = q.quot;
			q = div(q.rem, 60);
			int m = q.quot;
			int s = q.rem;
			sprintf(text, "%ldd %ldh %02ldm %02lds", d, h, m, s);
		}
		else if (drawTime > 3600)
		{
			q = div(drawTime, 3600);
			int h = q.quot;
			q = div(q.rem, 60);
			int m = q.quot;
			int s = q.rem;
			sprintf(text, "%ldh %02ldm %02lds", h, m, s);
		}
		else if (drawTime > 60)
		{
			q = div(drawTime, 60);
			int m = q.quot;
			int s = q.rem;
			sprintf(text, "%ld:%02ld", m, s);
		}
		else
		{
			sprintf(text, "%ld", (int)drawTime);
			r = 255, g = 16, b = 16;
		}
	}

	// Output to screen
	int ypos = ScreenHeight * TIMER_Y;
	int width = TextMessageDrawString(ScreenWidth + 1, ypos, text, r, g, b);
	TextMessageDrawString((ScreenWidth - width) / 2, ypos, text, r, g, b);

	return 1;
}
