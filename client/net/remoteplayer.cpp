//----------------------------------------------------------
//
// SA:MP Multiplayer Modification For GTA:SA
// Copyright 2004-2005 SA:MP team
//
// Version: $Id: remoteplayer.cpp,v 1.53 2006/05/21 11:20:49 kyeman Exp $
//
//----------------------------------------------------------

#include "../main.h"
#include "../game/util.h"
#include <math.h>

using namespace RakNet;

#define IS_TARGETING(x) (x & 128)

float fOnFootCorrectionMultiplier = 0.06f;
float fInCarCorrectionMultiplier = 0.06f;
float fInaccuracyFactorMultiplier = 5.0f;
float fInCarInaccuracyFactorMultiplier = 1.0f;

//----------------------------------------------------

CRemotePlayer::CRemotePlayer()
{
	m_byteUpdateFromNetwork = UPDATE_TYPE_NONE;
	m_byteState = PLAYER_STATE_NONE;
	m_bytePlayerID = INVALID_PLAYER_ID;
	m_pPlayerPed = NULL;
	m_byteUpdateFromNetwork = UPDATE_TYPE_NONE;
	m_VehicleID = 0;
	m_dwWaitForEntryExitAnims = GetTickCount();
	ResetAllSyncAttributes();
	m_byteTeam = NO_TEAM;
	m_bVisible = true;
	m_bShowNameTag = true;
	m_iVirtualWorld = 0;
	m_dwLastHeadUpdate = GetTickCount();
	m_dwStreamUpdate = 0;
	m_iScore = 0;
	m_usPing = 0;
	m_szName[0] = '\0';
}

//----------------------------------------------------

CRemotePlayer::~CRemotePlayer()
{
	if(m_pPlayerPed) {
		m_pPlayerPed->Destroy();
		delete m_pPlayerPed;
		m_pPlayerPed = NULL;
	}
}

void CRemotePlayer::SetName(const char* szName)
{
	strncpy_s(m_szName, szName, MAX_PLAYER_NAME);
}

const char* CRemotePlayer::GetName()
{
	return m_szName;
}

//----------------------------------------------------

void CRemotePlayer::Process(int iLocalWorld)
{
	CPlayerPool *pPool = pNetGame->GetPlayerPool();
	CVehiclePool *pVehiclePool = pNetGame->GetVehiclePool();
	CLocalPlayer *pLocalPlayer = pPool->GetLocalPlayer();
	MATRIX4X4 matVehicle;
	VECTOR vecMoveSpeed;

	if(IsActive())
	{
		if(m_pPlayerPed)
		{
			// Stream player markers based on the server script
			// settings for the chat radius

			if (m_iVirtualWorld != iLocalWorld || m_byteState == PLAYER_STATE_SPECTATING) m_bVisible = false;
			else m_bVisible = true;			

			m_pPlayerPed->ProcessMarkers(
				pNetGame->m_bLimitGlobalMarkerRadius,
				pNetGame->m_fGlobalMarkerRadius,
				m_bVisible);

			HandlePlayerPedStreaming();

			if(GetTickCount() > m_dwWaitForEntryExitAnims) {
				HandleVehicleEntryExit();
			}

			// ---- ONFOOT NETWORK PROCESSING ----
			if( GetState() == PLAYER_STATE_ONFOOT &&
				m_byteUpdateFromNetwork == UPDATE_TYPE_ONFOOT )
			{	
				if ( !IsSurfingOrTurretMode() )
				{
					// If the user hasn't sent these, they're obviously not on the car, so we just sync
					// their normal movement/speed/etc.
					UpdateOnFootPositionAndSpeed(&m_ofSync.vecPos,&m_ofSync.vecMoveSpeed);
					UpdateOnfootTargetPosition();
				}

				// UPDATE ROTATION
				m_pPlayerPed->SetTargetRotation(m_ofSync.fRotation);
		
				// UPDATE CURRENT WEAPON
				if(m_pPlayerPed->IsAdded() && m_pPlayerPed->GetCurrentWeapon() != m_ofSync.byteCurrentWeapon) {
					m_pPlayerPed->SetArmedWeapon(m_ofSync.byteCurrentWeapon);

					// double check
					if(m_pPlayerPed->GetCurrentWeapon() != m_ofSync.byteCurrentWeapon) {
						m_pPlayerPed->GiveWeapon(m_ofSync.byteCurrentWeapon,9999);
						m_pPlayerPed->SetArmedWeapon(m_ofSync.byteCurrentWeapon);
					}
				}

				// First person weapon action hack
				if (IS_TARGETING(m_ofSync.uiKeys)) {
					if (m_pPlayerPed->GetCurrentWeapon() == 34 ||
						m_pPlayerPed->GetCurrentWeapon() == 35 ||
						m_pPlayerPed->GetCurrentWeapon() == 36)
					{
						m_pPlayerPed->SetActionTrigger(12);
					}
				} else if (m_pPlayerPed->GetActionTrigger() == 12) {
					m_pPlayerPed->SetActionTrigger(1);
				}

				m_pCurrentVehicle = NULL;
				m_byteUpdateFromNetwork = UPDATE_TYPE_NONE;
			}

			// ---- DRIVER NETWORK PROCESSING ----
			else if(GetState() == PLAYER_STATE_DRIVER &&
				m_byteUpdateFromNetwork == UPDATE_TYPE_INCAR)
			{
				if(!m_pCurrentVehicle || !m_pCurrentVehicle->VerifyInstance()) return;

				//m_pCurrentVehicle->ProcessEngineAudio(m_pPlayerPed->m_bytePlayerNumber);
                
				// MATRIX
				DecompressNormalVector(&matVehicle.right,&m_icSync.cvecRoll);
				DecompressNormalVector(&matVehicle.up,&m_icSync.cvecDirection);

				matVehicle.pos = m_icSync.vecPos;

				// MOVE SPEED
				vecMoveSpeed = m_icSync.vecMoveSpeed;

				// Note: Train Speed and Tire Popping values are mutually exclusive, which means
				//       if one is set, the other one will be affected.
				if( m_pCurrentVehicle->GetModelIndex() == TRAIN_PASSENGER_LOCO ||
					m_pCurrentVehicle->GetModelIndex() == TRAIN_FREIGHT_LOCO ||
					m_pCurrentVehicle->GetModelIndex() == TRAIN_TRAM) {
					// TRAIN MATRIX UPDATE
					UpdateTrainDriverMatrixAndSpeed(&matVehicle,&vecMoveSpeed, m_icSync.fTrainSpeed);
				} else {

					// GENERIC VEHICLE MATRIX UPDATE
					UpdateInCarMatrixAndSpeed(matVehicle,vecMoveSpeed);
					UpdateIncarTargetPosition();
					
					// FOR TIRE POPPING
					if (pNetGame->m_bTirePopping) {
						if (m_pCurrentVehicle->GetVehicleSubtype() == VEHICLE_SUBTYPE_BIKE) {
							m_pCurrentVehicle->SetWheelPopped(0, m_icSync.byteTires[0]);
							m_pCurrentVehicle->SetWheelPopped(1, m_icSync.byteTires[1]);
						} 
						else if ( m_pCurrentVehicle->GetVehicleSubtype() == VEHICLE_SUBTYPE_CAR ) {
							m_pCurrentVehicle->SetWheelPopped(0, m_icSync.byteTires[0]);
							m_pCurrentVehicle->SetWheelPopped(1, m_icSync.byteTires[1]);
							m_pCurrentVehicle->SetWheelPopped(2, m_icSync.byteTires[2]);
							m_pCurrentVehicle->SetWheelPopped(3, m_icSync.byteTires[3]);
						}
					}
				}

				// HYDRA THRUSTERS
				if(m_pCurrentVehicle->GetModelIndex() == HYDRA) {
					m_pCurrentVehicle->SetHydraThrusters(m_icSync.dwHydraThrustAngle);
				}
				// Some other SPECIAL sync stuff (these can be optimized for specific vehicles, someday!)
				m_pCurrentVehicle->SetSirenOn(m_icSync.byteSirenOn);
				m_pCurrentVehicle->SetLandingGearState((eLandingGearState)m_icSync.byteLandingGearState);
		
				// VEHICLE HEALTH
				m_pCurrentVehicle->SetHealth(m_icSync.fCarHealth);

				// TRAILER AUTOMATIC ATTACHMENT AS THEY MOVE INTO IT
				if (m_pCurrentVehicle->GetDistanceFromLocalPlayerPed() < LOCKING_DISTANCE)
				{
					if (m_icSync.TrailerID && m_icSync.TrailerID < MAX_VEHICLES)
					{
						CVehicle* pRealTrailer = m_pCurrentVehicle->GetTrailer();
						CVehicle *pTrailerVehicle = pVehiclePool->GetAt(m_icSync.TrailerID);
						if (!pRealTrailer) {
							if (pTrailerVehicle) {
								m_pCurrentVehicle->SetTrailer(pTrailerVehicle);
								m_pCurrentVehicle->AttachTrailer();
							}
						}
					} 
					else
					{
						if (m_pCurrentVehicle->GetTrailer()) {
							m_pCurrentVehicle->DetachTrailer();
							m_pCurrentVehicle->SetTrailer(NULL);
						}
					}
				}
				else
				{
					m_pCurrentVehicle->SetTrailer(NULL);
				}

				// UPDATE CURRENT WEAPON (FOR DRIVER!)
				if( m_pPlayerPed->IsAdded() &&
					m_pPlayerPed->GetCurrentWeapon() != m_icSync.byteCurrentWeapon ) {
					m_pPlayerPed->SetArmedWeapon(m_icSync.byteCurrentWeapon);
					if(m_pPlayerPed->GetCurrentWeapon() != m_icSync.byteCurrentWeapon) {
						m_pPlayerPed->GiveWeapon(m_icSync.byteCurrentWeapon,9999);
						m_pPlayerPed->SetArmedWeapon(m_icSync.byteCurrentWeapon);
					}
				}
				m_byteUpdateFromNetwork = UPDATE_TYPE_NONE;
			}

			// ---- PASSENGER NETWORK PROCESSING ----
			else if(GetState() == PLAYER_STATE_PASSENGER && 
				m_byteUpdateFromNetwork == UPDATE_TYPE_PASSENGER)
			{
				if(!m_pCurrentVehicle || !m_pCurrentVehicle->VerifyInstance()) return;

				// UPDATE CURRENT WEAPON
				if( m_pPlayerPed->IsAdded() &&
					m_pPlayerPed->GetCurrentWeapon() != m_psSync.byteCurrentWeapon ) {
					m_pPlayerPed->SetArmedWeapon(m_psSync.byteCurrentWeapon);
					if(m_pPlayerPed->GetCurrentWeapon() != m_psSync.byteCurrentWeapon) {
						m_pPlayerPed->GiveWeapon(m_psSync.byteCurrentWeapon,9999);
						m_pPlayerPed->SetArmedWeapon(m_psSync.byteCurrentWeapon);
					}
				}
				// FOR INITIALISING PASSENGER DRIVEBY
				if(!m_bPassengerDriveByMode && m_pPlayerPed->GetActionTrigger() == ACTION_INCAR) {
					if(m_psSync.byteDriveBy) {
						if(m_pPlayerPed->IsAdded()) {
							if(m_pPlayerPed->StartPassengerDriveByMode()) {
								m_bPassengerDriveByMode = true;
							}
						}
					}
				}
				m_byteUpdateFromNetwork = UPDATE_TYPE_NONE;
			}

			// ------ PROCESSED FOR ALL FRAMES ----- 
			if(GetState() == PLAYER_STATE_ONFOOT) {
				m_bPassengerDriveByMode = false;
				ProcessSpecialActions(m_ofSync.byteSpecialAction);
				//m_pPlayerPed->SetMoveSpeedVector(m_ofSync.vecMoveSpeed);
				m_pPlayerPed->SetKeys(m_ofSync.uiKeys,m_ofSync.lrAnalog,m_ofSync.udAnalog);

				if(m_pPlayerPed->IsInJetpackMode()) {
					m_pPlayerPed->ForceTargetRotation(m_ofSync.fRotation);
				} else {
					m_pPlayerPed->SetTargetRotation(m_ofSync.fRotation);
				}

				if(IsSurfingOrTurretMode()) {
					UpdateSurfing();
					m_pPlayerPed->SetGravityProcessing(0);
					m_pPlayerPed->SetCollisionChecking(0);
				} else {
					m_pPlayerPed->SetGravityProcessing(1);
					m_pPlayerPed->SetCollisionChecking(1);
				}
			}
			else if(GetState() == PLAYER_STATE_DRIVER) {
				m_bPassengerDriveByMode = false;
				m_pPlayerPed->CheckVehicleParachute();
				
				// REMOVE PLAYER MODEL IN RC VEHICLE
				if(m_pCurrentVehicle && m_pCurrentVehicle->IsRCVehicle()) {
					m_pPlayerPed->Remove();
				}

				if(m_iIsInAModShop)	{
					VECTOR vec;
					m_pPlayerPed->SetKeys(0,0,0);
					if(m_pCurrentVehicle) m_pCurrentVehicle->SetMoveSpeedVector(vec);
				} else {
					m_pPlayerPed->SetKeys(m_icSync.uiKeys,m_icSync.lrAnalog,m_icSync.udAnalog);
					m_pPlayerPed->ProcessVehicleHorn();
				}
			}
			else if(GetState() == PLAYER_STATE_PASSENGER) {
				m_pPlayerPed->CheckVehicleParachute();
				m_pPlayerPed->SetKeys(m_psSync.uiKeys,0,0);
				
			}
			else {
				m_bPassengerDriveByMode = false;
				m_pPlayerPed->SetKeys(0,0,0);
			}

			if(GetState() != PLAYER_STATE_WASTED) {
				m_pPlayerPed->SetHealth(1000.0f);
				//m_pPlayerPed->SetArmour(100.0f);
			}			
		}
	}
	else { 
		// NOT ACTIVE
		if(m_pPlayerPed) {
			ResetAllSyncAttributes();
			m_pPlayerPed->Destroy();
			delete m_pPlayerPed;
			m_pPlayerPed = NULL;
		}
	}
}

//----------------------------------------------------

void CRemotePlayer::HandlePlayerPedStreaming()
{
	if(!m_pPlayerPed) return;

	if (GetState() == PLAYER_STATE_SPECTATING) {
		m_pPlayerPed->Remove();
		m_pPlayerPed->RemoveFromVehicleAndPutAt(0.0f, 0.0f, 0.0f);
		return;
	}

	// Remove or Add peds as they leave/enter a radius around the player
	if(m_pPlayerPed->IsInVehicle()) {

		CVehicle *pVehicle = pNetGame->GetVehiclePool()->GetAt(m_VehicleID);

		if(pVehicle) {
			if( m_bVisible && !pVehicle->IsRCVehicle() &&
				pVehicle->GetDistanceFromLocalPlayerPed() < LOCKING_DISTANCE ) {
				m_pPlayerPed->Add();
			} else {
				m_pPlayerPed->Remove();
			}
		}

	} else { // !InVehicle

		if(m_bVisible && m_pPlayerPed->GetDistanceFromLocalPlayerPed() < LOCKING_DISTANCE) {
			m_pPlayerPed->Add();
			//pChatWindow->AddDebugMessage("Player %u streamed in\n",m_pPlayerPed->m_bytePlayerNumber);
		} else {
			//pChatWindow->AddDebugMessage("Player %u streamed out\n",m_pPlayerPed->m_bytePlayerNumber);

			m_pPlayerPed->Remove();
		}
	}
}

//----------------------------------------------------

/*
	BUGS:
		- Player teleports on top of the vehicle when shot out of the vehicle
		- Player teleports in-out of the vehicle when enter or exits from the vehicle
	NOTE:
		- Sprint car jacking kill issue looks like its not related to this function.
		- Removing this function will cause players just simply standing outside when they actually in a vehicle
*/
void CRemotePlayer::HandleVehicleEntryExit()
{
	CVehiclePool *pVehiclePool = pNetGame->GetVehiclePool();
	MATRIX4X4 mat;

	if(!m_pPlayerPed) return;

	if(GetState() == PLAYER_STATE_ONFOOT) {
		if(m_pPlayerPed->IsInVehicle()) {
			m_pPlayerPed->GetMatrix(&mat);
			m_pPlayerPed->RemoveFromVehicleAndPutAt(mat.pos.X,mat.pos.Y,mat.pos.Z);
		}
		return;
	}

	if( GetState() == PLAYER_STATE_DRIVER || GetState() == PLAYER_STATE_PASSENGER )
	{
		if(!m_pPlayerPed->IsInVehicle()) {
			CVehicle *pVehicle = pVehiclePool->GetAt(m_VehicleID);
			if(pVehicle) {
				int iCarID = pVehiclePool->FindGtaIDFromID(m_VehicleID);
				m_pPlayerPed->PutDirectlyInVehicle(iCarID,m_byteSeatID);
			}
		}
	}
}

//----------------------------------------------------

void CRemotePlayer::ProcessSpecialActions(BYTE byteSpecialAction)
{
	if(!m_pPlayerPed || !m_pPlayerPed->IsAdded()) return;

	if(GetState() != PLAYER_STATE_ONFOOT) {
		byteSpecialAction = SPECIAL_ACTION_NONE;
		m_ofSync.byteSpecialAction = SPECIAL_ACTION_NONE;
	}

	// headsync:always
	if(GetState() == PLAYER_STATE_ONFOOT && m_pPlayerPed->IsAdded()) {
		if((GetTickCount() - m_dwLastHeadUpdate) > 500 && !pConfig->GetInt("disableheadmove")) {
            VECTOR LookAt;
			CAMERA_AIM *Aim = GameGetRemotePlayerAim(m_pPlayerPed->m_bytePlayerNumber);
            LookAt.X = Aim->pos1x + (Aim->f1x * 20.0f);
			LookAt.Y = Aim->pos1y + (Aim->f1y * 20.0f);
			LookAt.Z = Aim->pos1z + (Aim->f1z * 20.0f);
			m_pPlayerPed->ApplyCommandTask("FollowPedSA",0,2000,-1,&LookAt,0,0.1f,500,3,0);
			m_dwLastHeadUpdate = GetTickCount();
		}
	}

	// cellphone:start
	if( byteSpecialAction == SPECIAL_ACTION_USECELLPHONE &&
		!m_pPlayerPed->IsCellphoneEnabled() ) {
			//pChatWindow->AddDebugMessage("Player enabled Cell");
			m_pPlayerPed->ToggleCellphone(1);
			return;
	}

	// cellphone:stop
	if( byteSpecialAction != SPECIAL_ACTION_USECELLPHONE &&
		m_pPlayerPed->IsCellphoneEnabled() ) {
			m_pPlayerPed->ToggleCellphone(0);
			return;
	}

	// jetpack:start
	if(byteSpecialAction == SPECIAL_ACTION_USEJETPACK) {
		if(!m_pPlayerPed->IsInJetpackMode()) {
			m_pPlayerPed->StartJetpack();
			return;
		}
	}

	// jetpack:stop
	if(byteSpecialAction != SPECIAL_ACTION_USEJETPACK) {
		if(m_pPlayerPed->IsInJetpackMode()) {
			m_pPlayerPed->StopJetpack();
			return;
		}
	}

	// handsup:start
	if(byteSpecialAction == SPECIAL_ACTION_HANDSUP && !m_pPlayerPed->HasHandsUp()) {
		m_pPlayerPed->HandsUp();
	}

	// handsup:stop
	if(byteSpecialAction != SPECIAL_ACTION_HANDSUP && m_pPlayerPed->HasHandsUp()) {
		m_pPlayerPed->StopDancing(); // has the same effect
	}

	// dancing:start
	if(!m_pPlayerPed->IsDancing() && byteSpecialAction == SPECIAL_ACTION_DANCE1) {
		m_pPlayerPed->StartDancing(0);
	}
	if(!m_pPlayerPed->IsDancing() && byteSpecialAction == SPECIAL_ACTION_DANCE2) {
		m_pPlayerPed->StartDancing(1);
	}
	if(!m_pPlayerPed->IsDancing() && byteSpecialAction == SPECIAL_ACTION_DANCE3) {
		m_pPlayerPed->StartDancing(2);
	}
	if(!m_pPlayerPed->IsDancing() && byteSpecialAction == SPECIAL_ACTION_DANCE4) {
		m_pPlayerPed->StartDancing(3);
	}

	// dancing:stop
	if( m_pPlayerPed->IsDancing() && 
		(byteSpecialAction != SPECIAL_ACTION_DANCE1 &&
		byteSpecialAction != SPECIAL_ACTION_DANCE2 && 
		byteSpecialAction != SPECIAL_ACTION_DANCE3 && 
		byteSpecialAction != SPECIAL_ACTION_DANCE4) ) {
			m_pPlayerPed->StopDancing();
	}

	if(m_pPlayerPed->IsDancing()) m_pPlayerPed->ProcessDancing();

	// pissing:start
	if(!m_pPlayerPed->IsPissing() && byteSpecialAction == SPECIAL_ACTION_URINATE) {
		m_pPlayerPed->StartPissing();
	}

	// pissing:stop
	if(m_pPlayerPed->IsPissing() && byteSpecialAction != SPECIAL_ACTION_URINATE) {
		m_pPlayerPed->StopPissing();
	}

	// parachutes:we don't have any network indicators for this yet
	m_pPlayerPed->ProcessParachutes();
}

//----------------------------------------------------

bool CRemotePlayer::IsSurfingOrTurretMode()
{
	if(GetState() == PLAYER_STATE_ONFOOT) {
		if( m_ofSync.SurfVehicleId != 0 &&
			m_ofSync.SurfVehicleId != INVALID_VEHICLE_ID &&
			m_ofSync.SurfVehicleId < MAX_VEHICLES ) {
			return true;
		}
	}
	return false;
}

//----------------------------------------------------

void CRemotePlayer::UpdateSurfing()
{
	CVehiclePool* pVehiclePool = pNetGame->GetVehiclePool();
	CVehicle* pVehicle = pVehiclePool->GetAt(m_ofSync.SurfVehicleId);

	if (pVehicle) {
		MATRIX4X4 matPlayer, matVehicle;
		VECTOR vecMoveSpeed,vecTurnSpeed;

		pVehicle->GetMatrix(&matVehicle);
		pVehicle->GetMoveSpeedVector(&vecMoveSpeed);
		pVehicle->GetTurnSpeedVector(&vecTurnSpeed);
		m_pPlayerPed->GetMatrix(&matPlayer);
		
		matPlayer.pos.X = matVehicle.pos.X + m_ofSync.vecSurfOffsets.X;
		matPlayer.pos.Y = matVehicle.pos.Y + m_ofSync.vecSurfOffsets.Y;
		matPlayer.pos.Z = matVehicle.pos.Z + m_ofSync.vecSurfOffsets.Z;

		m_pPlayerPed->SetMatrix(matPlayer);
		m_pPlayerPed->SetMoveSpeedVector(vecMoveSpeed);
		//m_pPlayerPed->SetTurnSpeedVector(vecTurnSpeed);
	}
}

//----------------------------------------------------

void CRemotePlayer::UpdateOnfootTargetPosition()
{
	MATRIX4X4 matEnt;

	if(!m_pPlayerPed) return;
	m_pPlayerPed->GetMatrix(&matEnt);

	if(!m_pPlayerPed->IsAdded() || !m_pPlayerPed->IsOnGround()) {
        matEnt.pos = m_vecOnfootTargetPos;
		m_pPlayerPed->SetMatrix(matEnt);
	}

	m_vecPositionInaccuracy.X = FloatOffset(m_vecOnfootTargetPos.X,matEnt.pos.X);
	m_vecPositionInaccuracy.Y = FloatOffset(m_vecOnfootTargetPos.Y,matEnt.pos.Y);
    m_vecPositionInaccuracy.Z = FloatOffset(m_vecOnfootTargetPos.Z,matEnt.pos.Z);

     // check for an already valid position
	if( (m_vecPositionInaccuracy.X <= 0.0001f) &&
		(m_vecPositionInaccuracy.Y <= 0.0001f) &&
		(m_vecPositionInaccuracy.Z <= 0.0001f) )
	{
		// nothing to do
		return;
	}

	// Directly translate way-out position
	if( (m_vecPositionInaccuracy.X > 2.5f) ||
		(m_vecPositionInaccuracy.Y > 2.5f) ||
		(m_vecPositionInaccuracy.Z > 1.0f) ) {

		matEnt.pos = m_vecOnfootTargetPos;
		m_pPlayerPed->SetMatrix(matEnt);

		return;
	}

	// Avoid direct translation by increasing velocity
	// towards the target
	
	VECTOR vec;
	m_pPlayerPed->GetMoveSpeedVector(&vec);

	float fMultiplyAmount = fOnFootCorrectionMultiplier;

	if(!m_pPlayerPed->IsOnGround()) {
		fMultiplyAmount = fMultiplyAmount * 0.25f;
	}

	if( m_vecPositionInaccuracy.X > 0.0001f ) {
		vec.X += ((m_vecOnfootTargetPos.X - matEnt.pos.X) * fInaccuracyFactorMultiplier) * fMultiplyAmount;
	}
	if( m_vecPositionInaccuracy.Y > 0.0001f ) {
		vec.Y += ((m_vecOnfootTargetPos.Y - matEnt.pos.Y) * fInaccuracyFactorMultiplier) * fMultiplyAmount;
	}
	if( m_vecPositionInaccuracy.Z > 0.0001f ) {
		vec.Z += ((m_vecOnfootTargetPos.Z - matEnt.pos.Z) * fInaccuracyFactorMultiplier) * 0.025f;
	}

	m_pPlayerPed->SetMoveSpeedVector(vec);
	m_pPlayerPed->ApplyMoveSpeed();
}

//----------------------------------------------------

void CRemotePlayer::UpdateOnFootPositionAndSpeed(VECTOR * vecPos, VECTOR * vecMove)
{
	/* lag comp
	float fLowestPing = (float)pNetGame->GetRakClient()->GetLastPing();
	float fEstimateFramesPassed = fLowestPing * 0.1f;

	if(fEstimateFramesPassed > 1.0f) {
		m_vecOnfootTargetPos.X += (vecMove->X * fEstimateFramesPassed);
		m_vecOnfootTargetPos.Y += (vecMove->Y * fEstimateFramesPassed);
		//m_vecOnfootTargetPos.Z += (vecMove->Z * fEstimateFramesPassed);
	}*/
    
    m_vecOnfootTargetPos = vecPos;
    m_vecOnfootTargetSpeed = vecMove;
	m_pPlayerPed->SetMoveSpeedVector(m_vecOnfootTargetSpeed);
}

//----------------------------------------------------

void CRemotePlayer::UpdateIncarTargetPosition()
{
	MATRIX4X4 matEnt;
	VECTOR vec;

	if(!m_pCurrentVehicle) return;
	m_pCurrentVehicle->GetMatrix(&matEnt);

	if(!m_pCurrentVehicle->IsAdded()) {
		matEnt.pos = m_vecIncarTargetPos;
		m_pCurrentVehicle->SetMatrix(matEnt);
        //m_pCurrentVehicle->SetMoveSpeedVector(vec);
		return;
	}

  	// Don't apply any updates from vehicle mod shops
	if(m_iIsInAModShop)
	{
		m_pPlayerPed->SetKeys(0,0,0);
		m_pCurrentVehicle->SetMoveSpeedVector(vec);
		m_icSync.udAnalog = 0;
		m_icSync.lrAnalog = 0;
		m_icSync.uiKeys = 0;
		m_icSync.vecMoveSpeed = 0.0f;
		return;
	}
	
	// BUG: When player in a vehicle, the vehicle will jump/shaking around, sliding updside down,
	// or the vehicle slowly shifting in some direction when player just idling
	m_vecPositionInaccuracy.X = FloatOffset(m_vecIncarTargetPos.X,matEnt.pos.X);
	m_vecPositionInaccuracy.Y = FloatOffset(m_vecIncarTargetPos.Y,matEnt.pos.Y);
    m_vecPositionInaccuracy.Z = FloatOffset(m_vecIncarTargetPos.Z,matEnt.pos.Z);

    // check for an already valid position
	if( (m_vecPositionInaccuracy.X <= 0.002f) &&
		(m_vecPositionInaccuracy.Y <= 0.002f) &&
		(m_vecPositionInaccuracy.Z <= 0.002f) )
	{
		// nothing to do
		return;
	}

	float fTestZInaccuracy = 0.5f;

	if( m_pCurrentVehicle->GetVehicleSubtype() == VEHICLE_SUBTYPE_BOAT ||
		m_pCurrentVehicle->GetVehicleSubtype() == VEHICLE_SUBTYPE_PLANE ||
		m_pCurrentVehicle->GetVehicleSubtype() == VEHICLE_SUBTYPE_HELI )
	{
		fTestZInaccuracy = 5.0f;
	}

	// Directly translate way-out position
	if( (m_vecPositionInaccuracy.X > 8.0f) ||
		(m_vecPositionInaccuracy.Y > 8.0f) ||
		(m_vecPositionInaccuracy.Z > fTestZInaccuracy) )
	{
		matEnt.pos = m_vecIncarTargetPos;
		m_pCurrentVehicle->SetMatrix(matEnt);
        m_pCurrentVehicle->SetMoveSpeedVector(m_vecIncarTargetSpeed);

		return;
	}

	// Avoid direct translation by increasing velocity
	// towards the target
	m_pCurrentVehicle->GetMoveSpeedVector(&vec);

	if( m_vecPositionInaccuracy.X > 0.002f ) {
		vec.X += (m_vecIncarTargetPos.X - matEnt.pos.X) * fInCarCorrectionMultiplier;
	}
	if( m_vecPositionInaccuracy.Y > 0.002f ) {
		vec.Y += (m_vecIncarTargetPos.Y - matEnt.pos.Y) * fInCarCorrectionMultiplier;
	}
	if( m_vecPositionInaccuracy.Z > 0.002f ) {
		vec.Z += (m_vecIncarTargetPos.Z - matEnt.pos.Z) * fInCarCorrectionMultiplier;
	}

	m_pCurrentVehicle->SetMoveSpeedVector(vec);
}

//----------------------------------------------------

void CRemotePlayer::UpdateInCarMatrixAndSpeed(MATRIX4X4 mat, VECTOR vecMove)
{	
	/*
	float fLowestPing = (float)pNetGame->GetRakClient()->GetLowestPing();
	float fEstimateFramesPassed = fLowestPing * 0.025f;

	if(fEstimateFramesPassed > 1.0f) {
		m_vecIncarTargetPos.X += (vecMove.X * fEstimateFramesPassed);
		m_vecIncarTargetPos.Y += (vecMove.Y * fEstimateFramesPassed);
		m_vecIncarTargetPos.Z += (vecMove.Z * fEstimateFramesPassed);
	}*/

	MATRIX4X4 matEnt;
	m_pCurrentVehicle->GetMatrix(&matEnt);

	matEnt.right = mat.right;
	matEnt.up = mat.up;
	m_vecIncarTargetPos = mat.pos;
	m_vecIncarTargetSpeed = vecMove;

	m_pCurrentVehicle->SetMatrix(matEnt);
	m_pCurrentVehicle->SetMoveSpeedVector(vecMove);
}

//----------------------------------------------------

void CRemotePlayer::UpdateTrainDriverMatrixAndSpeed(MATRIX4X4 *matWorld,VECTOR *vecMoveSpeed, float fTrainSpeed)
{
	MATRIX4X4 matVehicle;
	VECTOR vecInternalMoveSpeed;
	bool bTeleport=false;
	float fDif;

	if(!m_pPlayerPed || !m_pCurrentVehicle) return;

	m_pCurrentVehicle->GetMatrix(&matVehicle);

	if(matWorld->pos.X >= matVehicle.pos.X) {
		fDif = matWorld->pos.X - matVehicle.pos.X;
	} else {
		fDif = matVehicle.pos.X - matWorld->pos.X;
	}
	if(fDif > 10.0f) bTeleport=true;

	if(matWorld->pos.Y >= matVehicle.pos.Y) {
		fDif = matWorld->pos.Y - matVehicle.pos.Y;
	} else {
		fDif = matVehicle.pos.Y - matWorld->pos.Y;
	}
	if(fDif > 10.0f) bTeleport=true;

	if(bTeleport) m_pCurrentVehicle->TeleportTo(matWorld->pos.X,matWorld->pos.Y,matWorld->pos.Z);
	
	m_pCurrentVehicle->GetMoveSpeedVector(&vecInternalMoveSpeed);
	vecInternalMoveSpeed = vecMoveSpeed;
	m_pCurrentVehicle->SetMoveSpeedVector(vecInternalMoveSpeed);
	m_pCurrentVehicle->SetTrainSpeed(fTrainSpeed);
}

//----------------------------------------------------

void CRemotePlayer::StoreOnFootFullSyncData(ONFOOT_SYNC_DATA *pofSync)
{
	if(GetTickCount() < m_dwWaitForEntryExitAnims) return;

	m_pCurrentVehicle = NULL;
	memcpy(&m_ofSync,pofSync,sizeof(ONFOOT_SYNC_DATA));
	m_fReportedHealth = (float)pofSync->byteHealth;
	m_fReportedArmour = (float)pofSync->byteArmour;
	m_byteUpdateFromNetwork = UPDATE_TYPE_ONFOOT;
	
	SetState(PLAYER_STATE_ONFOOT);
}

//----------------------------------------------------

void CRemotePlayer::StoreInCarFullSyncData(INCAR_SYNC_DATA *picSync)
{	
	if(GetTickCount() < m_dwWaitForEntryExitAnims) return;
	memcpy(&m_icSync,picSync,sizeof(INCAR_SYNC_DATA));

	m_VehicleID = picSync->VehicleID;
	m_byteSeatID = 0;
	m_pCurrentVehicle = pNetGame->GetVehiclePool()->GetAt(m_VehicleID);

	m_fReportedHealth = (float)picSync->bytePlayerHealth;
	m_fReportedArmour =	(float)picSync->bytePlayerArmour;
	m_byteUpdateFromNetwork = UPDATE_TYPE_INCAR;
	
	SetState(PLAYER_STATE_DRIVER);
}

//----------------------------------------------------

void CRemotePlayer::StorePassengerFullSyncData(PASSENGER_SYNC_DATA *ppsSync)
{
	if(GetTickCount() < m_dwWaitForEntryExitAnims) return;
	memcpy(&m_psSync,ppsSync,sizeof(PASSENGER_SYNC_DATA));

	m_VehicleID = ppsSync->VehicleID;
	m_byteSeatID = ppsSync->byteSeatFlags & 127;
	m_pCurrentVehicle = pNetGame->GetVehiclePool()->GetAt(m_VehicleID);

	m_fReportedHealth = (float)ppsSync->bytePlayerHealth;
	m_fReportedArmour = (float)ppsSync->bytePlayerArmour;
	m_byteUpdateFromNetwork = UPDATE_TYPE_PASSENGER;
	
	SetState(PLAYER_STATE_PASSENGER);
}

//----------------------------------------------------

void CRemotePlayer::UpdateAimFromSyncData(AIM_SYNC_DATA *paimSync)
{
	if(!m_pPlayerPed) return;
	m_pPlayerPed->SetCameraMode(paimSync->byteCamMode);

	CAMERA_AIM Aim;

	Aim.f1x = paimSync->vecAimf1.X;
	Aim.f1y = paimSync->vecAimf1.Y;
	Aim.f1z = paimSync->vecAimf1.Z;
	Aim.f2x = paimSync->vecAimf2.X;
	Aim.f2y = paimSync->vecAimf2.Y;
	Aim.f2z = paimSync->vecAimf2.Z;
	Aim.pos1x = paimSync->vecAimPos.X;
	Aim.pos1y = paimSync->vecAimPos.Y;
	Aim.pos1z = paimSync->vecAimPos.Z;
	Aim.pos2x = paimSync->vecAimPos.X;
	Aim.pos2y = paimSync->vecAimPos.Y;
	Aim.pos2z = paimSync->vecAimPos.Z;

	m_pPlayerPed->SetCurrentAim(&Aim);
	m_pPlayerPed->SetAimZ(paimSync->fAimZ);

	float fExtZoom = (float)(paimSync->byteCamExtZoom)/63.0f;
	m_pPlayerPed->SetCameraExtendedZoom(fExtZoom);

	WEAPON_SLOT_TYPE* pwstWeapon = m_pPlayerPed->GetCurrentWeaponSlot();
	if (paimSync->byteWeaponState == WS_RELOADING)
		pwstWeapon->dwState = 2;		// Reloading
	else
		if (paimSync->byteWeaponState != WS_MORE_BULLETS) 
			pwstWeapon->dwAmmoInClip = (DWORD)paimSync->byteWeaponState;
		else
			if (pwstWeapon->dwAmmoInClip < 2)
				pwstWeapon->dwAmmoInClip = 2;


}

//----------------------------------------------------

void CRemotePlayer::StoreTrailerFullSyncData(TRAILER_SYNC_DATA* trSync)
{
	VEHICLEID TrailerID = m_icSync.TrailerID;
	if (!TrailerID) return;
	
	CVehicle *pVehicle = pNetGame->GetVehiclePool()->GetAt(TrailerID);
	if(pVehicle) 
	{
		MATRIX4X4 matWorld;

		memcpy(&matWorld.pos, &trSync->vecPos, sizeof(VECTOR));
		DecompressNormalVector(&matWorld.up, &trSync->cvecDirection);
		DecompressNormalVector(&matWorld.right, &trSync->cvecRoll);
		
		pVehicle->SetMatrix(matWorld);
		pVehicle->SetMoveSpeedVector(trSync->vecMoveSpeed);
	}
}

//----------------------------------------------------

bool CRemotePlayer::Spawn(BYTE byteTeam, int iSkin, VECTOR * vecPos, float fRotation, DWORD dwColor)
{
	if(!pGame->IsGameLoaded()) return false;

	if(m_pPlayerPed != NULL) {
		m_pPlayerPed->Destroy();
		delete m_pPlayerPed;
	}

	CPlayerPed *pPlayer = pGame->NewPlayer(m_bytePlayerID+2,iSkin,vecPos->X,
		vecPos->Y,vecPos->Z,fRotation,pNetGame->m_bShowPlayerMarkers); // the +2 is because we can't use 0 or 1, even though the server does.
	
	if(pPlayer) 
	{
		if(dwColor!=0) SetPlayerColor(dwColor);

		m_pPlayerPed = pPlayer;
		m_fReportedHealth = 100.0f;
		pPlayer->SetKeys(0,0,0);
		ResetAllSyncAttributes();

		SetState(PLAYER_STATE_SPAWNED);

		return true;
	}

	SetState(PLAYER_STATE_NONE);
	return false;
}

//----------------------------------------------------

void CRemotePlayer::ResetAllSyncAttributes()
{	
	m_VehicleID = 0;
	memset(&m_ofSync,0,sizeof(ONFOOT_SYNC_DATA));
	memset(&m_icSync,0,sizeof(INCAR_SYNC_DATA));
	memset(&m_psSync,0,sizeof(PASSENGER_SYNC_DATA));
	memset(&m_aimSync,0,sizeof(AIM_SYNC_DATA));
	m_fReportedHealth = 0.0f;
	m_fReportedArmour = 0.0f;
	m_pCurrentVehicle = NULL;
	m_byteSeatID = 0;
	m_bPassengerDriveByMode = false;
	m_iIsInAModShop = 0;
	m_dwWaitForEntryExitAnims = GetTickCount();
}

//----------------------------------------------------

void CRemotePlayer::HandleDeath()
{
	CLocalPlayer *pLocalPlayer = pNetGame->GetPlayerPool()->GetLocalPlayer();
	if (pLocalPlayer->IsSpectating() && pLocalPlayer->m_SpectateID == m_bytePlayerID) {
			//pLocalPlayer->ToggleSpectating(FALSE);
	}
	if(m_pPlayerPed) {
		m_pPlayerPed->SetKeys(0,0,0);
		m_pPlayerPed->SetDead();
	}
	SetState(PLAYER_STATE_WASTED);
	ResetAllSyncAttributes();
}

//----------------------------------------------------

void CRemotePlayer::Say(unsigned char *szText)
{
	pChatWindow->AddChatMessage(m_szName,GetPlayerColorAsARGB(),(char*)szText);
}

//----------------------------------------------------

float CRemotePlayer::GetDistanceFromRemotePlayer(CRemotePlayer *pFromPlayer)
{
	MATRIX4X4	matFromPlayer;
	MATRIX4X4	matThisPlayer;

	float		fSX,fSY,fSZ;

	if(!m_pPlayerPed) return 10000.0f; // very far away
	
	m_pPlayerPed->GetMatrix(&matThisPlayer);
	pFromPlayer->GetPlayerPed()->GetMatrix(&matFromPlayer);
	
	fSX = (matThisPlayer.pos.X - matFromPlayer.pos.X) * (matThisPlayer.pos.X - matFromPlayer.pos.X);
	fSY = (matThisPlayer.pos.Y - matFromPlayer.pos.Y) * (matThisPlayer.pos.Y - matFromPlayer.pos.Y);
	fSZ = (matThisPlayer.pos.Z - matFromPlayer.pos.Z) * (matThisPlayer.pos.Z - matFromPlayer.pos.Z);
		
	return sqrtf(fSX + fSY + fSZ);	
}

//----------------------------------------------------

float CRemotePlayer::GetDistanceFromLocalPlayer()
{
	if(!m_pPlayerPed) return 10000.0f; // very far away

	if(GetState() == PLAYER_STATE_DRIVER && m_pCurrentVehicle && m_pPlayerPed->IsInVehicle()) {
		return m_pCurrentVehicle->GetDistanceFromLocalPlayerPed();
	} else {
		return m_pPlayerPed->GetDistanceFromLocalPlayerPed();
	}
}

//----------------------------------------------------

void CRemotePlayer::SetPlayerColor(DWORD dwColor)
{
	SetRadarColor(m_bytePlayerID,dwColor);	
}

//----------------------------------------------------

DWORD CRemotePlayer::GetPlayerColorAsRGBA()
{
	return TranslateColorCodeToRGBA(m_bytePlayerID);
}

//----------------------------------------------------

DWORD CRemotePlayer::GetPlayerColorAsARGB()
{
	return (TranslateColorCodeToRGBA(m_bytePlayerID) >> 8) | 0xFF000000;	
}

//----------------------------------------------------

void CRemotePlayer::EnterVehicle(VEHICLEID VehicleID, bool bPassenger)
{
	CVehiclePool *pVehiclePool = pNetGame->GetVehiclePool();
	CVehicle *pVehicle = pVehiclePool->GetAt(VehicleID);

	if(m_pPlayerPed && pVehicle && !m_pPlayerPed->IsInVehicle()) {

		if(bPassenger) {
			SetState(PLAYER_STATE_ENTER_VEHICLE_PASSENGER);
		} else {
			SetState(PLAYER_STATE_ENTER_VEHICLE_DRIVER);
		}
		
		m_pPlayerPed->SetKeys(0,0,0);

		if(m_bVisible && m_pPlayerPed->IsAdded() && pVehicle->IsAdded()) {
			m_pPlayerPed->EnterVehicle(pVehiclePool->FindGtaIDFromID(VehicleID),bPassenger);
		}
	}
	m_dwWaitForEntryExitAnims = GetTickCount() + 1500;
}

//----------------------------------------------------

void CRemotePlayer::ExitVehicle()
{
	if(m_pPlayerPed && m_pPlayerPed->IsInVehicle()) {
		SetState(PLAYER_STATE_EXIT_VEHICLE);
		m_pPlayerPed->SetKeys(0,0,0);
		if(m_bVisible && m_pPlayerPed->GetDistanceFromLocalPlayerPed() < 200.0f) {
			m_pPlayerPed->ExitCurrentVehicle();
		}
	}
	m_dwWaitForEntryExitAnims = GetTickCount() + 1500;
}

//----------------------------------------------------
// callback. called when this player's state changes.

void CRemotePlayer::StateChange(BYTE byteNewState, BYTE byteOldState)
{
	if(byteNewState == PLAYER_STATE_DRIVER && byteOldState == PLAYER_STATE_ONFOOT)
	{
		// If their new vehicle is the one the local player
		// is driving, we'll have to kick the local player out
		CPlayerPed *pLocalPlayerPed = pGame->FindPlayerPed();
		VEHICLEID LocalVehicle=0xFFFF;
		MATRIX4X4 mat;

		if(pLocalPlayerPed && pLocalPlayerPed->IsInVehicle() && !pLocalPlayerPed->IsAPassenger())
		{
			LocalVehicle = pNetGame->GetVehiclePool()->FindIDFromGtaPtr(pLocalPlayerPed->GetGtaVehicle());
			if(LocalVehicle == m_VehicleID) {
				pLocalPlayerPed->GetMatrix(&mat);
				pLocalPlayerPed->RemoveFromVehicleAndPutAt(mat.pos.X,mat.pos.Y,mat.pos.Z + 1.0f);	
				pGame->DisplayGameText("~r~Car Jacked~w~!",1000,5);
			}
		}
	}	
}

//----------------------------------------------------

void CRemotePlayer::ForceOutOfCurrentVehicle()
{    
	if(m_pPlayerPed && m_pPlayerPed->IsInVehicle()) {
		MATRIX4X4 mat;
		m_pPlayerPed->GetMatrix(&mat);
		m_pPlayerPed->RemoveFromVehicleAndPutAt(mat.pos.X,mat.pos.Y,mat.pos.Z);
		m_pCurrentVehicle = NULL;
	}
}

//----------------------------------------------------