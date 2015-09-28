//====== Copyright © 1996-2013, Valve Corporation, All rights reserved. =======//
//
// Purpose: Grants the user a mini-gun arm known as the Firearm for 15 to 30 seconds, debating on how long this should be.
//	  TODO: Get an actual model, gonna use the Firearm model for now
//
//=============================================================================//
#include "cbase.h"
#include "items.h"
#include "tf_gamerules.h"
#include "tf_shareddefs.h"
#include "tf_player.h"
#include "tf_team.h"
#include "engine/IEngineSound.h"
#include "tf_powerup_firearm.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=============================================================================

BEGIN_DATADESC(CTFPowerupFirearm)
END_DATADESC()

LINK_ENTITY_TO_CLASS(item_powerup_critdamage, CTFPowerupFirearm);

//=============================================================================

//-----------------------------------------------------------------------------
// Purpose: Constructor 
//-----------------------------------------------------------------------------
CTFPowerupFirearm::CTFPowerupFirearm()
{
	m_iRespawnTime = 30;
	m_strModelName = MAKE_STRING("models/items/powerup_crit.mdl");
	m_strPickupSound = MAKE_STRING("HealthKit.Touch");
	//	m_iEffectDuration = 15;
}

//-----------------------------------------------------------------------------
// Purpose: Spawn function 
//-----------------------------------------------------------------------------
void CTFPowerupFirearm::Spawn(void)
{
	BaseClass::Spawn();
}

//-----------------------------------------------------------------------------
// Purpose: Precache 
//-----------------------------------------------------------------------------
void CTFPowerupFirearm::Precache(void)
{
	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: Touch function
//-----------------------------------------------------------------------------
bool CTFPowerupFirearm::MyTouch(CBasePlayer *pPlayer)
{
	return BaseClass::MyTouch(pPlayer);
}
