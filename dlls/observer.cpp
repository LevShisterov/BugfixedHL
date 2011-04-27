//=========== (C) Copyright 1999 Valve, L.L.C. All rights reserved. ===========
//
// The copyright to the contents herein is the property of Valve, L.L.C.
// The contents may be used and/or copied only with the written permission of
// Valve, L.L.C., or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================
// observer.cpp
//
#include	"extdll.h"
#include	"util.h"
#include	"cbase.h"
#include	"player.h"
#include	"weapons.h"

#define NEXT_OBSERVER_INPUT_DELAY	0.2

extern int gmsgCurWeapon;
extern int gmsgSetFOV;
extern int gmsgTeamInfo;

//=========================================================
// Player has become a spectator. Set it up.
//=========================================================
void CBasePlayer::StartObserver(Vector vecPosition, Vector vecViewAngle)
{
	// clear any clientside entities attached to this player
	MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
		WRITE_BYTE(TE_KILLPLAYERATTACHMENTS);
		WRITE_BYTE(ENTINDEX(edict()));	// index number of primary entity
	MESSAGE_END();

	// Holster weapon immediately, to allow it to cleanup
	if (m_pActiveItem)
		m_pActiveItem->Holster();

	// Let's go of tanks
	if (m_pTank != NULL)
		m_pTank->Use(this, this, USE_OFF, 0);

	// clear out the suit message cache so we don't keep chattering
	SetSuitUpdate(NULL, FALSE, 0);

	// Tell Ammo Hud that the player is dead
	MESSAGE_BEGIN(MSG_ONE, gmsgCurWeapon, NULL, pev);
		WRITE_BYTE(0);
		WRITE_BYTE(0XFF);
		WRITE_BYTE(0xFF);
	MESSAGE_END();

	// reset FOV
	pev->fov = m_iFOV = m_iClientFOV = 0;

	MESSAGE_BEGIN(MSG_ONE, gmsgSetFOV, NULL, pev);
		WRITE_BYTE(m_iFOV);
	MESSAGE_END();

	// Setup flags
	m_iHideHUD = HIDEHUD_WEAPONS | HIDEHUD_HEALTH;
	m_afPhysicsFlags |= PFLAG_OBSERVER;
	pev->effects = EF_NODRAW;
	pev->view_ofs = g_vecZero;
	pev->angles = pev->v_angle = vecViewAngle;
	pev->fixangle = TRUE;
	pev->solid = SOLID_NOT;
	pev->takedamage = DAMAGE_NO;
	pev->movetype = MOVETYPE_NONE;
	ClearBits(m_afPhysicsFlags, PFLAG_DUCKING);
	ClearBits(pev->flags, FL_DUCKING);
	pev->deadflag = DEAD_RESPAWNABLE;
	pev->health = 1;

	// Clear out the status bar
	m_fInitHUD = TRUE;

	// Update Team Status
	pev->team = 0;

	MESSAGE_BEGIN(MSG_ALL, gmsgTeamInfo);
		WRITE_BYTE(ENTINDEX(edict()));	// index number of primary entity
		WRITE_STRING("");
	MESSAGE_END();

	// Remove all the player's stuff
	RemoveAllItems(FALSE);

	// Move them to the new position
	UTIL_SetOrigin(pev, vecPosition);

	// Delay between observer inputs
	m_flNextObserverInput = 0;

	// Find a player to watch
	Observer_SetMode(m_iObserverMode);
}

//=========================================================
// Leave observer mode
//=========================================================
void CBasePlayer::StopObserver(void)
{
	// Turn off spectator
	if (pev->iuser1 || pev->iuser2)
	{
		pev->iuser1 = pev->iuser2 = 0; 
		m_hObserverTarget = NULL;
		m_iHideHUD = 0;

		//pev->team = 
		MESSAGE_BEGIN(MSG_ALL, gmsgTeamInfo);
			WRITE_BYTE(ENTINDEX(edict()));
			WRITE_STRING(m_szTeamName);
		MESSAGE_END();
	}
}

//=========================================================
// Attempt to change the observer mode
//=========================================================
void CBasePlayer::Observer_SetMode(int iMode)
{
	// Just abort if we're changing to the mode we're already in
	if (iMode == pev->iuser1)
		return;

	// is valid mode ?
	if (iMode < OBS_CHASE_LOCKED || iMode > OBS_MAP_CHASE)
		iMode = OBS_IN_EYE; // now it is

	if (m_hObserverTarget && 
		(m_hObserverTarget == this || m_hObserverTarget->pev->iuser1 || (m_hObserverTarget->pev->effects & EF_NODRAW)))
	{
		m_hObserverTarget = NULL;
	}

	// set spectator mode
	pev->iuser1 = iMode;

	// if we are not roaming, we need a valid target to track
	if (iMode != OBS_ROAMING && m_hObserverTarget == NULL)
	{
		Observer_FindNextPlayer(false);

		// if we didn't find a valid target switch to roaming
		if (m_hObserverTarget == NULL)
		{
			ClientPrint(pev, HUD_PRINTCENTER, "#Spec_NoTarget");
			pev->iuser1 = OBS_ROAMING;
		}
	}

	// set target if not roaming
	if (pev->iuser1 == OBS_ROAMING)
		pev->iuser2 = 0;
	else
		pev->iuser2 = ENTINDEX(m_hObserverTarget->edict());

	pev->iuser3 = 0;

	// print spectator mode on client screen
	char modemsg[16];
	sprintf(modemsg,"#Spec_Mode%i", pev->iuser1);
	ClientPrint(pev, HUD_PRINTCENTER, modemsg);

	m_iObserverMode = pev->iuser1;
}

// Find the next client in the game for this player to spectate
void CBasePlayer::Observer_FindNextPlayer(bool bReverse)
{
	// MOD AUTHORS: Modify the logic of this function if you want to restrict the observer to watching
	//				only a subset of the players. e.g. Make it check the target's team.

	int iStart;
	if (m_hObserverTarget)
		iStart = ENTINDEX(m_hObserverTarget->edict());
	else
		iStart = ENTINDEX(edict());
	m_hObserverTarget = NULL;

	int iCurrent = iStart;
	int iDir = bReverse ? -1 : 1; 
	do
	{
		iCurrent += iDir;

		// Loop through the clients
		if (iCurrent > gpGlobals->maxClients)
			iCurrent = 1;
		if (iCurrent < 1)
			iCurrent = gpGlobals->maxClients;

		CBaseEntity *pEnt = UTIL_PlayerByIndex(iCurrent);
		if (!pEnt)
			continue;
		if (pEnt == this)
			continue;
		// Don't spec observers or invisible players
		if (((CBasePlayer*)pEnt)->IsObserver() || (pEnt->pev->effects & EF_NODRAW))
			continue;

		m_hObserverTarget = pEnt;
		break;

	} while (iCurrent != iStart);

	// Did we find a target?
	if (m_hObserverTarget)
	{
		// Move to the target
		UTIL_SetOrigin(pev, m_hObserverTarget->pev->origin);

		// Store the target in pev so the physics DLL can get to it
		if (pev->iuser1 != OBS_ROAMING)
			pev->iuser2 = ENTINDEX(m_hObserverTarget->edict());

		//ALERT(at_console, "Now Tracking %s\n", STRING(m_hObserverTarget->pev->netname));
	}
	else
	{
		//ALERT(at_console, "No observer targets.\n");
	}
}

// Handle buttons in observer mode
void CBasePlayer::Observer_HandleButtons()
{
	// Slow down mouse and keyboard clicks
	if (m_flNextObserverInput > gpGlobals->time)
		return;

	// Jump changes view modes
	if (m_afButtonPressed & IN_JUMP)
	{
		int iMode;
		switch(pev->iuser1)
		{
		case OBS_CHASE_LOCKED:
			iMode = OBS_CHASE_FREE;
			break;
		case OBS_CHASE_FREE:
			iMode = OBS_MAP_CHASE;
			break;
		case OBS_ROAMING:
			iMode = OBS_IN_EYE;
			break;
		case OBS_IN_EYE:
			iMode = OBS_CHASE_LOCKED;
			break;
		case OBS_MAP_FREE:
			iMode = OBS_ROAMING;
			break;
		case OBS_MAP_CHASE:
			iMode = OBS_MAP_FREE;
			break;
		default:
			iMode = OBS_ROAMING;
		}
		Observer_SetMode(iMode);

		m_flNextObserverInput = gpGlobals->time + NEXT_OBSERVER_INPUT_DELAY;
	}

	// Attack moves to the next player
	if (m_afButtonPressed & IN_ATTACK && pev->iuser1 != OBS_ROAMING)
	{
		Observer_FindNextPlayer(false);

		m_flNextObserverInput = gpGlobals->time + NEXT_OBSERVER_INPUT_DELAY;
	}

	// Attack2 moves to the prev player
	if (m_afButtonPressed & IN_ATTACK2 && pev->iuser1 != OBS_ROAMING)
	{
		Observer_FindNextPlayer(true);

		m_flNextObserverInput = gpGlobals->time + NEXT_OBSERVER_INPUT_DELAY;
	}
}

void CBasePlayer::Observer_CheckTarget()
{
	// if we are not roaming, we need a valid target to track
	if (pev->iuser1 != OBS_ROAMING)
	{
		if (m_hObserverTarget == NULL)
		{
			Observer_FindNextPlayer(false);
		}

		// if we didn't find a valid target switch to roaming
		if (m_hObserverTarget == NULL)
		{
			int iMode = pev->iuser1;
			Observer_SetMode(OBS_ROAMING);
			m_iObserverMode = iMode;
		}
		else
		{
			CBasePlayer *pPlayer = (CBasePlayer*)UTIL_PlayerByIndex(ENTINDEX(m_hObserverTarget->edict()));
			if (!pPlayer || (pPlayer->pev->deadflag == DEAD_DEAD && pPlayer->m_fDeadTime + 2.0 < gpGlobals->time))
				Observer_FindNextPlayer(false);
		}
	}
}

void CBasePlayer::Observer_CheckProperties()
{
	if (pev->iuser1 == OBS_IN_EYE && m_hObserverTarget)
	{
		CBasePlayer *pPlayer = (CBasePlayer*)UTIL_PlayerByIndex(ENTINDEX(m_hObserverTarget->edict()));
		if (pPlayer)
		{
			int iItem;
			if (pPlayer->m_pActiveItem)
				iItem = pPlayer->m_pActiveItem->m_iId;
			else
				iItem = 0;
			if (m_iFOV != pPlayer->m_iFOV || m_iObservedWeaponId != iItem)
			{
				m_iFOV = m_iClientFOV = pPlayer->m_iFOV;
				MESSAGE_BEGIN(MSG_ONE, gmsgSetFOV, NULL, pev);
					WRITE_BYTE(m_iFOV);
				MESSAGE_END();

				m_iObservedWeaponId = iItem;
				MESSAGE_BEGIN(MSG_ONE, gmsgCurWeapon, NULL, pev);
					WRITE_BYTE(1);
					WRITE_BYTE(m_iObservedWeaponId);
					WRITE_BYTE(0);
				MESSAGE_END();
			}
		}
	}
	else
	{
		m_iFOV = 90;
		if (m_iObservedWeaponId)
		{
			m_iObservedWeaponId = 0;

			MESSAGE_BEGIN(MSG_ONE, gmsgCurWeapon, NULL, pev);
				WRITE_BYTE(1);
				WRITE_BYTE(m_iObservedWeaponId);
				WRITE_BYTE(0);
			MESSAGE_END();
		}
	}
}
