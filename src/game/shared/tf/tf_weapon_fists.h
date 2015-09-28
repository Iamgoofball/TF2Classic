//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================

#ifndef TF_WEAPON_FISTS_H
#define TF_WEAPON_FISTS_H
#ifdef _WIN32
#pragma once
#endif

#include "tf_weaponbase_melee.h"

#ifdef CLIENT_DLL
#define CTFFists C_TFFists
#define CTFHydraulic_Hammers C_TFHydraulic_Hammers
#endif

//=============================================================================
//
// Fists weapon class.
//
class CTFFists : public CTFWeaponBaseMelee
{
public:

	DECLARE_CLASS( CTFFists, CTFWeaponBaseMelee );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	CTFFists() {}
	virtual int			GetWeaponID( void ) const			{ return TF_WEAPON_FISTS; }

	virtual void PrimaryAttack();
	virtual void SecondaryAttack();

	virtual void SendPlayerAnimEvent( CTFPlayer *pPlayer );

	virtual void DoViewModelAnimation( void );

	void Punch( void );

private:

	CTFFists( const CTFFists & ) {}
};

class CTFHydraulic_Hammers : public CTFFists
{
public:
	DECLARE_CLASS(CTFHydraulic_Hammers, CTFFists);
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	virtual bool	Holster(CBaseCombatWeapon *pSwitchingTo);
	virtual int		GetWeaponID(void) const			{ return TF_WEAPON_HYDRAULIC_HAMMERS; }
};

#endif // TF_WEAPON_FISTS_H
