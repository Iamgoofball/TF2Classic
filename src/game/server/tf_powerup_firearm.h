//====== Copyright © 1996-2013, Valve Corporation, All rights reserved. =======//
//
// Purpose: Grants the user 
// Crit glow effect would match the player’s merc color or be blue.
//
//=============================================================================//

#ifndef POWERUP_FIREARM_H
#define POWERUP_FIREARM_H

#ifdef _WIN32
#pragma once
#endif

#include "tf_basedmpowerup.h"

//=============================================================================

class CTFPowerupFirearm : public CTFBaseDMPowerup
{
public:
	DECLARE_CLASS(CTFPowerupFirearm, CTFBaseDMPowerup);
	DECLARE_DATADESC();

	CTFPowerupFirearm();

	void	Spawn(void);
	void	Precache(void);
	bool	MyTouch(CBasePlayer *pPlayer);

	virtual int	GetEffectDuration(void) { return 15; }
	virtual int	GetCondition(void) { return TF_COND_POWERUP_FIREARM; }

	powerupsize_t	GetPowerupSize(void) { return POWERUP_FULL; }
};

#endif // POWERUP_CRITDAMAGE_H


