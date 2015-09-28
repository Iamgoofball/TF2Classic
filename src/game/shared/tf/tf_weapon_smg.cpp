//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======
//
//
//=============================================================================
#include "cbase.h"
#include "tf_weapon_smg.h"
// Client specific.
#ifdef CLIENT_DLL
#include "c_tf_player.h"
// Server specific.
#else
#include "tf_player.h"
#endif

//=============================================================================
//
// Weapon SMG tables.
//
#define CREATE_SIMPLE_WEAPON_TABLE( WpnName, entityname )			\
																	\
	IMPLEMENT_NETWORKCLASS_ALIASED( WpnName, DT_##WpnName )	\
															\
	BEGIN_NETWORK_TABLE( C##WpnName, DT_##WpnName )			\
	END_NETWORK_TABLE()										\
															\
	BEGIN_PREDICTION_DATA( C##WpnName )						\
	END_PREDICTION_DATA()									\
															\
	LINK_ENTITY_TO_CLASS( entityname, C##WpnName );			\
	PRECACHE_WEAPON_REGISTER( entityname );


CREATE_SIMPLE_WEAPON_TABLE(TFSMG, tf_weapon_smg)
CREATE_SIMPLE_WEAPON_TABLE(TFSMG_Scout, tf_weapon_smg_scout)
CREATE_SIMPLE_WEAPON_TABLE(TFFirearm, tf_weapon_firearm)

// Server specific.
//#ifndef CLIENT_DLL
//BEGIN_DATADESC( CTFSMG )
//END_DATADESC()
//#endif

//=============================================================================
//
// Weapon SMG functions.
//

bool CTFFirearm::Holster(CBaseCombatWeapon *pSwitchingTo)
{
	CTFPlayer *pPlayer = GetTFPlayerOwner();
	if (!pPlayer)
		return false;
	if (pPlayer->m_Shared.InCond(TF_COND_POWERUP_FIREARM))
	{
		return false;
	}
	return BaseClass::Holster(pSwitchingTo);
}