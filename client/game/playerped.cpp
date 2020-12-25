//----------------------------------------------------------
//
// SA:MP Multiplayer Modification For GTA:SA
// Copyright 2004-2005 SA:MP team
//
// Version: $Id: playerped.cpp,v 1.56 2006/05/20 08:28:04 kyeman Exp $
//
//----------------------------------------------------------

#include <windows.h>
#include <assert.h>
#define _ASSERT assert

#include "../main.h"
#include "game.h"
#include "util.h"
#include "keystuff.h"
#include "task.h"

//extern BOOL	bIgnoreNextEntry;
//extern BOOL	bIgnoreNextExit;

extern BYTE	*pbyteCurrentPlayer;

/*char szSpecialActors[][32] =
{"TENPEN", "PULASKI", "HERN", "SMOKE", "SWEET",
"RYDER2", "RYDER3", "EMMET", "ANDRE", "OGLOC", "CESAR", "KENDL",
"WUZIMU", "CAT", "TRUTH", "JETHRO", "DWAYNE", "ZERO", "TBONE",
"JIZZY", "TORINO", "SINDACO", "MACCER", "PAUL", "ROSE", "FORELLI",
"MADDOGG", "CROGRL2", "JANITOR", "BBTHIN", "SMOKEV", "GANGRL2",
"MECGRL2", "GUNGRL2", "COPGRL2", "NURGRL2"};*/


//-----------------------------------------------------------
// Used for instancing the local player.

CPlayerPed::CPlayerPed()
{
	m_dwGTAId = 1; // 0x001
	m_pPed = GamePool_FindPlayerPed();
	m_pEntity = (ENTITY_TYPE *)GamePool_FindPlayerPed();
	
	m_bytePlayerNumber = 0;
	SetPlayerPedPtrRecord(m_bytePlayerNumber,(DWORD)m_pPed);
	ScriptCommand(&set_actor_weapon_droppable,m_dwGTAId,1);

	ResetForRespawn();

	m_dwArrow = 0;
	m_iParachuteAnim = 0;
	m_iParachuteState = 0;
	m_dwParachuteObject = 0;
	m_iDanceState = 0;
	m_iCellPhoneEnabled = 0;

	m_iPissingState = 0;
	m_dwPissParticlesHandle = 0;
	
	m_iBeerState = 0;
}

//-----------------------------------------------------------
// This is the constructor for creating new player.

CPlayerPed::CPlayerPed(int iPlayerNumber, int iSkin, float fX, float fY,float fZ,float fRotation,BYTE byteCreateMarker)
{
	DWORD dwPlayerActorID=0;
	
	m_pPed=0;
	m_dwGTAId=0;

	// CREATE PLAYER
	ScriptCommand(&create_player, &iPlayerNumber, fX, fY, fZ, &dwPlayerActorID);
	ScriptCommand(&create_actor_from_player,&iPlayerNumber,&dwPlayerActorID);
	
	m_dwGTAId = dwPlayerActorID;
	m_pPed = GamePool_Ped_GetAt(m_dwGTAId);
	m_pEntity = (ENTITY_TYPE *)m_pPed;

	m_bytePlayerNumber = (BYTE)iPlayerNumber;
	SetPlayerPedPtrRecord(m_bytePlayerNumber,(DWORD)m_pPed);
	ScriptCommand(&set_actor_weapon_droppable,m_dwGTAId,1);
	ScriptCommand(&set_actor_immunities,m_dwGTAId,0,0,1,0,0);

	//ScriptCommand(&set_actor_immunities,m_dwGTAId,0,0,1,1,0); // (0.1b)
	
	SetMoney(pNetGame->m_iDeathDropMoney);

	ResetForRespawn();

	m_dwArrow = 0;
	m_byteCreateMarker = byteCreateMarker;
	if(m_byteCreateMarker) ShowMarker();

	m_iParachuteAnim = 0;
	m_iParachuteState = 0;
	m_dwParachuteObject = 0;
	m_iCellPhoneEnabled = 0;

	m_iDanceState = 0;

	m_iPissingState = 0;
	m_dwPissParticlesHandle = 0;

	m_iBeerState = 0;

	SetModelIndex(iSkin);
	SetTargetRotation(fRotation);
}

//-----------------------------------------------------------

CPlayerPed::~CPlayerPed(){}

//-----------------------------------------------------------

void CPlayerPed::ResetForRespawn()
{
	ScriptCommand(&set_actor_can_be_decapitated,m_dwGTAId,0);
}

//-----------------------------------------------------------

void CPlayerPed::Destroy()
{
	DWORD dwPedPtr = (DWORD)m_pPed;

	// If it points to the CPlaceable vtable it's not valid
	if(!m_pPed || !GamePool_Ped_GetAt(m_dwGTAId) || m_pPed->entity.vtable == 0x863C40)
	{
		m_pPed = NULL;
		m_pEntity = NULL;
		m_dwGTAId = 0;
		return;
	}

	if(m_dwParachuteObject) {
		ScriptCommand(&disassociate_object,m_dwParachuteObject,0.0f,0.0f,0.0f,0);
		ScriptCommand(&destroy_object_with_fade,m_dwParachuteObject);
		m_dwParachuteObject = 0;
	}

	if(IN_VEHICLE(m_pPed)) {
		RemoveFromVehicleAndPutAt(100.0f,100.0f,10.0f);
	}

	// FIX FOR PLAYER DESTRUCTION PROBLEM
	_asm mov esi, dwPedPtr
	_asm mov eax, [esi+1152]
	_asm mov dword ptr [eax+76], 0

	// DESTROY METHOD
	_asm mov ecx, dwPedPtr
	_asm mov ebx, [ecx] ; vtable
	_asm push 1
	_asm call [ebx] ; destroy

	m_pPed = NULL;
	m_pEntity = NULL;
}

//-----------------------------------------------------------
// If the game has internally destroyed the ped
// during this frame, the ped pointer should become 0

void CPlayerPed::ResetPointers()
{
	m_pPed = GamePool_Ped_GetAt(m_dwGTAId);
	m_pEntity = (ENTITY_TYPE *)m_pPed;
}

//-----------------------------------------------------------

void CPlayerPed::SetInitialState()
{
	DWORD dwPedPtr = (DWORD)m_pPed;
	//int iPlayerNumber = 0;
	//DWORD dwPlayerActorID = 0;

	_asm push 0
	_asm mov ecx, dwPedPtr
	_asm mov edx, 0x60CD20 ; internal_CPlayerPed_SetInitialState
	_asm call edx
	
	/* DESTROY METHOD
	_asm mov ecx, dwPedPtr
	_asm mov ebx, [ecx] ; vtable
	_asm push 1
	_asm call [ebx] ; destroy
	*/

	// CREATE PLAYER
	/*
	ScriptCommand(&create_player, &iPlayerNumber, x, y, z, &dwPlayerActorID);
	ScriptCommand(&create_actor_from_player,&iPlayerNumber,&dwPlayerActorID);

	m_dwGTAId = dwPlayerActorID;
	m_pPed = GamePool_Ped_GetAt(m_dwGTAId);
	m_bytePlayerNumber = 0;
	m_pEntity = (ENTITY_TYPE *)GamePool_Ped_GetAt(m_dwGTAId);
	*/
}

//-----------------------------------------------------------

void CPlayerPed::SetKeys(UINT uiKeys, WORD lrAnalog, WORD udAnalog)
{
	GTA_CONTROLSET *pPlayerKeys = GameGetPlayerKeys(m_bytePlayerNumber);
	
	 // LEFT/RIGHT
	 if(pPlayerKeys->wKeys1[0] && lrAnalog) {
		pPlayerKeys->wKeys1[0] = lrAnalog;
		pPlayerKeys->wKeys2[0] = lrAnalog;
	 } 
	 else if(lrAnalog && !pPlayerKeys->wKeys1[0]) {
		pPlayerKeys->wKeys1[0] = lrAnalog;
		pPlayerKeys->wKeys2[0] = 0;
	 }
	 else if(!lrAnalog) {
		pPlayerKeys->wKeys1[0] = 0;
		pPlayerKeys->wKeys2[0] = 0;
	 }

	 // UP/DOWN
	 if(pPlayerKeys->wKeys1[1] && udAnalog) {
		pPlayerKeys->wKeys1[1] = udAnalog;
		pPlayerKeys->wKeys2[1] = udAnalog;
	 } 
	 else if(udAnalog && !pPlayerKeys->wKeys1[1]) {
		pPlayerKeys->wKeys1[1] = udAnalog;
		pPlayerKeys->wKeys2[1] = 0;
	 }
	 else if(!udAnalog) {
		pPlayerKeys->wKeys1[1] = 0;
		pPlayerKeys->wKeys2[1] = 0;
	 }

	 // SECONDARY FIRE (4)
	 if(uiKeys & 1) {
		if(pPlayerKeys->wKeys1[4]) {
			pPlayerKeys->wKeys1[4] = 0xFF;
			pPlayerKeys->wKeys2[4] = 0xFF;
		} else {
			pPlayerKeys->wKeys1[4] = 0xFF;
			pPlayerKeys->wKeys2[4] = 0x00;
		}
	 } else { 
		pPlayerKeys->wKeys1[4] = 0x00;
		pPlayerKeys->wKeys2[4] = 0x00;
	 }
	 uiKeys >>= 1; // 1

	 // CROUCHING TOGGLE (18)
	 if(uiKeys & 1) {
		if(pPlayerKeys->wKeys1[18]) {
			pPlayerKeys->wKeys1[18] = 0xFF;
			pPlayerKeys->wKeys2[18] = 0xFF;
		} else {
			pPlayerKeys->wKeys1[18] = 0xFF;
			pPlayerKeys->wKeys2[18] = 0x00;
		}
	 } else {
		pPlayerKeys->wKeys1[18] = 0x00;
		pPlayerKeys->wKeys2[18] = 0x00;
	 }
	 uiKeys >>= 1; // 2

	  // FIRING (17)
	 if(uiKeys & 1) {
		if(pPlayerKeys->wKeys1[17]) {
			pPlayerKeys->wKeys1[17] = 0xFF;
			pPlayerKeys->wKeys2[17] = 0xFF;
		} else {
			pPlayerKeys->wKeys1[17] = 0xFF;
			pPlayerKeys->wKeys2[17] = 0x00;
		}
	 } else {
		pPlayerKeys->wKeys1[17] = 0x00;
		pPlayerKeys->wKeys2[17] = 0x00;
	 }
	 uiKeys >>= 1; // 3
	 
	  // SPRINT (16)
	 if(uiKeys & 1) {
		if(pPlayerKeys->wKeys1[16]) {
			pPlayerKeys->wKeys1[16] = 0xFF;
			pPlayerKeys->wKeys2[16] = 0xFF;
		} else {
			pPlayerKeys->wKeys1[16] = 0xFF;
			pPlayerKeys->wKeys2[16] = 0x00;
		}
	 } else {
		pPlayerKeys->wKeys1[16] = 0x00;
		pPlayerKeys->wKeys2[16] = 0x00;
	 }
	 uiKeys >>= 1; // 4


	 // SECONDARY ONFOOT ATTACK (15)
	 if( (uiKeys & 1) && !IsInJetpackMode() ) {
		if(pPlayerKeys->wKeys1[15]) {
			pPlayerKeys->wKeys1[15] = 0xFF;
			pPlayerKeys->wKeys2[15] = 0xFF;
		} else {
			pPlayerKeys->wKeys1[15] = 0xFF;
			pPlayerKeys->wKeys2[15] = 0x00;
		}
	 } else {
		pPlayerKeys->wKeys1[15] = 0x00;
		pPlayerKeys->wKeys2[15] = 0x00;
	 }
	 uiKeys >>= 1; // 5

	 // JUMP (14)
	 if(uiKeys & 1) {
		if(pPlayerKeys->wKeys1[14]) {
			pPlayerKeys->wKeys1[14] = 0xFF;
			pPlayerKeys->wKeys2[14] = 0xFF;
		} else {
			pPlayerKeys->wKeys1[14] = 0xFF;
			pPlayerKeys->wKeys2[14] = 0x00;
		}
	 } else {
		pPlayerKeys->wKeys1[14] = 0x00;
		pPlayerKeys->wKeys2[14] = 0x00;
	 }
	 uiKeys >>= 1; // 6

	 // INCAR LOOK RIGHT (7)
	 if(uiKeys & 1) {
		if(pPlayerKeys->wKeys1[7]) {
			pPlayerKeys->wKeys1[7] = 0xFF;
			pPlayerKeys->wKeys2[7] = 0xFF;
		} else {
			pPlayerKeys->wKeys1[7] = 0xFF;
			pPlayerKeys->wKeys2[7] = 0x00;
		}
	 } else {
		pPlayerKeys->wKeys1[7] = 0x00;
		pPlayerKeys->wKeys2[7] = 0x00;
	 }
	 uiKeys >>= 1; // 7

	 // INCAR HANDBRAKE / ONFOOT TARGET (6)
	 if(uiKeys & 1) {
		if(pPlayerKeys->wKeys1[6]) {
			pPlayerKeys->wKeys1[6] = 0xFF;
			pPlayerKeys->wKeys2[6] = 0xFF;
		} else {
			pPlayerKeys->wKeys1[6] = 0xFF;
			pPlayerKeys->wKeys2[6] = 0x00;
		}
	 } else {
		pPlayerKeys->wKeys1[6] = 0x00;
		pPlayerKeys->wKeys2[6] = 0x00;
	 }
	 uiKeys >>= 1; // 8

 	 // INCAR LOOK LEFT (5)
	 if(uiKeys & 1) {
		if(pPlayerKeys->wKeys1[5]) {
			pPlayerKeys->wKeys1[5] = 0xFF;
			pPlayerKeys->wKeys2[5] = 0xFF;
		} else {
			pPlayerKeys->wKeys1[5] = 0xFF;
			pPlayerKeys->wKeys2[5] = 0x00;
		}
	 } else {
		pPlayerKeys->wKeys1[5] = 0x00;
		pPlayerKeys->wKeys2[5] = 0x00;
	 }
	 uiKeys >>= 1; // 9

	 // SUBMISSION (19)
	 if(uiKeys & 1) {
		if(pPlayerKeys->wKeys1[19] == 0xFF) {
			pPlayerKeys->wKeys1[19] = 0xFF;
			pPlayerKeys->wKeys2[19] = 0xFF;
		} else {
			pPlayerKeys->wKeys1[19] = 0xFF;
			pPlayerKeys->wKeys2[19] = 0x00;
		}
	 } else { 
		pPlayerKeys->wKeys1[19] = 0x00;
		pPlayerKeys->wKeys2[19] = 0x00;
	 }
	 uiKeys >>= 1; // 10

	 // WALKING (21)
	 if(uiKeys & 1) {
		if(pPlayerKeys->wKeys1[21] == 0xFF) {
			pPlayerKeys->wKeys1[21] = 0xFF;
			pPlayerKeys->wKeys2[21] = 0xFF;
		} else {
			pPlayerKeys->wKeys1[21] = 0xFF;
			pPlayerKeys->wKeys2[21] = 0x00;
		}
	 } else { 
		pPlayerKeys->wKeys1[21] = 0x00;
		pPlayerKeys->wKeys2[21] = 0x00;
	 }
	 uiKeys >>= 1; // 11

	 // ANALOG2 D/U
	 BYTE byteVal = uiKeys & 3;
	
	 if(byteVal==2) {
		pPlayerKeys->wKeys1[3] = (WORD)(128);
		pPlayerKeys->wKeys2[3] = (WORD)(128);
	 } else if(byteVal==1) {
		pPlayerKeys->wKeys1[3] = (WORD)(-128);
		pPlayerKeys->wKeys2[3] = (WORD)(-128);
	 } else {
		pPlayerKeys->wKeys1[3] = 0;
		pPlayerKeys->wKeys2[3] = 0;
	 }
	 uiKeys >>= 2; // 12-13

	 // ANALOG2 L/R
	 byteVal = uiKeys & 3;
	
	 if(byteVal==2) {
		pPlayerKeys->wKeys1[2] = (WORD)(128);
		pPlayerKeys->wKeys2[2] = (WORD)(128);
	 } else if(byteVal==1) {
		pPlayerKeys->wKeys1[2] = (WORD)(-128);
		pPlayerKeys->wKeys2[2] = (WORD)(-128);
	 } else {
		pPlayerKeys->wKeys1[2] = 0;
		pPlayerKeys->wKeys2[2] = 0;
	 }
	 // 14-15

	 GameStoreRemotePlayerKeys(m_bytePlayerNumber,pPlayerKeys);
}

//-----------------------------------------------------------

UINT CPlayerPed::GetKeys(WORD* lrAnalog, WORD* udAnalog)
{
	UINT uiRet = 0;
	GTA_CONTROLSET *pInternalKeys = GameGetInternalKeys();
	bool bInVehicle = IsInVehicle();

	*lrAnalog = pInternalKeys->wKeys1[0]; // left/right analog
	*udAnalog = pInternalKeys->wKeys1[1]; // up/down analog

	if (pInternalKeys->wKeys1[13]) uiRet |= 1; // camera modes
	uiRet <<= 1;

	if (pInternalKeys->wKeys1[8]) uiRet |= 1; // recruit
	uiRet <<= 1;

	if (pInternalKeys->wKeys1[9]) uiRet |= 1; // honk
	uiRet <<= 1;

	if (pInternalKeys->wKeys1[10]) uiRet |= 1; // no
	uiRet <<= 1;

	if (pInternalKeys->wKeys1[11]) uiRet |= 1; // yes
	uiRet <<= 1;

	if(((short)pInternalKeys->wKeys1[2]) > 0) uiRet |= 1; // analog2 L
	uiRet <<= 1;

	if(((short)pInternalKeys->wKeys1[2]) < 0) uiRet |= 1; // analog2 R
	uiRet <<= 1;

	if(((short)pInternalKeys->wKeys1[3]) > 0) uiRet |= 1; // analog2 D
	uiRet <<= 1;

	if(((short)pInternalKeys->wKeys1[3]) < 0) uiRet |= 1; // analog2 U
	uiRet <<= 1;
	
	if(pInternalKeys->wKeys1[21]) uiRet |= 1; // walking
	uiRet <<= 1;

	if(pInternalKeys->wKeys1[19]) uiRet |= 1; // submission
	uiRet <<= 1;

	if(bInVehicle && pInternalKeys->wKeys1[5]) uiRet |= 1; // incar look left
	uiRet <<= 1;
	
	if(pInternalKeys->wKeys1[6]) uiRet |= 1; // incar handbrake / target
	uiRet <<= 1;

	if(bInVehicle && pInternalKeys->wKeys1[7]) uiRet |= 1; // incar look right
	uiRet <<= 1;

	if(pInternalKeys->wKeys1[14]) uiRet |= 1; // jump
	uiRet <<= 1;

	if(!IsInJetpackMode() && pInternalKeys->wKeys1[15]) uiRet |= 1; // secondary onfoot attack
	uiRet <<= 1;
	
	if(pInternalKeys->wKeys1[16]) uiRet |= 1; // sprint
	uiRet <<= 1;
		
	if(HasAmmoForCurrentWeapon() && pInternalKeys->wKeys1[17]) uiRet |= 1; // fire
	uiRet <<= 1;
	
	if(pInternalKeys->wKeys1[18]) uiRet |= 1; // crouch
	uiRet <<= 1;

	if(pInternalKeys->wKeys1[4]) uiRet |= 1; // secondary fire

	return uiRet;
}

//-----------------------------------------------------------

CAMERA_AIM * CPlayerPed::GetCurrentAim()
{
	return GameGetInternalAim();
}

//-----------------------------------------------------------

void CPlayerPed::SetCurrentAim(CAMERA_AIM *pAim)
{
	GameStoreRemotePlayerAim(m_bytePlayerNumber, pAim);
}

//-----------------------------------------------------------

BYTE CPlayerPed::GetCurrentWeapon()
{
	if(!m_pPed) return 0;
	if(GamePool_Ped_GetAt(m_dwGTAId) == 0) return 0;

	DWORD dwRetVal;
	ScriptCommand(&get_actor_armed_weapon,m_dwGTAId,&dwRetVal);
	return (BYTE)dwRetVal;
}

//-----------------------------------------------------------

int CPlayerPed::GetCurrentVehicleID()
{
	if(!m_pPed) return 0;

	VEHICLE_TYPE *pVehicle = (VEHICLE_TYPE *)m_pPed->pVehicle;
	return GamePool_Vehicle_GetIndex(pVehicle);
}

//-----------------------------------------------------------

void CPlayerPed::SetImmunities(int bBullet, int bFire, int bExplosion, int bCollision, int bMelee)
{
	if (!m_pPed) return;
	if (!GamePool_Ped_GetAt(m_dwGTAId)) return;

	ScriptCommand(&set_actor_immunities, m_dwGTAId, bBullet, bFire, bExplosion, bCollision, bMelee);
}

//-----------------------------------------------------------

// Shows the normal marker
void CPlayerPed::ShowMarker() //int iMarkerColor)
{	
	if (m_dwArrow) HideMarker();
	ScriptCommand(&create_arrow_above_actor, m_dwGTAId, &m_dwArrow);
	//ScriptCommand(&unknown_arrow,dwArrow,0);
	//ScriptCommand(&tie_marker_to_actor, m_dwGTAId, 0, 0, &dwArrow);
	ScriptCommand(&set_marker_color, m_dwArrow, m_bytePlayerNumber-2);
	ScriptCommand(&show_on_radar2, m_dwArrow, 2);
}

//-----------------------------------------------------------

// Shows an arrow above their head
/*void CPlayerPed::ShowMarkerEx()
{
	//ScriptCommand(&unknown_arrow,m_dwArrow,0);
	//ScriptCommand(&tie_marker_to_actor, m_dwGTAId, 0, 0, &m_dwArrow);
	ScriptCommand(&set_marker_color, m_dwArrow, m_bytePlayerNumber-2);
	ScriptCommand(&show_on_radar, m_dwArrow, 3);
}*/

//-----------------------------------------------------------

void CPlayerPed::HideMarker()
{	
	if (m_dwArrow) ScriptCommand(&disable_marker, m_dwArrow);
	m_dwArrow = NULL; // Just make sure
}

//-----------------------------------------------------------

bool CPlayerPed::IsOnScreen()
{
	if(m_pPed)	return GameIsEntityOnScreen((DWORD *)m_pPed);
	return false;
}

//-----------------------------------------------------------

float CPlayerPed::GetHealth()
{	
	if(!m_pPed) return 0.0f;
	return m_pPed->fHealth;
}

//-----------------------------------------------------------

void CPlayerPed::SetHealth(float fHealth)
{	
	if(!m_pPed) return;
	m_pPed->fHealth = fHealth;
}	

//-----------------------------------------------------------

float CPlayerPed::GetArmour()
{
	if(!m_pPed) return 0.0f;
	return m_pPed->fArmour;
}

//-----------------------------------------------------------

void CPlayerPed::SetArmour(float fArmour)
{
	if(!m_pPed) return;
	m_pPed->fArmour = fArmour;
}	

//-----------------------------------------------------------

DWORD CPlayerPed::GetStateFlags()
{	
	if(!m_pPed) return 0;
	return m_pPed->dwStateFlags;
}

//-----------------------------------------------------------

void CPlayerPed::SetStateFlags(DWORD dwState)
{	
	if(!m_pPed) return;
	m_pPed->dwStateFlags = dwState;
}	

//-----------------------------------------------------------

bool CPlayerPed::IsDead()
{
	if(!m_pPed) return true;
	if(m_pPed->fHealth > 0.0f) return false;
	return true;
}

//-----------------------------------------------------------

BYTE CPlayerPed::GetActionTrigger()
{
	return (BYTE)m_pPed->dwAction;
}

//-----------------------------------------------------------

void CPlayerPed::SetActionTrigger(BYTE byteTrigger)
{
	if(!m_pPed) return;

	m_pPed->dwAction = byteTrigger;
}

//-----------------------------------------------------------

bool CPlayerPed::IsInVehicle()
{
	if(!m_pPed) return false;

	if(IN_VEHICLE(m_pPed)) {
		return true;
	}
	return false;
}

//-----------------------------------------------------------

float CPlayerPed::GetTargetRotation()
{
	float fRotation;
	ScriptCommand(&get_actor_z_angle,m_dwGTAId,&fRotation);
	return fRotation;

	//return m_pPed->fRotation2;
}

//-----------------------------------------------------------

void CPlayerPed::SetTargetRotation(float fRotation)
{	
	//pChatWindow->AddDebugMessage("deg: %f rad: %f", fRotation, DegToRad(fRotation));
	
	if(!m_pPed) return;
	if(!GamePool_Ped_GetAt(m_dwGTAId)) return;

	m_pPed->fRotation2 = DegToRad(fRotation);

	if(!IsOnGround()) {
		m_pPed->fRotation1 = DegToRad(fRotation);
		ScriptCommand(&set_actor_z_angle,m_dwGTAId,fRotation);
	}
}

//-----------------------------------------------------------

void CPlayerPed::ForceTargetRotation(float fRotation)
{
	if(!m_pPed) return;
	if(!GamePool_Ped_GetAt(m_dwGTAId)) return;

	m_pPed->fRotation1 = DegToRad(fRotation);
	m_pPed->fRotation2 = DegToRad(fRotation);

	ScriptCommand(&set_actor_z_angle,m_dwGTAId,fRotation);    
}

//-----------------------------------------------------------

bool CPlayerPed::IsAPassenger()
{
	if( m_pPed->pVehicle && IN_VEHICLE(m_pPed) )
	{
		VEHICLE_TYPE * pVehicle = (VEHICLE_TYPE *)m_pPed->pVehicle;

		if( pVehicle->pDriver != m_pPed || 
			pVehicle->entity.nModelIndex == TRAIN_PASSENGER ||
			pVehicle->entity.nModelIndex == TRAIN_FREIGHT ) {
			return true;
		}
		else {
			return false;
		}
	}

	return false;
}

//-----------------------------------------------------------

VEHICLE_TYPE * CPlayerPed::GetGtaVehicle()
{
	return (VEHICLE_TYPE *)m_pPed->pVehicle;
}

//-----------------------------------------------------------

void CPlayerPed::GiveWeapon(int iWeaponID, int iAmmo)
{
	int iModelID = 0;
	iModelID = GameGetWeaponModelIDFromWeaponID(iWeaponID);

	if(iModelID == -1) return;

	if(!CGame::IsModelLoaded(iModelID)) {
		CGame::RequestModel(iModelID);
		CGame::LoadRequestedModels();
		while(!CGame::IsModelLoaded(iModelID)) Sleep(1);
	}

	//ScriptCommand(&give_actor_weapon,this->m_dwGTAId,iWeaponID,iAmmo);

	*pbyteCurrentPlayer = m_bytePlayerNumber;

	DWORD dwPedPtr = (DWORD)m_pPed;

	_asm mov ecx, dwPedPtr
	_asm push 1
	_asm push iAmmo
	_asm push iWeaponID
	_asm mov edx, 0x5E6080
	_asm call edx

	SetArmedWeapon(iWeaponID);

	*pbyteCurrentPlayer = 0;

	//pGame->RemoveModel(iModelID);
}


//-----------------------------------------------------------

void CPlayerPed::ClearAllWeapons()
{
	DWORD dwPedPtr = (DWORD)m_pPed;

	*pbyteCurrentPlayer = m_bytePlayerNumber;

	if(dwPedPtr) {
		_asm mov ecx, dwPedPtr
		_asm mov eax, 0x5E6320
		_asm call eax
	}

	*pbyteCurrentPlayer = 0;
}

//-----------------------------------------------------------

void CPlayerPed::SetArmedWeapon(int iWeaponType)
{
	*pbyteCurrentPlayer = m_bytePlayerNumber;

	ScriptCommand(&set_actor_armed_weapon,this->m_dwGTAId,iWeaponType);

	*pbyteCurrentPlayer = 0;
}

//-----------------------------------------------------------

WEAPON_SLOT_TYPE * CPlayerPed::GetCurrentWeaponSlot()
{
	if(m_pPed) {
		return &m_pPed->WeaponSlots[m_pPed->byteCurWeaponSlot];
	}
	return NULL;
}

//-----------------------------------------------------------

bool CPlayerPed::HasAmmoForCurrentWeapon()
{
	if(m_pPed) {
		WEAPON_SLOT_TYPE * WeaponSlot = GetCurrentWeaponSlot();

		if(!WeaponSlot) return true;
		
		// Melee types always have ammo.
		if( WeaponSlot->dwType <= WEAPON_CANE ||
			WeaponSlot->dwType == WEAPON_PARACHUTE ) return true;

		if(!WeaponSlot->dwAmmo) return false;
	}
	return true;
}

//-----------------------------------------------------------

float CPlayerPed::GetDistanceFromVehicle(CVehicle* pVehicle)
{
	MATRIX4X4 matPed, matVehicle;
	float fX, fY, fZ;

	pVehicle->GetMatrix(&matVehicle);
	GetMatrix(&matPed);

	fX = matPed.pos.X - matVehicle.pos.X;
	fY = matPed.pos.Y - matVehicle.pos.Y;
	fZ = matPed.pos.Z - matVehicle.pos.Z;

	return sqrtf(fX * fX + fY * fY + fZ * fZ);
}

//-----------------------------------------------------------

int CPlayerPed::GetVehicleSeatID()
{
	VEHICLE_TYPE *pVehicle;

	if( GetActionTrigger() == ACTION_INCAR && (pVehicle = (VEHICLE_TYPE *)m_pPed->pVehicle) != 0 ) {
		if(pVehicle->pDriver == m_pPed) return 0;
		if(pVehicle->pPassengers[0] == m_pPed) return 1;
		if(pVehicle->pPassengers[1] == m_pPed) return 2;
		if(pVehicle->pPassengers[2] == m_pPed) return 3;
		if(pVehicle->pPassengers[3] == m_pPed) return 4;
		if(pVehicle->pPassengers[4] == m_pPed) return 5;
		if(pVehicle->pPassengers[5] == m_pPed) return 6;
		if(pVehicle->pPassengers[6] == m_pPed) return 7;
	}

	return (-1);
}

//-----------------------------------------------------------

void CPlayerPed::PutDirectlyInVehicle(int iVehicleID, int iSeat)
{
	if(!m_pPed) return;
	if(!GamePool_Vehicle_GetAt(iVehicleID)) return;
	if(!GamePool_Ped_GetAt(m_dwGTAId)) return;
	
	if(GetCurrentWeapon() == WEAPON_PARACHUTE) {
		SetArmedWeapon(0);
	}

	VEHICLE_TYPE *pVehicle = GamePool_Vehicle_GetAt(iVehicleID);

	if(pVehicle->fHealth == 0.0f) return;

	// Check to make sure internal data structure of the vehicle hasn't been deleted
	// by checking if the vtbl points to CPlaceable_vtbl
	if (pVehicle->entity.vtable == 0x863C40) return;

	if(iSeat==0) {
		if(pVehicle->pDriver && IN_VEHICLE(pVehicle->pDriver))
		{
			//pChatWindow->AddDebugMessage("CPlayerPed::PutDirectlyInVehicle: vehicle already has a driver");
            return;
		}
		ScriptCommand(&put_actor_in_car,m_dwGTAId,iVehicleID);
	} else {
		iSeat--;
		ScriptCommand(&put_actor_in_car2,m_dwGTAId,iVehicleID,iSeat);
	}

	if(pNetGame) {
		CVehiclePool* pVehiclePool = pNetGame->GetVehiclePool();
		VEHICLEID TrainVehicleId = pVehiclePool->FindIDFromGtaPtr(pVehicle);
		if(TrainVehicleId == INVALID_VEHICLE_ID || TrainVehicleId > MAX_VEHICLES) return;

		CVehicle* pTrain = pVehiclePool->GetAt(TrainVehicleId);

		if ( pTrain && pTrain->IsATrainPart() && this == pNetGame->GetPlayerPool()->GetLocalPlayer()->m_pPlayerPed ) {
			ScriptCommand(&camera_on_vehicle, pTrain->m_dwGTAId, 3, 2);
		}
	}
}

//-----------------------------------------------------------

void CPlayerPed::EnterVehicle(int iVehicleID, bool bPassenger)
{
	if(!m_pPed) return;
	if(!GamePool_Vehicle_GetAt(iVehicleID)) return;
	if(!GamePool_Ped_GetAt(m_dwGTAId)) return;

	//bIgnoreNextEntry = TRUE;

	if(GetCurrentWeapon() == WEAPON_PARACHUTE) {
		SetArmedWeapon(0);
	}

	if(bPassenger) {
		//pChatWindow->AddDebugMessage("Passenger: %u 0x%X",m_bytePlayerNumber,iVehicleID);
		ScriptCommand(&send_actor_to_car_passenger,m_dwGTAId,iVehicleID,3000,-1);
	} else {
		//pChatWindow->AddDebugMessage("Driver: %u 0x%X",m_bytePlayerNumber,iVehicleID);
		ScriptCommand(&send_actor_to_car_driverseat,m_dwGTAId,iVehicleID,3000);
	}				
}

//-----------------------------------------------------------
// Graceful vehicle exit.

void CPlayerPed::ExitCurrentVehicle()
{
	if(!m_pPed) return;
	if(!GamePool_Ped_GetAt(m_dwGTAId)) return;
	if(IN_VEHICLE(m_pPed)) {
		ScriptCommand(&make_actor_leave_car,m_dwGTAId,GetCurrentVehicleID());
	}
}

//-----------------------------------------------------------
// Forceful removal

void CPlayerPed::RemoveFromVehicleAndPutAt(float fX, float fY, float fZ)
{
	if(!GamePool_Ped_GetAt(m_dwGTAId)) return;
	if(m_pPed && IN_VEHICLE(m_pPed)) {
		ScriptCommand(&remove_actor_from_car_and_put_at,m_dwGTAId,fX,fY,fZ);
	}
}

//-----------------------------------------------------------

void CPlayerPed::RestartIfWastedAt(VECTOR *vecRestart, float fRotation)
{	
	ScriptCommand(&restart_if_wasted_at, vecRestart->X,vecRestart->Y,vecRestart->Z,fRotation,0);
}

//-----------------------------------------------------------

void CPlayerPed::TogglePlayerControllable(int iControllable)
{
	MATRIX4X4 mat;

	if(!GamePool_Ped_GetAt(m_dwGTAId)) return;

	if(!iControllable) {
		ScriptCommand(&toggle_player_controllable,m_bytePlayerNumber,0);
		ScriptCommand(&lock_actor,m_dwGTAId,1);
	} else {
		ScriptCommand(&toggle_player_controllable,m_bytePlayerNumber,1);
		ScriptCommand(&lock_actor,m_dwGTAId,0);
		if(!IsInVehicle()) {
			GetMatrix(&mat);
			TeleportTo(mat.pos.X,mat.pos.Y,mat.pos.Z);
		}
	}
}

//-----------------------------------------------------------

BYTE CPlayerPed::FindDeathReasonAndResponsiblePlayer(BYTE * nPlayer)
{	
	BYTE byteDeathReason;
	BYTE bytePlayerIDWhoKilled;
	CVehiclePool *pVehiclePool;
	CPlayerPool *pPlayerPool;

	// grab the vehicle/player pool now anyway, even though we may not need it.
	if(pNetGame) {
		pVehiclePool = pNetGame->GetVehiclePool();
		pPlayerPool = pNetGame->GetPlayerPool();
	}
	else { // just leave if there's no netgame.
		*nPlayer = INVALID_PLAYER_ID;
		return INVALID_PLAYER_ID;
	}

	if(m_pPed) 
	{
		byteDeathReason = (BYTE)m_pPed->dwWeaponUsed;
		if(byteDeathReason < WEAPON_CAMERA || byteDeathReason == WEAPON_HELIBLADES || byteDeathReason == WEAPON_EXPLOSION) { // It's a weapon of some sort.

			if(m_pPed->pdwDamageEntity) { // check for a player pointer.
				
				bytePlayerIDWhoKilled = pPlayerPool->
					FindRemotePlayerIDFromGtaPtr((PED_TYPE *)m_pPed->pdwDamageEntity);

				if(bytePlayerIDWhoKilled != INVALID_PLAYER_ID) {
					// killed by another player with a weapon, this is all easy.
					*nPlayer = bytePlayerIDWhoKilled;
					return byteDeathReason;
				} else { // could be a vehicle
					if(pVehiclePool->FindIDFromGtaPtr((VEHICLE_TYPE *)m_pPed->pdwDamageEntity) != INVALID_VEHICLE_ID) {
						VEHICLE_TYPE *pGtaVehicle = (VEHICLE_TYPE *)m_pPed->pdwDamageEntity;
						bytePlayerIDWhoKilled = pPlayerPool->
							FindRemotePlayerIDFromGtaPtr((PED_TYPE *)pGtaVehicle->pDriver);
												
						if(bytePlayerIDWhoKilled != INVALID_PLAYER_ID) {
							*nPlayer = bytePlayerIDWhoKilled;
							return byteDeathReason;
						}
					}
				}
			}
			//else { // weapon was used but who_killed is 0 (?)
			*nPlayer = INVALID_PLAYER_ID;
			return INVALID_PLAYER_ID;
			//}
		}
		else if(byteDeathReason == WEAPON_DROWN) {
			*nPlayer = INVALID_PLAYER_ID;
			return WEAPON_DROWN;
		}
		else if(byteDeathReason == WEAPON_VEHICLE) {

			if(m_pPed->pdwDamageEntity) {
				// now, if we can find the vehicle
				// we can probably derive the responsible player.
				// Look in the vehicle pool for this vehicle.
				if(pVehiclePool->FindIDFromGtaPtr((VEHICLE_TYPE *)m_pPed->pdwDamageEntity) != INVALID_VEHICLE_ID)
				{
					VEHICLE_TYPE *pGtaVehicle = (VEHICLE_TYPE *)m_pPed->pdwDamageEntity;

					bytePlayerIDWhoKilled = pPlayerPool->
						FindRemotePlayerIDFromGtaPtr((PED_TYPE *)pGtaVehicle->pDriver);
											
					if(bytePlayerIDWhoKilled != INVALID_PLAYER_ID) {
						*nPlayer = bytePlayerIDWhoKilled;
						return WEAPON_VEHICLE;
					}
				}									
			}
		}
		else if(byteDeathReason == WEAPON_COLLISION) {

			if(m_pPed->pdwDamageEntity) {
				if(pVehiclePool->FindIDFromGtaPtr((VEHICLE_TYPE *)m_pPed->pdwDamageEntity) != INVALID_VEHICLE_ID)
				{
					VEHICLE_TYPE *pGtaVehicle = (VEHICLE_TYPE *)m_pPed->pdwDamageEntity;
										
					bytePlayerIDWhoKilled = pPlayerPool->
						FindRemotePlayerIDFromGtaPtr((PED_TYPE *)pGtaVehicle->pDriver);
						
					if(bytePlayerIDWhoKilled != INVALID_PLAYER_ID) {
						*nPlayer = bytePlayerIDWhoKilled;
						return WEAPON_COLLISION;
					}
				}
				else {
					*nPlayer = INVALID_PLAYER_ID;
					return WEAPON_COLLISION;
				}
			}
		}
	}

	// Unhandled death type.
	*nPlayer = INVALID_PLAYER_ID;
	return INVALID_PLAYER_ID;
}

//-----------------------------------------------------------

void CPlayerPed::HandsUp()
{
	if(!m_pPed || IN_VEHICLE(m_pPed)) return;
	if(!IsAdded()) return;
	if(!GamePool_Ped_GetAt(m_dwGTAId)) return;
	ScriptCommand(&actor_task_handsup,m_dwGTAId,-2);
}

//-----------------------------------------------------------

bool CPlayerPed::HasHandsUp()
{
	if(!m_pPed || IN_VEHICLE(m_pPed)) return false;
	if(!IsAdded()) return false;
	if(!GamePool_Ped_GetAt(m_dwGTAId)) return false;
	if(m_pPed->Tasks->pdwJumpJetPack == NULL) return false;
	DWORD dwJmpVtbl = m_pPed->Tasks->pdwJumpJetPack[0];
	if(dwJmpVtbl == 0x85A29C) return true;
	
	return false;
}

//-----------------------------------------------------------

bool CPlayerPed::IsJumpTask()
{
	if (!m_pPed || IN_VEHICLE(m_pPed) || !m_pPed->Tasks)
		return false;
	
	return GetTaskTypeFromTask(m_pPed->Tasks->pdwJumpJetPack) == 211;
}

//-----------------------------------------------------------

bool CPlayerPed::IsTakeDamageFallTask()
{
	if (!m_pPed || IN_VEHICLE(m_pPed) || !m_pPed->Tasks)
		return false;

	return GetTaskTypeFromTask(m_pPed->Tasks->pdwDamage) == 208;
}

//-----------------------------------------------------------

bool CPlayerPed::IsSwimTask()
{
	if (!m_pPed || IN_VEHICLE(m_pPed) || !m_pPed->Tasks)
		return false;

	return GetTaskTypeFromTask(m_pPed->Tasks->pdwSwimWasted) == 268;
}

//-----------------------------------------------------------

void CPlayerPed::HoldItem(int iObject)
{
	if(!m_pPed) return;
	if(!IsAdded()) return;
	if(!GamePool_Ped_GetAt(m_dwGTAId)) return;

	DWORD dwPed = (DWORD)m_pPed;
	_asm push 1
	_asm push iObject
	_asm mov ecx, dwPed
	_asm mov ebx, 0x5E4390
	_asm call ebx

}

//-----------------------------------------------------------

void CPlayerPed::StartJetpack()
{
	if(!m_pPed) return;

	*pbyteCurrentPlayer = m_bytePlayerNumber;

	_asm mov eax, 0x439600
	_asm call eax

	*pbyteCurrentPlayer = 0;
}

//-----------------------------------------------------------

void CPlayerPed::StopJetpack()
{
	if(!m_pPed || IN_VEHICLE(m_pPed)) return;

	if(m_pPed->Tasks->pdwJumpJetPack == NULL) return;

	DWORD dwJmpVtbl = m_pPed->Tasks->pdwJumpJetPack[0];

	if(dwJmpVtbl == 0x8705C4) {
		DWORD dwJetPackTask = (DWORD)m_pPed->Tasks->pdwJumpJetPack;
		_asm mov ecx, dwJetPackTask
		_asm mov edx, 0x6801D0
		_asm push 1
		_asm call edx
		m_pPed->Tasks->pdwJumpJetPack = 0;
	}
}

//-----------------------------------------------------------

bool CPlayerPed::IsInJetpackMode()
{
	if(!m_pPed || IN_VEHICLE(m_pPed)) return false;
	if(m_pPed->Tasks->pdwJumpJetPack == NULL) return false;

	DWORD dwJmpVtbl = m_pPed->Tasks->pdwJumpJetPack[0];

	if(dwJmpVtbl == 0x8705C4) return true;

	return false;
}

//-----------------------------------------------------------

bool CPlayerPed::StartPassengerDriveByMode()
{
	if(m_pPed) {

		if(!IN_VEHICLE(m_pPed) || !m_pPed->pVehicle) return false;

		int iWeapon = GetCurrentWeapon();
		
		// Don't allow them to enter driveby with a para
		if(iWeapon == WEAPON_PARACHUTE) {
			SetArmedWeapon(0);
			return false;
		}

		// Check for an uzi type weapon.
		if((iWeapon != WEAPON_UZI) && (iWeapon != WEAPON_MP5) && (iWeapon != WEAPON_TEC9)) {
			return false;
		}

		SetArmedWeapon(iWeapon);
	
		/* We should replace them in their current seat.
		int iVehicleID = GamePool_Vehicle_GetIndex((VEHICLE_TYPE *)m_pPed->pVehicle);
		int iSeatID = GetVehicleSeatID();
		PutDirectlyInVehicle(iVehicleID,iSeatID);*/	

		ScriptCommand(&enter_passenger_driveby,
			m_dwGTAId,-1,-1,0.0f,0.0f,0.0f,300.0f,8,1,100);

		//SetWeaponModelIndex(iWeapon);

		return true;
	}
	return false;
}

//-----------------------------------------------------------

bool CPlayerPed::PerformingDriveByFreeAimTask()
{
	if (!m_pPed) return false;
	if (!GamePool_Ped_GetAt(m_dwGTAId)) return false;
	if (!m_pPed->Tasks) return false;

	return GetTaskTypeFromTask(m_pPed->Tasks->pdwJumpJetPack) == 1022;
}

//-----------------------------------------------------------

void CPlayerPed::DestroyDriveByFreeAimTask()
{
	DWORD* pTask;

	if (!m_pPed) return;
	if (!GamePool_Ped_GetAt(m_dwGTAId)) return;
	if (!m_pPed->Tasks && !m_pPed->Tasks->pdwJumpJetPack) return;
	
	pTask = m_pPed->Tasks->pdwJumpJetPack;
	if (GetTaskTypeFromTask(pTask) == 1022)
	{
		_asm {
			mov ecx, pTask
			mov edx, 0x6275E0
			push 1
			call edx
		}
		m_pPed->Tasks->pdwJumpJetPack = 0;
	}
}

//-----------------------------------------------------------

void CPlayerPed::SetModelIndex(UINT uiModel)
{
	DWORD dwPedPtr = (DWORD)m_pPed;
	if(!GamePool_Ped_GetAt(m_dwGTAId)) return;

	if(!IsValidModel(uiModel)) uiModel = 0;

	if(dwPedPtr) {

		// CPlayerPed_UpdateAfterPhysicalChange nulled
		//UnFuck(0x5A82C0,1);
		*(BYTE *)0x5A82C0 = 0xC3;

		DestroyFollowPedTask();

		CEntity::SetModelIndex(uiModel);

		// Need to reset the Ped Audio Attributes.
		_asm mov eax, dwPedPtr
		_asm lea ecx, [eax+660]
		_asm push eax
		_asm mov ebx, 0x4E68D0
		_asm call ebx
	}
}

//-----------------------------------------------------------

void CPlayerPed::SetDead()
{
	if(m_dwGTAId && m_pPed) {

		if(IsInJetpackMode()) StopJetpack();

		try {
			ExtinguishFire();
		} catch(...) {}

		MATRIX4X4 mat;
		GetMatrix(&mat);
		// will reset the tasks
		TeleportTo(mat.pos.X,mat.pos.Y,mat.pos.Z);
        m_pPed->fHealth = 0.0f;
		*pbyteCurrentPlayer = m_bytePlayerNumber;
		ScriptCommand(&kill_actor,m_dwGTAId);
		*pbyteCurrentPlayer = 0;
	}
}

//-----------------------------------------------------------

void CPlayerPed::ExtinguishFire()
{
	if (m_pPed)
	{
		// Remove ped's fire if any
		DWORD pFireObject = m_pPed->pFireObject;
		if (pFireObject != NULL)
		{
			_asm 
			{
				mov ecx, pFireObject;
				//and byte ptr [ecx], 253;
				mov eax, 0x5393F0;                         // CFire__Extinguish
				call eax;
			}
		}

		// Remove ped's vehicle's fire if any
		if (IN_VEHICLE(m_pPed))
		{
			VEHICLE_TYPE *pVeh = (VEHICLE_TYPE *)m_pPed->pVehicle;
			pFireObject = pVeh->pFireObject;

			if (pFireObject)
			{
				_asm 
				{
					mov ecx, pFireObject;
					//and byte ptr [ecx], 253;
					mov eax, 0x5393F0;                         // CFire__Extinguish
					call eax;
				}
			}
		}
	}
}

//-----------------------------------------------------------

WORD CPlayerPed::GetAmmo()
{
	if(m_pPed) {
		WEAPON_SLOT_TYPE * WeaponSlot = GetCurrentWeaponSlot();

		if(!WeaponSlot) return -1;
		
		// Melee types always have ammo.
		if( WeaponSlot->dwType <= WEAPON_CANE ||
			WeaponSlot->dwType == WEAPON_PARACHUTE ) return -1;

		return (WORD)WeaponSlot->dwAmmo;
	}
	return 0;
}

//-----------------------------------------------------------

void CPlayerPed::SetAmmo(BYTE byteWeapon, WORD wordAmmo)
{
	if(m_pPed)
	{
		//WEAPON_SLOT_TYPE * WeaponSlot = GetCurrentWeaponSlot();
		WEAPON_SLOT_TYPE * WeaponSlot = FindWeaponSlot((DWORD)byteWeapon);
		if(!WeaponSlot) return;
		WeaponSlot->dwAmmo = (DWORD)wordAmmo;
		//WeaponSlot->dwAmmoInClip = 0;
	}
}

//-----------------------------------------------------------

WEAPON_SLOT_TYPE * CPlayerPed::FindWeaponSlot(DWORD dwWeapon)
{
	if (m_pPed)
	{
		BYTE i;
		for (i = 0; i < 13; i++)
		{
			if (m_pPed->WeaponSlots[i].dwType == dwWeapon) return &m_pPed->WeaponSlots[i];
		}
	}
	return NULL;
}

//-----------------------------------------------------------

void CPlayerPed::SetAnimationSet(PCHAR szAnim)
{
	if(m_pPed) {
		ScriptCommand(&set_actor_animation_set,m_dwGTAId,szAnim);
	}
}

//-----------------------------------------------------------

void CPlayerPed::SetMoney(int iAmount)
{
	ScriptCommand(&set_actor_money,m_dwGTAId,0);
	ScriptCommand(&set_actor_money,m_dwGTAId,iAmount);
}

//-----------------------------------------------------------

void CPlayerPed::ApplyAnimation( char *szAnimName, char *szAnimFile, float fT,
								 int opt1, int opt2, int opt3, int opt4, int iUnk )
{
	//bool bLoaded = false;
	int iWaitAnimLoad=0;

	
	if(!m_pPed) return;
	if(!GamePool_Ped_GetAt(m_dwGTAId)) return;
	if(IsPissing()) StopPissing();
	if(IsBeer()) StopBeer();
    
#ifdef _DEBUG
	//if(pChatWindow) pChatWindow->AddDebugMessage("Anim(%s,%s,%f,%d,%d,%d,%d,%d)",
		//szAnimName,szAnimFile,fT,opt1,opt2,opt3,opt4,iUnk);
#endif

	int iRet = CGame::IsAnimationLoaded(szAnimFile);
	if (iRet == 0) {
		CGame::RequestAnimation(szAnimFile);
		while(!CGame::IsAnimationLoaded(szAnimFile)) {
			Sleep(5);
			iWaitAnimLoad++;
			if(iWaitAnimLoad == 10) return; // we can't wait forever
		}		
		//bLoaded = true;
	} else if (iRet == -1)
		return;

	ScriptCommand(&apply_animation,m_dwGTAId,szAnimName,szAnimFile,fT,opt1,opt2,opt3,opt4,iUnk);

	//if (bLoaded) {
		// not sure about this (how many anim files can we keep loaded?)
		//pGame->ReleaseAnimation(szAnimFile);
	//}
}

//-----------------------------------------------------------

bool CPlayerPed::IsPerformingAnimation(char *szAnimName)
{
	if(m_pPed && ScriptCommand(&is_actor_performing_anim,m_dwGTAId,szAnimName)) {
		return true;
	}
	return false;	
}

//-----------------------------------------------------------

void CPlayerPed::SetInterior(BYTE byteID)
{
	if(!m_pPed) return;

	ScriptCommand(&select_interior,byteID);
	ScriptCommand(&link_actor_to_interior,m_dwGTAId,byteID);

	MATRIX4X4 mat;
	GetMatrix(&mat);
	ScriptCommand(&refresh_streaming_at,mat.pos.X,mat.pos.Y);
}

//-----------------------------------------------------------

void CPlayerPed::CheckVehicleParachute()
{
	if(m_dwParachuteObject)
	{
		ScriptCommand(&disassociate_object,m_dwParachuteObject,0.0f,0.0f,0.0f,0);
		ScriptCommand(&destroy_object,m_dwParachuteObject);
		m_dwParachuteObject = 0;
	}
}

//-----------------------------------------------------------

void CPlayerPed::ProcessParachutes()
{
	if(!GamePool_Ped_GetAt(m_dwGTAId)) return;
	if(!IsAdded()) return;

	if(m_iParachuteState == 0) {

		if(m_dwParachuteObject) {
			ScriptCommand(&disassociate_object,m_dwParachuteObject,0.0f,0.0f,0.0f,0);
			ScriptCommand(&destroy_object_with_fade,m_dwParachuteObject);
			m_dwParachuteObject = 0;
		}

		// See if we should enter the initial parachuting state.
		if(GetCurrentWeapon() == WEAPON_PARACHUTE) { // armed with para
			if(ScriptCommand(&is_actor_falling_think,m_dwGTAId)) { // is falling

				float fDistanceFromGround;

				ScriptCommand(&get_actor_distance_from_ground,m_dwGTAId,&fDistanceFromGround);
				if(fDistanceFromGround > 20.0f) {
					//ScriptCommand(&actor_set_collision,m_dwGTAId,0);
					m_iParachuteState = 1;
					m_iParachuteAnim = 0;
				}
			}
		}
		return;
	}
	
	if( (GetCurrentWeapon() != WEAPON_PARACHUTE) || 
		ScriptCommand(&is_actor_in_the_water,m_dwGTAId) ) {
		// A parachuting state is active, but they no longer have the parachute
		// or they've ended up in the water.
		if(m_dwParachuteObject) {
			MATRIX4X4 mat;
			ScriptCommand(&disassociate_object,m_dwParachuteObject,0.0f,0.0f,0.0f,0);
			ScriptCommand(&destroy_object_with_fade,m_dwParachuteObject);
			//ScriptCommand(&actor_set_collision,m_dwGTAId,1);
			GetMatrix(&mat);
			TeleportTo(mat.pos.X,mat.pos.Y,mat.pos.Z);						
			m_dwParachuteObject = 0;			
		}
		m_iParachuteState = 0;
		m_iParachuteAnim = 0;
	}		

	if(m_iParachuteState == 1) {
		ProcessParachuteSkydiving();
		return;
	}
	
	if(m_iParachuteState == 2) {
		ProcessParachuting();
		return;
	}	
}

//-----------------------------------------------------------

void CPlayerPed::ProcessParachuteSkydiving()
{
	GTA_CONTROLSET *pPlayerControls = GameGetPlayerKeys(m_bytePlayerNumber);
	short sUpDown = (short)pPlayerControls->wKeys1[1];
	MATRIX4X4 mat;

	if((sUpDown < 0) && (m_iParachuteAnim != FALL_SKYDIVE_ACCEL)) {
		ApplyAnimation("FALL_SKYDIVE_ACCEL","PARACHUTE",1.0f,1,0,0,1,-2);
		m_iParachuteAnim = FALL_SKYDIVE_ACCEL;
	}
	else if((sUpDown >= 0) && (m_iParachuteAnim != FALL_SKYDIVE)) {
		ApplyAnimation("FALL_SKYDIVE","PARACHUTE",1.0f,1,0,0,1,-2);
		m_iParachuteAnim = FALL_SKYDIVE;
	}

	// if the parachute object isn't created, do it now.
	if(!m_dwParachuteObject) {
		GetMatrix(&mat);
		ScriptCommand(&create_object,OBJECT_PARACHUTE,mat.pos.X,mat.pos.Y,mat.pos.Z,&m_dwParachuteObject);
		
		if(!GamePool_Object_GetAt(m_dwParachuteObject)) {
			m_dwParachuteObject = 0;
			return;
		}

		ScriptCommand(&attach_object_to_actor,m_dwParachuteObject,m_dwGTAId,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f);
		ScriptCommand(&set_object_visible,m_dwParachuteObject,0);
	}

	if(!GamePool_Object_GetAt(m_dwParachuteObject)) {
		m_dwParachuteObject = 0;
		return;
	}

	// process parachute opening event
	if(pPlayerControls->wKeys1[17]) {
		ApplyAnimation("PARA_OPEN","PARACHUTE",8.0f,0,0,0,1,-2);
		ScriptCommand(&apply_object_animation,m_dwParachuteObject,"PARA_OPEN_O","PARACHUTE",1000.0f,0,1);
		ScriptCommand(&set_object_visible,m_dwParachuteObject,1);
		ScriptCommand(&set_object_scaling,m_dwParachuteObject,1.0f);
		m_iParachuteState = 2;
		m_iParachuteAnim = 0;
	}
}

//-----------------------------------------------------------

void CPlayerPed::ProcessParachuting()
{	
	GTA_CONTROLSET *pPlayerControls = GameGetPlayerKeys(m_bytePlayerNumber);
	short sUpDown = (short)pPlayerControls->wKeys1[1];

	if((sUpDown > 0) && (m_iParachuteAnim != PARA_DECEL)) {
		ApplyAnimation("PARA_DECEL","PARACHUTE",1.0f,1,0,0,1,-2);
		ScriptCommand(&apply_object_animation,m_dwParachuteObject,"PARA_DECEL_O","PARACHUTE",1.0f,1,1);
		m_iParachuteAnim = PARA_DECEL;
	}
	else if((sUpDown <= 0) && (m_iParachuteAnim != PARA_FLOAT)) {
		ApplyAnimation("PARA_FLOAT","PARACHUTE",1.0f,1,0,0,1,-2);
		ScriptCommand(&apply_object_animation,m_dwParachuteObject,"PARA_FLOAT_O","PARACHUTE",1.0f,1,1);
		m_iParachuteAnim = PARA_FLOAT;
	}
}

//-----------------------------------------------------------

bool CPlayerPed::IsOnGround()
{
	if(m_pPed) {
		if(m_pPed->dwStateFlags & 3) {
			return true;
		}
	}
	return false;
}

//-----------------------------------------------------------

void CPlayerPed::ResetDamageEntity()
{
	if(m_pPed) {
		m_pPed->pdwDamageEntity = 0;
		m_pPed->dwWeaponUsed = 255;
	}
}

//-----------------------------------------------------------

void CPlayerPed::SetWeaponModelIndex(int iWeapon)
{
	DWORD dwWeaponInfo;

	if(m_pPed) {
		DWORD dwPed = (DWORD)m_pPed;
		dwWeaponInfo = pGame->GetWeaponInfo(iWeapon,1);
		
		_asm mov ebx, dwWeaponInfo
		_asm mov eax, [ebx+12]

		_asm push eax
		_asm mov ecx, dwPed
		_asm mov edx, 0x5E3990
		_asm call edx
	}
}

//-----------------------------------------------------------

float CPlayerPed::GetAimZ()
{
	if(m_pPed) {
		DWORD dwPlayerInfo = m_pPed->dwPlayerInfoOffset;

		float fAimZ;
		_asm mov eax, dwPlayerInfo
		_asm mov ebx, [eax+84]
		_asm mov fAimZ, ebx

		return fAimZ;
	}
	return 0.0f;
}

//-----------------------------------------------------------

void CPlayerPed::SetAimZ(float fAimZ)
{
	if (!_isnan(fAimZ) && fAimZ <= 100.0f && fAimZ >= -100.0f)
	{
		if (m_pPed) {
			DWORD dwPlayerInfo = m_pPed->dwPlayerInfoOffset;
			_asm mov eax, dwPlayerInfo
			_asm mov ebx, fAimZ
			_asm mov[eax + 84], ebx
		}
	}
}
//-----------------------------------------------------------

ENTITY_TYPE* CPlayerPed::GetGtaContactEntity()
{
	if (!m_pPed->pContactEntity) return NULL;
	return (ENTITY_TYPE*)m_pPed->pContactEntity;
}

//-----------------------------------------------------------

VEHICLE_TYPE* CPlayerPed::GetGtaContactVehicle()
{
	return (VEHICLE_TYPE*)m_pPed->pContactVehicle;
}

//-----------------------------------------------------------

int CPlayerPed::GetPedStat()
{
	if(!m_pPed) return -1;
	return Game_PedStatPrim(m_pPed->entity.nModelIndex);
}

//-----------------------------------------------------------

bool CPlayerPed::IsPerformingCustomAnim()
{
	if(!m_pPed) return false;
	if(!IsAdded()) return false;

	if(m_pPed->Tasks->pdwJumpJetPack) {
		return true;
	}

	return false;
}

//-----------------------------------------------------------

char DanceStyleLibs[4][16] = {"WOP","GFUNK","RUNNINGMAN","STRIP"};
char DanceIdleLoops[4][16] = {"DANCE_LOOP","DANCE_LOOP","DANCE_LOOP","STR_Loop_B"};

char szDanceAnimNamesFemale[16][16] = {
	"DANCE_G1","DANCE_G2","DANCE_G3","DANCE_G4",
	"DANCE_G5","DANCE_G6","DANCE_G7","DANCE_G8",
	"DANCE_G9","DANCE_G10","DANCE_G11","DANCE_G12",
	"DANCE_G13","DANCE_G14","DANCE_G15","DANCE_G16"
};

char szDanceAnimNamesMale[16][16] = {
	"DANCE_B1","DANCE_B2","DANCE_B3","DANCE_B4",
	"DANCE_B5","DANCE_B6","DANCE_B7","DANCE_B8",
	"DANCE_B9","DANCE_B10","DANCE_B11","DANCE_B12",
	"DANCE_B13","DANCE_B14","DANCE_B15","DANCE_B16"
};

char szStripAnims[16][16] = {
	"strip_A","strip_B","strip_C","strip_D",
	"strip_E","strip_F","strip_G","STR_A2B",
	"STR_B2A","STR_B2C","STR_C1","STR_C2",
	"STR_C2B","STR_A2B","STR_B2C","STR_C2"
};

void CPlayerPed::StartDancing(int iStyle)
{	
	if(iStyle < 0 || iStyle > 3) return;

	/* if(iStyle == 3 && GetPedStat() != 5 && GetPedStat() != 22) {
		// stops male stripping
		return;
	}*/

	m_iDanceState = 1;
	m_iDanceStyle = iStyle;

	//if(!m_bytePlayerNumber) ?
	ApplyAnimation(DanceIdleLoops[m_iDanceStyle],DanceStyleLibs[m_iDanceStyle],16.0,1,0,0,0,-1);
}

//-----------------------------------------------------------

void CPlayerPed::StopDancing()
{
	m_iDanceState = 0;
	MATRIX4X4 mat;
	GetMatrix(&mat);
	TeleportTo(mat.pos.X,mat.pos.Y,mat.pos.Z);
}

//-----------------------------------------------------------

bool CPlayerPed::IsDancing()
{
	if(m_iDanceState) return true;
	return false;
}

//-----------------------------------------------------------

void CPlayerPed::ProcessDancing()
{
	GTA_CONTROLSET *pPlayerControls;
	int iNewMove=0;

	if(!m_iDanceState) return;
	if(!m_pPed) return;
	if(!IsAdded()) return;
	
	if(!m_bytePlayerNumber) {
		pPlayerControls = GameGetInternalKeys();
	} else {
		pPlayerControls = GameGetPlayerKeys(m_bytePlayerNumber);
	}

	short sUpDown = (short)pPlayerControls->wKeys2[1];
	short sLeftRight = (short)pPlayerControls->wKeys2[0];

	if(!IsPerformingCustomAnim()) {
		ApplyAnimation(DanceIdleLoops[m_iDanceStyle],DanceStyleLibs[m_iDanceStyle],4.0,1,0,0,0,0);
	}

	int iExtra = pPlayerControls->wKeys1[16];
	
	if(sUpDown > 0 && !sLeftRight && !iExtra) iNewMove = 0; // UP
	else if(sUpDown < 0 && !sLeftRight && !iExtra) iNewMove = 1; // DOWN
	else if(!sUpDown && sLeftRight < 0 && !iExtra) iNewMove = 2; // LEFT
	else if(!sUpDown && sLeftRight > 0 && !iExtra) iNewMove = 3; // RIGHT
	else if(sUpDown > 0 && sLeftRight < 0 && !iExtra) iNewMove = 4; // UP-LEFT
	else if(sUpDown > 0 && sLeftRight > 0 && !iExtra) iNewMove = 5; // UP-RIGHT
	else if(sUpDown < 0 && sLeftRight < 0 && !iExtra) iNewMove = 6; // DOWN-LEFT
	else if(sUpDown < 0 && sLeftRight > 0 && !iExtra) iNewMove = 7; // DOWN-RIGHT
	else if(sUpDown > 0 && !sLeftRight && iExtra) iNewMove = 8; // UP/EX
	else if(sUpDown < 0 && !sLeftRight && iExtra) iNewMove = 9; // DOWN/EX
	else if(!sUpDown && sLeftRight < 0 && iExtra) iNewMove = 10; // LEFT/EX
	else if(!sUpDown && sLeftRight > 0 && iExtra) iNewMove = 11; // RIGHT/EX
	else if(sUpDown > 0 && sLeftRight < 0 && iExtra) iNewMove = 12; // UP-LEFT/EX
	else if(sUpDown > 0 && sLeftRight > 0 && iExtra) iNewMove = 13; // UP-RIGHT/EX
	else if(sUpDown < 0 && sLeftRight < 0 && iExtra) iNewMove = 14; // DOWN-LEFT/EX
	else if(sUpDown < 0 && sLeftRight > 0 && iExtra) iNewMove = 15; // DOWN-RIGHT/EX
	else return;

	if(iNewMove == m_iLastDanceMove) return; // don't allow the same move twice

	m_iLastDanceMove = iNewMove;

	char * szAnimName = GetDanceAnimForMove(iNewMove);
	if(!szAnimName) return;

	ApplyAnimation(szAnimName,DanceStyleLibs[m_iDanceStyle],4.0,0,0,0,0,0);
}

//-----------------------------------------------------------

char * CPlayerPed::GetDanceAnimForMove(int iMove)
{
	if(!m_iDanceState) return ""; // shouldn't ever happen but..

	// style 0-2 have the same anim names, but M/F versions
	if(m_iDanceStyle >= 0 && m_iDanceStyle <= 2) {
		if(GetPedStat() == 5 || GetPedStat() == 22) { // female or pro (only female types)
			return szDanceAnimNamesFemale[iMove];
		}
		return szDanceAnimNamesMale[iMove];
	} else if(m_iDanceStyle == 3) {
		return szStripAnims[iMove];
	}
	return "";
}

//-----------------------------------------------------------

void CPlayerPed::ProcessMarkers(bool bMarkerStreamingEnabled, float fMarkerStreamRadius, bool bVisible)
{
	if(!m_pPed) return;

	// Add or remove player markers.
	if (bVisible && (!bMarkerStreamingEnabled || GetDistanceFromLocalPlayerPed() < fMarkerStreamRadius)) {
		if (m_byteCreateMarker && !m_dwArrow) ShowMarker(); 
	}
	else {
		if (m_dwArrow) HideMarker();
	}
}

//-----------------------------------------------------------

void CPlayerPed::ApplyCommandTask(char *szTaskName, int p1, int p2, int p3, 
								  VECTOR *p4, int p5, float p6, int p7, int p8, int p9)
{
	DWORD dwPed = (DWORD)m_pPed;
	if(!dwPed) return;

    _asm push p9
	_asm push p8
	_asm push p7
	_asm push p6
	_asm push p5
	_asm push p4
	_asm push p3
	_asm push p2
	_asm push p1
	_asm push dwPed
	_asm push szTaskName
	_asm mov ecx, 0xC15448
	_asm mov edx, 0x618970
	_asm call edx
}

//-----------------------------------------------------------

DWORD dwExt4;

void CPlayerPed::DestroyFollowPedTask()
{    
	if(!m_pPed) return;
	dwExt4 = (DWORD)m_pPed->Tasks->pdwExtUnk4;
	if(!dwExt4) return;

	_asm mov ecx, dwExt4
	_asm mov edx, 0x639330
	_asm push 1
	_asm call edx

	m_pPed->Tasks->pdwExtUnk4 = 0;
}

//-----------------------------------------------------------

void CPlayerPed::ToggleCellphone(int iOn)
{
	if(!m_pPed) return;
	m_iCellPhoneEnabled = iOn;
	ScriptCommand(&toggle_actor_cellphone,m_dwGTAId,iOn);
}

//-----------------------------------------------------------

int CPlayerPed::IsCellphoneEnabled()
{
    return m_iCellPhoneEnabled;
}

//-----------------------------------------------------------

void CPlayerPed::StartPissing()
{
	if(!m_pPed) return;
	if(!GamePool_Ped_GetAt(m_dwGTAId)) return;

	if(m_iPissingState != 0) {
		// already started
		return;
	}

	ApplyAnimation("PISS_LOOP", "PAULNMAC", 4.0f, 1, 0, 0, 0, -1);

	//opcode_066a('PETROLCAN', lhActor0, 0.0, 0.58, -0.08, 0.0, 0.01, 0.0, 1, l000f);

	ScriptCommand(&attach_particle_to_actor2,"PETROLCAN",m_dwGTAId, 
		0.0f, 0.58f, -0.08f, 0.0f, 0.01f, 0.0f, 1, &m_dwPissParticlesHandle);

	ScriptCommand(&make_particle_visible,m_dwPissParticlesHandle);

	m_iPissingState = 1;	
}

//-----------------------------------------------------------

void CPlayerPed::StopPissing()
{
	if(!m_pPed) return;
	if(!GamePool_Ped_GetAt(m_dwGTAId)) return;
	if(!m_iPissingState) return;

	if(m_dwPissParticlesHandle) {
		ScriptCommand(&destroy_particle,m_dwPissParticlesHandle);
		m_dwPissParticlesHandle = 0;
	}

	MATRIX4X4 mat;
	GetMatrix(&mat);
	TeleportTo(mat.pos.X,mat.pos.Y,mat.pos.Z);
	
	//ApplyAnimation("PISS_OUT", "PAULNMAC", 4.0f, 0, 0, 0, 0, -1);

	m_iPissingState = 0;
}

//-----------------------------------------------------------

int CPlayerPed::IsPissing()
{
	//if(!m_pPed) return 0;
	//if(!GamePool_Ped_GetAt(m_dwGTAId)) return 0;
	return m_iPissingState;
}

//-----------------------------------------------------------

void CPlayerPed::StartBeer()
{
	if(!m_pPed) return;
	if(!GamePool_Ped_GetAt(m_dwGTAId)) return;

	ScriptCommand(&set_camera_drunk,0,100);
	m_iBeerState = 1;
}

void CPlayerPed::StopBeer()
{
	if(!m_pPed) return;
	if(!GamePool_Ped_GetAt(m_dwGTAId)) return;
	
	ScriptCommand(&set_camera_drunk,0,0);
	m_iBeerState = 0;
}

int CPlayerPed::IsBeer()
{
	return m_iBeerState;
}

void CPlayerPed::StopSpecialAction()
{
	if(IsInJetpackMode()) StopJetpack();

	if(IsDancing()) StopDancing();

	if(HasHandsUp()) {
		MATRIX4X4 mat;
		GetMatrix(&mat);
		TeleportTo(mat.pos.X,mat.pos.Y,mat.pos.Z);
	}

	if(IsCellphoneEnabled()) ToggleCellphone(0);

	if(IsBeer()) StopBeer();

	if(IsPissing()) StopPissing();
}

void CPlayerPed::ProcessVehicleHorn()
{
	if(!m_pPed) return;
	if(!GamePool_Ped_GetAt(m_dwGTAId)) return;

	GTA_CONTROLSET *pPlayerControls;
	bool bHasHydraulics = false;

	if(!m_bytePlayerNumber) {
		pPlayerControls = GameGetInternalKeys();
	} else {
		pPlayerControls = GameGetPlayerKeys(m_bytePlayerNumber);
	}

	VEHICLE_TYPE *pGtaVehicle = (VEHICLE_TYPE *)m_pPed->pVehicle;
	if(pGtaVehicle) {
		for (unsigned i = 0; i < 15; i++) {
			if (pGtaVehicle->sComponent[i] == 1087) {
				bHasHydraulics = true;
				break;
			}
		}
		if(IN_VEHICLE(m_pPed) && (pGtaVehicle->pDriver == m_pPed) && !bHasHydraulics) {
			if(pPlayerControls->wKeys1[18]) {
                //pGtaVehicle->byteHorn = 1;
				pGtaVehicle->byteHorn2 = 1;
			} else {
				//pGtaVehicle->byteHorn = 0;
				pGtaVehicle->byteHorn2 = 0;
			}
		}
	}
}

//-----------------------------------------------------------

void CPlayerPed::SetFightingStyle(unsigned char style, unsigned char move)
{
	// from OPCODE_07FE
	if (m_pPed)
	{
		*(BYTE*)(m_pPed + 0x72D) = style;
		switch (move)
		{
		//case 0:
		default:
			*(BYTE*)(m_pPed + 0x72E) = 0;
			break;
		case 1:
			*(BYTE*)(m_pPed + 0x72E) = 1;
			break;
		case 2:
			*(BYTE*)(m_pPed + 0x72E) = 2;
			break;
		case 3:
			*(BYTE*)(m_pPed + 0x72E) = 4;
			break;
		case 4:
			*(BYTE*)(m_pPed + 0x72E) = 8;
			break;
		case 5:
			*(BYTE*)(m_pPed + 0x72E) = 15;
			break;
		}
	}
}

void CPlayerPed::GetFightingStyle(unsigned char* style, unsigned char* move)
{
	*style = *(BYTE*)(m_pPed + 0x72D);
	*move = *(BYTE*)(m_pPed + 0x72E);
}

bool CPlayerPed::IsDucking()
{
	return (m_pPed->dwStateFlags >> 26) & 1;
}

unsigned char CPlayerPed::IsEnteringVehicle()
{
	if (m_pPed && m_pPed->Tasks && m_pPed->Tasks->pdwJumpJetPack) {
		int iType = GetTaskTypeFromTask(m_pPed->Tasks->pdwJumpJetPack);
		if (iType == 700 || iType == 712)
			return 2;
		if (iType == 701 || iType == 713)
			return 1;
	}
	return 0;
}

bool CPlayerPed::IsExitingVehicle()
{
	if (m_pPed && m_pPed->Tasks && m_pPed->Tasks->pdwJumpJetPack) {
		if (GetTaskTypeFromTask(m_pPed->Tasks->pdwJumpJetPack) == 704)
			return true;
	}
	return false;
}

/*
	TODO: Needs further checking.

	Bug in SA-MP:
	Whatever you gave it to SetPlayerShopName, ("AMMUN1", "", " ", etc... Well, somehow 30 "x" did nothing..)
	it disables every/whatever interior scripts the ped in. Resets after reentering the interior.
	Apperently LoadShoppingData() deletes it, and ped + 1932 will not do anything.
*/

void CPlayerPed::SetEntryExit(char* szSectionName)
{
	if (!m_pPed) return;
	if (!GamePool_Ped_GetAt(m_dwGTAId)) return;

	m_pPed->pEntryExit = NULL;

	LoadShoppingData(szSectionName);
}

void CPlayerPed::LoadShoppingData(char* szSectionName)
{
	DWORD dwFunc = 0x49BBE0;
	_asm {
		push szSectionName
		call dwFunc
		add esp, 4
	}
}

VECTOR* CPlayerPed::GetBonePosition(int iBoneID, VECTOR* pPos)
{
	if (m_pPed && m_pPed->entity.vtable != ADDR_PLACEABLE_VTBL) return 0;

	DWORD dwThis = (DWORD)m_pPed;
	VECTOR* pReturn;
	_asm
	{
		push 0
		push iBoneID
		push pPos
		mov ecx, m_pPed
		mov edx, 0x5E4280
		call edx
		mov pReturn, eax
	}
	return pReturn;
}
