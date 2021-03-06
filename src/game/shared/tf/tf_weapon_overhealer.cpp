﻿//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:			The Medic's Medikit weapon
//					
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "in_buttons.h"
#include "engine/IEngineSound.h"
#include "tf_gamerules.h"

#if defined( CLIENT_DLL )
#include <vgui_controls/Panel.h>
#include <vgui/ISurface.h>
#include "particles_simple.h"
#include "c_tf_player.h"
#include "soundenvelope.h"
#else
#include "ndebugoverlay.h"
#include "tf_player.h"
#include "tf_team.h"
#include "tf_gamestats.h"
#include "ilagcompensationmanager.h"
#endif
#include "tf_weapon_medigun.h"

#include "tf_weapon_overhealer.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Buff ranges
ConVar weapon_overhealer_damage_modifier( "weapon_overhealer_damage_modifier", "1.5", FCVAR_CHEAT | FCVAR_REPLICATED | FCVAR_DEVELOPMENTONLY, "Scales the damage a player does while being healed with the medigun." );

static const char *s_pszMedigunHealTargetThink = "MedigunHealTargetThink";

#ifdef CLIENT_DLL
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void RecvProxy_HealingTargetOverhealer( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	CWeaponOverhealer *pMedigun = ((CWeaponOverhealer*)(pStruct));
	if ( pMedigun != NULL )
	{
		pMedigun->ForceHealingTargetUpdate();
	}

	RecvProxy_IntToEHandle( pData, pStruct, pOut );
}
#endif

LINK_ENTITY_TO_CLASS(tf_weapon_overhealer, CWeaponOverhealer);
PRECACHE_WEAPON_REGISTER(tf_weapon_overhealer);

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponOverhealer, DT_WeaponOverhealer )

BEGIN_NETWORK_TABLE(CWeaponOverhealer, DT_WeaponOverhealer)
#if !defined( CLIENT_DLL )
	SendPropEHandle( SENDINFO( m_hHealingTarget ) ),
	SendPropBool( SENDINFO( m_bHealing ) ),
	SendPropBool( SENDINFO( m_bAttacking ) ),
	SendPropBool( SENDINFO( m_bHolstered ) ),
#else
RecvPropEHandle(RECVINFO(m_hHealingTarget), RecvProxy_HealingTargetOverhealer),
	RecvPropBool( RECVINFO( m_bHealing ) ),
	RecvPropBool( RECVINFO( m_bAttacking ) ),
	RecvPropBool( RECVINFO( m_bHolstered ) ),
#endif
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( CWeaponOverhealer  )

	DEFINE_PRED_FIELD( m_bHealing, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_bAttacking, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_bHolstered, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_hHealingTarget, FIELD_EHANDLE, FTYPEDESC_INSENDTABLE ),

	DEFINE_FIELD( m_flHealEffectLifetime, FIELD_FLOAT ),

END_PREDICTION_DATA()
#endif

#define PARTICLE_PATH_VEL				140.0
#define NUM_PATH_PARTICLES_PER_SEC		300.0f
#define NUM_MEDIGUN_PATH_POINTS		8

extern ConVar tf_max_health_boost;

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CWeaponOverhealer::CWeaponOverhealer(void)
{
	WeaponReset();
	SetPredictionEligible( true );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponOverhealer::WeaponReset(void)
{
	BaseClass::WeaponReset();

	m_flHealEffectLifetime = 0;

	m_bHealing = false;
	m_bAttacking = false;
	m_bHolstered = true;

	m_bCanChangeTarget = true;

	m_flNextBuzzTime = 0;

	RemoveHealingTarget( true );

#if defined( CLIENT_DLL )
	m_bPlayingSound = false;
	m_bUpdateHealingTargets = false;
	//m_bOldChargeRelease = false;

	UpdateEffects();
	ManageChargeEffect();

#endif

}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponOverhealer::Precache()
{
	BaseClass::Precache();
	PrecacheScriptSound( "WeaponMedigun.NoTarget" );
	PrecacheScriptSound( "WeaponMedigun.Healing" );
	PrecacheParticleSystem( "medicgun_beam_red" );
	PrecacheParticleSystem( "medicgun_beam_blue" );
	PrecacheParticleSystem("medicgun_beam_green");
	PrecacheParticleSystem("medicgun_beam_yellow");
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CWeaponOverhealer::Deploy(void)
{
	if ( BaseClass::Deploy() )
	{
		m_bHolstered = false;

#ifdef CLIENT_DLL
		ManageChargeEffect();
#endif

		m_flNextTargetCheckTime = gpGlobals->curtime;

		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CWeaponOverhealer::Holster(CBaseCombatWeapon *pSwitchingTo)
{
	RemoveHealingTarget( true );
	m_bAttacking = false;
	m_bHolstered = true;



#ifdef GAME_DLL
	CTFPlayer *pOwner = ToTFPlayer( GetOwnerEntity() );
	if ( pOwner )
	{
		pOwner->m_Shared.RecalculateInvuln( true );
	}
#endif

#ifdef CLIENT_DLL
	UpdateEffects();
	ManageChargeEffect();
#endif

	return BaseClass::Holster( pSwitchingTo );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponOverhealer::UpdateOnRemove(void)
{
	RemoveHealingTarget( true );
	m_bAttacking = false;

#ifdef CLIENT_DLL
	if ( m_bPlayingSound )
	{
		m_bPlayingSound = false;
		StopHealSound();
	}

	UpdateEffects();
#endif



	BaseClass::UpdateOnRemove();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
float CWeaponOverhealer::GetTargetRange(void)
{
	return (float)m_pWeaponInfo->GetWeaponData( m_iWeaponMode ).m_flRange;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
float CWeaponOverhealer::GetStickRange(void)
{
	return (GetTargetRange() * 1.2);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
float CWeaponOverhealer::GetHealRate(void)
{
	return (float)m_pWeaponInfo->GetWeaponData( m_iWeaponMode ).m_nDamage;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CWeaponOverhealer::HealingTarget(CBaseEntity *pTarget)
{
	if ( pTarget == m_hHealingTarget )
		return true;

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CWeaponOverhealer::AllowedToHealTarget(CBaseEntity *pTarget)
{
	CTFPlayer *pOwner = ToTFPlayer( GetOwnerEntity() );
	if ( !pOwner )
		return false;

	CTFPlayer *pTFPlayer = ToTFPlayer( pTarget );
	if ( !pTFPlayer )
		return false;

	bool bStealthed = pTFPlayer->m_Shared.InCond( TF_COND_STEALTHED );
	bool bDisguised = pTFPlayer->m_Shared.InCond( TF_COND_DISGUISED );

	// We can heal teammates and enemies that are disguised as teammates
	if ( !bStealthed &&
		( pTFPlayer->InSameTeam( pOwner ) ||
		( bDisguised && pTFPlayer->m_Shared.GetDisguiseTeam() == pOwner->GetTeamNumber() ) ) )
	{
		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CWeaponOverhealer::CouldHealTarget(CBaseEntity *pTarget)
{
	CTFPlayer *pOwner = ToTFPlayer( GetOwnerEntity() );
	if ( !pOwner )
		return false;

	if ( pTarget->IsPlayer() && pTarget->IsAlive() && !HealingTarget(pTarget) )
		return AllowedToHealTarget( pTarget );

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponOverhealer::MaintainTargetInSlot()
{
	CTFPlayer *pOwner = ToTFPlayer( GetOwnerEntity() );
	if ( !pOwner )
		return;

	CBaseEntity *pTarget = m_hHealingTarget;
	Assert( pTarget );

	// Make sure the guy didn't go out of range.
	bool bLostTarget = true;
	Vector vecSrc = pOwner->Weapon_ShootPosition( );
	Vector vecTargetPoint = pTarget->WorldSpaceCenter();
	Vector vecPoint;

	// If it's brush built, use absmins/absmaxs
	pTarget->CollisionProp()->CalcNearestPoint( vecSrc, &vecPoint );

	float flDistance = (vecPoint - vecSrc).Length();
	if ( flDistance < GetStickRange() )
	{
		if ( m_flNextTargetCheckTime > gpGlobals->curtime )
			return;

		m_flNextTargetCheckTime = gpGlobals->curtime + 1.0f;

		trace_t tr;
		CMedigunFilter drainFilter( pOwner );

		Vector vecAiming;
		pOwner->EyeVectors( &vecAiming );

		Vector vecEnd = vecSrc + vecAiming * GetTargetRange();
		UTIL_TraceLine( vecSrc, vecEnd, (MASK_SHOT & ~CONTENTS_HITBOX), pOwner, DMG_GENERIC, &tr );

		// Still visible?
		if ( tr.m_pEnt == pTarget )
			return;

		UTIL_TraceLine( vecSrc, vecTargetPoint, MASK_SHOT, &drainFilter, &tr );

		// Still visible?
		if (( tr.fraction == 1.0f) || (tr.m_pEnt == pTarget))
			return;

		// If we failed, try the target's eye point as well
		UTIL_TraceLine( vecSrc, pTarget->EyePosition(), MASK_SHOT, &drainFilter, &tr );
		if (( tr.fraction == 1.0f) || (tr.m_pEnt == pTarget))
			return;
	}

	// We've lost this guy
	if ( bLostTarget )
	{
		RemoveHealingTarget();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponOverhealer::FindNewTargetForSlot()
{
	CTFPlayer *pOwner = ToTFPlayer( GetOwnerEntity() );
	if ( !pOwner )
		return;

	Vector vecSrc = pOwner->Weapon_ShootPosition( );
	if ( m_hHealingTarget )
	{
		RemoveHealingTarget();
	}

	// In Normal mode, we heal players under our crosshair
	Vector vecAiming;
	pOwner->EyeVectors( &vecAiming );

	// Find a player in range of this player, and make sure they're healable.
	Vector vecEnd = vecSrc + vecAiming * GetTargetRange();
	trace_t tr;

	UTIL_TraceLine( vecSrc, vecEnd, (MASK_SHOT & ~CONTENTS_HITBOX), pOwner, DMG_GENERIC, &tr );
	if ( tr.fraction != 1.0 && tr.m_pEnt )
	{
		if ( CouldHealTarget( tr.m_pEnt ) )
		{
#ifdef GAME_DLL
			pOwner->SpeakConceptIfAllowed( MP_CONCEPT_MEDIC_STARTEDHEALING );
			if ( tr.m_pEnt->IsPlayer() )
			{
				CTFPlayer *pTarget = ToTFPlayer( tr.m_pEnt );
				pTarget->SpeakConceptIfAllowed( MP_CONCEPT_HEALTARGET_STARTEDHEALING );
			}

			// Start the heal target thinking.
			SetContextThink(&CWeaponOverhealer::HealTargetThink, gpGlobals->curtime, s_pszMedigunHealTargetThink);
#endif

			m_hHealingTarget.Set( tr.m_pEnt );
			m_flNextTargetCheckTime = gpGlobals->curtime + 1.0f;
		}			
	}
}

#ifdef GAME_DLL
//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeaponOverhealer::HealTargetThink(void)
{	
	// Verify that we still have a valid heal target.
	CBaseEntity *pTarget = m_hHealingTarget;
	if ( !pTarget || !pTarget->IsAlive() )
	{
		SetContextThink( NULL, 0, s_pszMedigunHealTargetThink );
		return;
	}

	CTFPlayer *pOwner = ToTFPlayer( GetOwnerEntity() );
	if ( !pOwner )
		return;

	float flTime = gpGlobals->curtime - pOwner->GetTimeBase();
	if ( flTime > 5.0f || !AllowedToHealTarget(pTarget) )
	{
		RemoveHealingTarget( true );
	}

	SetNextThink( gpGlobals->curtime + 0.2f, s_pszMedigunHealTargetThink );
}
#endif

//-----------------------------------------------------------------------------
// Purpose: Returns a pointer to a healable target
//-----------------------------------------------------------------------------
bool CWeaponOverhealer::FindAndHealTargets(void)
{
	CTFPlayer *pOwner = ToTFPlayer( GetOwnerEntity() );
	if ( !pOwner )
		return false;

	bool bFound = false;

	// Maintaining beam to existing target?
	CBaseEntity *pTarget = m_hHealingTarget;
	if ( pTarget && pTarget->IsAlive() )
	{
		MaintainTargetInSlot();
	}
	else
	{	
		FindNewTargetForSlot();
	}

	CBaseEntity *pNewTarget = m_hHealingTarget;
	if ( pNewTarget && pNewTarget->IsAlive() )
	{
#ifdef GAME_DLL
		CTFPlayer *pTFPlayer = ToTFPlayer( pNewTarget );

		// HACK: For now, just deal with players
		if ( pTFPlayer )
		{
			if ( pTarget != pNewTarget && pNewTarget->IsPlayer() )
			{
				pTFPlayer->m_Shared.Heal( pOwner, GetHealRate() );
			}

			pTFPlayer->m_Shared.RecalculateInvuln( false );
		}
#endif
		bFound = true;
	}

	return bFound;
}

//-----------------------------------------------------------------------------
// Purpose: Overloaded to handle the hold-down healing
//-----------------------------------------------------------------------------
void CWeaponOverhealer::ItemPostFrame(void)
{
	CTFPlayer *pOwner = ToTFPlayer( GetOwnerEntity() );
	if ( !pOwner )
		return;

	// If we're lowered, we're not allowed to fire
	if ( CanAttack() == false )
	{
		RemoveHealingTarget( true );
		return;
	}

#if !defined( CLIENT_DLL )
	if ( AppliesModifier() )
	{
		m_DamageModifier.SetModifier( weapon_overhealer_damage_modifier.GetFloat() );
	}
#endif

	// Try to start healing
	m_bAttacking = false;
	if ( pOwner->GetMedigunAutoHeal() )
	{
		if ( pOwner->m_nButtons & IN_ATTACK )
		{
			if ( m_bCanChangeTarget )
			{
				RemoveHealingTarget();
#if defined( CLIENT_DLL )
				m_bPlayingSound = false;
				StopHealSound();
#endif
				// can't change again until we release the attack button
				m_bCanChangeTarget = false;
			}
		}
		else
		{
			m_bCanChangeTarget = true;
		}

		if ( m_bHealing || ( pOwner->m_nButtons & IN_ATTACK ) )
		{
			PrimaryAttack();
			m_bAttacking = true;
		}
	}
	else
	{
		if ( /*m_bChargeRelease || */ pOwner->m_nButtons & IN_ATTACK )
		{
			PrimaryAttack();
			m_bAttacking = true;
		}
 		else if ( m_bHealing )
 		{
 			// Detach from the player if they release the attack button.
 			RemoveHealingTarget();
 		}
	}

	WeaponIdle();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CWeaponOverhealer::Lower(void)
{
	// Stop healing if we are
	if ( m_bHealing )
	{
		RemoveHealingTarget( true );
		m_bAttacking = false;

#ifdef CLIENT_DLL
		UpdateEffects();
#endif
	}

	return BaseClass::Lower();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponOverhealer::RemoveHealingTarget(bool bStopHealingSelf)
{
	CTFPlayer *pOwner = ToTFPlayer( GetOwnerEntity() );
	if ( !pOwner )
		return;

#ifdef GAME_DLL
	if ( m_hHealingTarget )
	{
		// HACK: For now, just deal with players
		if ( m_hHealingTarget->IsPlayer() )
		{
			CTFPlayer *pOwner = ToTFPlayer( GetOwnerEntity() );
			CTFPlayer *pTFPlayer = ToTFPlayer( m_hHealingTarget );
			pTFPlayer->m_Shared.StopHealing( pOwner );
			pTFPlayer->m_Shared.RecalculateInvuln( false );

			pOwner->SpeakConceptIfAllowed( MP_CONCEPT_MEDIC_STOPPEDHEALING, pTFPlayer->IsAlive() ? "healtarget:alive" : "healtarget:dead" );
			pTFPlayer->SpeakConceptIfAllowed( MP_CONCEPT_HEALTARGET_STOPPEDHEALING );
		}
	}

	// Stop thinking - we no longer have a heal target.
	SetContextThink( NULL, 0, s_pszMedigunHealTargetThink );
#endif

	m_hHealingTarget.Set( NULL );

	// Stop the welding animation
	if ( m_bHealing )
	{
		SendWeaponAnim( ACT_MP_ATTACK_STAND_POSTFIRE );
		pOwner->DoAnimationEvent( PLAYERANIMEVENT_ATTACK_POST );
	}

#ifndef CLIENT_DLL
	m_DamageModifier.RemoveModifier();
#endif
	m_bHealing = false;

}


//-----------------------------------------------------------------------------
// Purpose: Attempt to heal any player within range of the medikit
//-----------------------------------------------------------------------------
void CWeaponOverhealer::PrimaryAttack(void)
{
	CTFPlayer *pOwner = ToTFPlayer( GetOwnerEntity() );
	if ( !pOwner )
		return;

	if ( !CanAttack() )
		return;

#if !defined (CLIENT_DLL)
	lagcompensation->StartLagCompensation( pOwner, pOwner->GetCurrentCommand() );
#endif

	if ( FindAndHealTargets() )
	{
		// Start the animation
		if ( !m_bHealing )
		{
#ifdef GAME_DLL
			pOwner->SpeakWeaponFire();
#endif

			SendWeaponAnim( ACT_MP_ATTACK_STAND_PREFIRE );
			pOwner->DoAnimationEvent( PLAYERANIMEVENT_ATTACK_PRE );
		}

		m_bHealing = true;
	}
	else
	{
		RemoveHealingTarget();
	}
	
#if !defined (CLIENT_DLL)
	lagcompensation->FinishLagCompensation( pOwner );
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Idle tests to see if we're facing a valid target for the medikit
//			If so, move into the "heal-able" animation. 
//			Otherwise, move into the "not-heal-able" animation.
//-----------------------------------------------------------------------------
void CWeaponOverhealer::WeaponIdle(void)
{
	if ( HasWeaponIdleTimeElapsed() )
	{
		// Loop the welding animation
		if ( m_bHealing )
		{
			SendWeaponAnim( ACT_VM_PRIMARYATTACK );
			return;
		}

		return BaseClass::WeaponIdle();
	}
}

#if defined( CLIENT_DLL )
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponOverhealer::StopHealSound(bool bStopHealingSound, bool bStopNoTargetSound)
{
	if ( bStopHealingSound )
	{
		StopSound( "WeaponMedigun.Healing" );
	}

	if ( bStopNoTargetSound )
	{
		StopSound( "WeaponMedigun.NoTarget" );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponOverhealer::ManageChargeEffect(void)
{
	C_TFPlayer *pLocalPlayer = C_TFPlayer::GetLocalTFPlayer();
	C_BaseEntity *pEffectOwner = this;

	if ( pLocalPlayer == NULL )
		return;

	if ( pLocalPlayer == GetTFPlayerOwner() )
	{
		pEffectOwner = pLocalPlayer->GetViewModel();
		if ( !pEffectOwner )
			return;
	}

	bool bOwnerTaunting = false;

	if ( GetTFPlayerOwner() && GetTFPlayerOwner()->m_Shared.InCond( TF_COND_TAUNTING ) == true )
	{
		bOwnerTaunting = true;
	}

}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : updateType - 
//-----------------------------------------------------------------------------
void CWeaponOverhealer::OnDataChanged(DataUpdateType_t updateType)
{
	BaseClass::OnDataChanged( updateType );

	if ( m_bUpdateHealingTargets )
	{
		UpdateEffects();
		m_bUpdateHealingTargets = false;
	}

	// Think?
	if ( m_bHealing )
	{
		ClientThinkList()->SetNextClientThink( GetClientHandle(), CLIENT_THINK_ALWAYS );
	}
	else
	{
		ClientThinkList()->SetNextClientThink( GetClientHandle(), CLIENT_THINK_NEVER );
		m_bPlayingSound = false;
		StopHealSound( true, false );

		// Are they holding the attack button but not healing anyone? Give feedback.
		if ( IsActiveByLocalPlayer() && GetOwner() && GetOwner()->IsAlive() && m_bAttacking && GetOwner() == C_BasePlayer::GetLocalPlayer() && CanAttack() == true )
		{
			if ( gpGlobals->curtime >= m_flNextBuzzTime )
			{
				CLocalPlayerFilter filter;
				EmitSound( filter, entindex(), "WeaponMedigun.NoTarget" );
				m_flNextBuzzTime = gpGlobals->curtime + 0.5f;	// only buzz every so often.
			}
		}
		else
		{
			StopHealSound( false, true );	// Stop the "no target" sound.
		}
	}

	ManageChargeEffect();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponOverhealer::ClientThink()
{
	C_TFPlayer *pLocalPlayer = C_TFPlayer::GetLocalTFPlayer();
	if ( !pLocalPlayer )
		return;

	// Don't show it while the player is dead. Ideally, we'd respond to m_bHealing in OnDataChanged,
	// but it stops sending the weapon when it's holstered, and it gets holstered when the player dies.
	CTFPlayer *pFiringPlayer = ToTFPlayer( GetOwnerEntity() );
	if ( !pFiringPlayer || pFiringPlayer->IsPlayerDead() || pFiringPlayer->IsDormant() )
	{
		ClientThinkList()->SetNextClientThink( GetClientHandle(), CLIENT_THINK_NEVER );
		m_bPlayingSound = false;
		StopHealSound();
		return;
	}
		
	// If the local player is the guy getting healed, let him know 
	// who's healing him, and their charge level.
	if( m_hHealingTarget != NULL )
	{
		if ( pLocalPlayer == m_hHealingTarget )
		{
			pLocalPlayer->SetHealer( pFiringPlayer, 0 );
		}

		if ( !m_bPlayingSound )
		{
			m_bPlayingSound = true;
			CLocalPlayerFilter filter;
			EmitSound( filter, entindex(), "WeaponMedigun.Healing" );
		}
	}

//	if ( m_bOldChargeRelease != m_bChargeRelease )
	{
//		m_bOldChargeRelease = m_bChargeRelease;
//		ForceHealingTargetUpdate();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponOverhealer::UpdateEffects(void)
{
	CTFPlayer *pFiringPlayer = ToTFPlayer( GetOwnerEntity() );
	if ( !pFiringPlayer )
		return;

	C_TFPlayer *pLocalPlayer = C_TFPlayer::GetLocalTFPlayer();
	C_BaseEntity *pEffectOwner = this;
	if ( pLocalPlayer == pFiringPlayer )
	{
		pEffectOwner = pLocalPlayer->GetViewModel();
		if ( !pEffectOwner )
			return;
	}

	// Remove all the effects
	if ( pEffectOwner )
	{
		pEffectOwner->ParticleProp()->StopEmission( m_hHealingTargetEffect.pEffect );
	}
	else
	{
		m_hHealingTargetEffect.pEffect->StopEmission();
	}
	m_hHealingTargetEffect.pTarget = NULL;
	m_hHealingTargetEffect.pEffect = NULL;

	// Don't add targets if the medic is dead
	if ( !pEffectOwner || pFiringPlayer->IsPlayerDead() || !pFiringPlayer->IsPlayerClass( TF_CLASS_MEDIC ) )
		return;

	// Add our targets
	// Loops through the healing targets, and make sure we have an effect for each of them
	if ( m_hHealingTarget )
	{
		if ( m_hHealingTargetEffect.pTarget == m_hHealingTarget )
			return;

		const char *pszEffectName;
		switch (GetTeamNumber())
		{
		case TF_TEAM_BLUE:
			pszEffectName = "medicgun_beam_blue";
			break;
		case TF_TEAM_RED:
			pszEffectName = "medicgun_beam_red";
			break;
		case TF_TEAM_GREEN:
			pszEffectName = "medicgun_beam_green";
			break;
		case TF_TEAM_YELLOW:
			pszEffectName = "medicgun_beam_yellow";
			break;
		default:
			pszEffectName = "medicgun_beam_blue";
			break;
		}

		CNewParticleEffect *pEffect = pEffectOwner->ParticleProp()->Create( pszEffectName, PATTACH_POINT_FOLLOW, "muzzle" );
		pEffectOwner->ParticleProp()->AddControlPoint( pEffect, 1, m_hHealingTarget, PATTACH_ABSORIGIN_FOLLOW, NULL, Vector(0,0,50) );

		m_hHealingTargetEffect.pTarget = m_hHealingTarget;
		m_hHealingTargetEffect.pEffect = pEffect;
	}
}
#endif
