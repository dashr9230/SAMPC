//----------------------------------------------------------
//
// SA:MP Multiplayer Modification For GTA:SA
// Copyright 2004-2005 SA:MP team
//
// Version: $Id: localplayer.cpp,v 1.78 2006/05/21 11:20:49 kyeman Exp $
//
//----------------------------------------------------------

#include "../main.h"
#include "../game/util.h"
#include "../game/keystuff.h"

using namespace RakNet;

#define IS_TARGETING(x) (x & 128)
#define IS_FIRING(x) (x & 4)

// SEND RATE TICKS
#define NETMODE_IDLE_ONFOOT_SENDRATE	100
#define NETMODE_NORMAL_ONFOOT_SENDRATE	40
#define NETMODE_IDLE_INCAR_SENDRATE		100
#define NETMODE_NORMAL_INCAR_SENDRATE	40

#define NETMODE_HEADSYNC_SENDRATE		1000
#define NETMODE_AIM_SENDRATE			100
#define NETMODE_FIRING_SENDRATE			40

#define LANMODE_IDLE_ONFOOT_SENDRATE	20
#define LANMODE_NORMAL_ONFOOT_SENDRATE	15
#define LANMODE_IDLE_INCAR_SENDRATE		30
#define LANMODE_NORMAL_INCAR_SENDRATE	15

#define NETMODE_SEND_MULTIPLIER			2

#define STATS_UPDATE_TICKS 1000 // 1 second

bool bFirstSpawn = true;

static DWORD dwEnterVehTimeElasped = -1;

extern int iTimesDataModified;

//----------------------------------------------------------

CLocalPlayer::CLocalPlayer()
{
	m_bHasSpawnInfo = false;
	m_pPlayerPed = pGame->FindPlayerPed();
	m_bIsActive = false;
	m_bIsWasted = false;
	m_ulThisSyncFrame = 0;
	m_ulLastSyncFrame = 0;
	m_wLastKeys = 0;
	m_iDisplayZoneTick = 0;
	m_dwLastSendTick = GetTickCount();
	m_dwLastSendSpecTick = GetTickCount();
	m_dwLastAimSendTick = m_dwLastSendTick;
	m_dwLastStatsUpdateTick = m_dwLastSendTick;
	m_bWantsAnotherClass = false;
	m_bWaitingForSpawnRequestReply = false;
	m_bControlsVisible = false;
	m_iSelectedClass = 0;
	m_iVirtualWorld = 0;
	m_dwLastSpawnSelectionTick = GetTickCount();
	m_dwLastHeadUpdate = GetTickCount();
	m_bInRCMode = false;
	m_sNormalOnfootRate = NETMODE_NORMAL_ONFOOT_SENDRATE;
	m_sNormalIncarRate = NETMODE_NORMAL_INCAR_SENDRATE;
	m_iScore = 0;
	m_usPing = 0;
	m_szName[0] = '\0';

	m_iVehicleDamageStatus.MonitoredVehicleID = INVALID_VEHICLE_ID;
	m_iVehicleDamageStatus.iPanels = 0;
	m_iVehicleDamageStatus.iDoors = 0;
	m_iVehicleDamageStatus.ucLights = 0;
	m_iVehicleDamageStatus.ucWheels = 0;

	m_bIsSpectating = false;
	m_byteSpectateType = SPECTATE_TYPE_NONE;
	m_SpectateID = 0xFFFFFFFF;
	ResetAllSyncAttributes();
		
	BYTE i;
	for (i = 0; i < 13; i++)
	{
		m_byteLastWeapon[i] = 0;
		m_dwLastAmmo[i] = 0;
	}
	m_byteTeam = NO_TEAM;
	m_bAllowedClass = false;
}

//----------------------------------------------------------

void CLocalPlayer::SetName(const char* szName)
{
	strncpy_s(m_szName, szName, MAX_PLAYER_NAME);
}

const char* CLocalPlayer::GetName()
{
	return m_szName;
}

bool CLocalPlayer::DestroyPlayer()
{
	return true;
}

//----------------------------------------------------------

void CLocalPlayer::ResetAllSyncAttributes()
{
	m_byteCurInterior = 0;
	m_LastVehicle = 0xFFFF;
	m_bInRCMode = false;
}

//----------------------------------------------------------

bool CLocalPlayer::Process()
{
	DWORD dwThisTick;

	if(m_bIsActive && (NULL != m_pPlayerPed))
	{
		// ACTIVE LOCAL PLAYER

		/*
		if(iTimesDataModified > 10)
		{
			FORCE_EXIT(0x3);
		}*/
		
		// HANDLE I'M A DEAD LOCAL PLAYER PED
		if (!m_bIsWasted && m_pPlayerPed->GetActionTrigger() == ACTION_DEATH || m_pPlayerPed->IsDead()) {
			// DEAD
			ToggleSpectating(false); // Player shouldn't die while spectating, but scripts may mess with that
			
			if(m_pPlayerPed->IsDancing()) {
				m_pPlayerPed->StopDancing(); // there's no need to dance when you're dead
			}

			if(m_pPlayerPed->IsCellphoneEnabled()) {
				m_pPlayerPed->ToggleCellphone(0);
			}

			if(m_pPlayerPed->IsPissing()) {
				m_pPlayerPed->StopPissing();
			}

			// A hack for reseting the animations/tasks
			m_pPlayerPed->TogglePlayerControllable(1);

			if(m_bInRCMode) {
				m_bInRCMode = false;
				m_pPlayerPed->Add();
			}

			if (m_pPlayerPed->IsInVehicle() && !m_pPlayerPed->IsAPassenger())
			{
				SendInCarFullSyncData(); // One last time - for explosions
				m_LastVehicle = pNetGame->GetVehiclePool()->FindIDFromGtaPtr(m_pPlayerPed->GetGtaVehicle());
			}			

			m_pPlayerPed->ExtinguishFire();

			SendWastedNotification();

			m_bIsActive = false;
			m_bIsWasted = true;

			// Disable zone names till they respawn (looks silly in request spawn)
			pGame->EnableZoneNames(0);

			return true;
		}

		// HANDLE DANCING LOCAL PED
		if(m_pPlayerPed->IsDancing()) {
			pGame->DisplayHud(false);
			m_pPlayerPed->ProcessDancing();
			if(GameGetInternalKeys()->wKeys1[17]) m_pPlayerPed->StopDancing();
		}

		// HANDLE I GOT MY HANDS UP BUT DON'T WANT TO ANYMORE
		if(m_pPlayerPed->HasHandsUp() && GameGetInternalKeys()->wKeys1[17]) {
			m_pPlayerPed->TogglePlayerControllable(1);
		}

		// HANDLE I'M TAKING A PISS AND I'M DONE
		if(m_pPlayerPed->IsPissing() && (GameGetInternalKeys()->wKeys1[17] || m_pPlayerPed->IsInVehicle())) {
			m_pPlayerPed->StopPissing();
		}

		if(m_pPlayerPed->IsInVehicle() && m_pPlayerPed->IsDancing()) 
			m_pPlayerPed->StopDancing(); // can't dance in vehicle

		dwThisTick = GetTickCount();

		if ( dwEnterVehTimeElasped != -1 && 
			(dwThisTick - dwEnterVehTimeElasped) > 5000 &&
			!m_pPlayerPed->IsInVehicle() )
		{
			pGame->GetCamera()->SetBehindPlayer();
			dwEnterVehTimeElasped = -1;
		}
		
		// Enable zone names if required on spawn so they're not there in class selection
		// 1 second delay so it doesn't display your selection area when you spawn
		if ((int)dwThisTick >= m_iDisplayZoneTick) {
			pGame->EnableZoneNames(pNetGame->m_bZoneNames);
		}

		// SERVER CHECKPOINTS UPDATE
		pGame->UpdateCheckpoints();

		// STATS UPDATES
		if((dwThisTick - m_dwLastStatsUpdateTick) > STATS_UPDATE_TICKS) {
			SendStatsUpdate();
			m_dwLastStatsUpdateTick = dwThisTick;
		}
		
		CheckWeapons();
		
		// Handle interior updates to the server
		BYTE byteInterior = pGame->GetActiveInterior();
		if (byteInterior != m_byteCurInterior) {
			UpdateRemoteInterior(byteInterior);
		}
		
		// Disabled weapons
		if ((byteInterior != 0) &&
			(!pNetGame->m_bAllowWeapons)) {
			m_pPlayerPed->SetArmedWeapon(0);
		}
				
		// The new regime for adjusting sendrates is based on the number
		// of players that will be effected by this update. The more players
		// there are within a small radius, the more we must scale back
		// the number of sends.
		int iNumberOfPlayersInLocalRange=0;
		iNumberOfPlayersInLocalRange = DetermineNumberOfPlayersInLocalRange();
		if(!iNumberOfPlayersInLocalRange) iNumberOfPlayersInLocalRange = 10;

		// PLAYER DATA UPDATES
		if (m_bIsSpectating) {
			ProcessSpectating();
			m_bPassengerDriveByMode = false;
		}
		// DRIVER CONDITIONS
		else if(m_pPlayerPed->IsInVehicle() && !m_pPlayerPed->IsAPassenger())
		{
            ProcessInCarWorldBounds();
			
			CVehiclePool *pVehiclePool = pNetGame->GetVehiclePool();
			CVehicle *pVehicle;
			if (pVehiclePool)
				m_CurrentVehicle = pVehiclePool->FindIDFromGtaPtr(m_pPlayerPed->GetGtaVehicle());
			pVehicle = pVehiclePool->GetAt(m_CurrentVehicle);

			SendVehicleDamageStatus(m_CurrentVehicle);

			// HANDLE DRIVING AN RC VEHICLE
			if(pVehicle && !m_bInRCMode && pVehicle->IsRCVehicle()) {
				m_pPlayerPed->Remove();
				m_bInRCMode = true;
			}

			if(m_bInRCMode && !pVehicle) {
				m_pPlayerPed->SetHealth(0.0f);
				m_pPlayerPed->SetDead();
			}

			if(m_bInRCMode && pVehicle && pVehicle->GetHealth() == 0.0f) {
				m_pPlayerPed->SetHealth(0.0f);
				m_pPlayerPed->SetDead();
			}

			if((dwThisTick - m_dwLastSendTick) > (UINT)GetOptimumInCarSendRate(iNumberOfPlayersInLocalRange)) {
				m_dwLastSendTick = GetTickCount();
				SendInCarFullSyncData(); // INCAR - DRIVER				
			}
			m_bPassengerDriveByMode = false;
		}
		// ONFOOT CONDITIONS
		else if(m_pPlayerPed->GetActionTrigger() == ACTION_NORMAL || m_pPlayerPed->GetActionTrigger() == ACTION_SCOPE) // Scoped - THIS IS A QUICK HACK CHANGEME
		{
			UpdateSurfing();

			// MAKE MY HEAD MOVE WITH THE CAMERA
			if((dwThisTick - m_dwLastHeadUpdate) > 1000 && !pConfig->GetInt("disableheadmove")) {
                VECTOR LookAt;
				CAMERA_AIM *Aim = GameGetInternalAim();
				LookAt.X = Aim->pos1x + (Aim->f1x * 20.0f);
				LookAt.Y = Aim->pos1y + (Aim->f1y * 20.0f);
				LookAt.Z = Aim->pos1z + (Aim->f1z * 20.0f);
				pGame->FindPlayerPed()->ApplyCommandTask("FollowPedSA",0,2000,-1,&LookAt,0,0.1f,500,3,0);
				m_dwLastHeadUpdate = dwThisTick;
			}

			if(m_bInRCMode) {
				m_bInRCMode = false;
				m_pPlayerPed->Add();
			}

			HandlePassengerEntry();
			ProcessOnFootWorldBounds();

			// TIMING FOR ONFOOT SEND RATES
			if((dwThisTick - m_dwLastSendTick) > (UINT)GetOptimumOnFootSendRate(iNumberOfPlayersInLocalRange)) {
				m_dwLastSendTick = GetTickCount();
				SendOnFootFullSyncData(); // ONFOOT

				if (m_CurrentVehicle != 0xFFFF) {
					m_LastVehicle = m_CurrentVehicle;
					m_CurrentVehicle = 0xFFFF;
				}
			}

			// TIMING FOR ONFOOT AIM SENDS
			WORD lrAnalog,udAnalog;
			UINT uiKeys = m_pPlayerPed->GetKeys(&lrAnalog,&udAnalog);
			
			// Not targeting or firing. We need a very slow rate to sync the head.
			if(!IS_TARGETING(uiKeys) && !IS_FIRING(uiKeys)) {
				if((dwThisTick - m_dwLastAimSendTick) > (UINT)NETMODE_HEADSYNC_SENDRATE){
					m_dwLastAimSendTick = dwThisTick;
					SendAimSyncData();
				}
			}
			// Targeting only. Just synced for show really, so use a slower rate
			else if(IS_TARGETING(uiKeys) && !IS_FIRING(uiKeys)) {
				if((dwThisTick - m_dwLastAimSendTick) > (UINT)NETMODE_AIM_SENDRATE+(iNumberOfPlayersInLocalRange*NETMODE_SEND_MULTIPLIER)){
					m_dwLastAimSendTick = dwThisTick;
					SendAimSyncData();
				}
			}
			// Targeting and Firing. Needs a very accurate send rate.
			else if(IS_TARGETING(uiKeys) && IS_FIRING(uiKeys)) {
				if((dwThisTick - m_dwLastAimSendTick) > (UINT)NETMODE_FIRING_SENDRATE+(iNumberOfPlayersInLocalRange*NETMODE_SEND_MULTIPLIER)) {
					m_dwLastAimSendTick = dwThisTick;
					SendAimSyncData();
				}
			}
			// Firing without targeting. Needs a normal onfoot sendrate.
			else if(!IS_TARGETING(uiKeys) && IS_FIRING(uiKeys)) {
				if((dwThisTick - m_dwLastAimSendTick) > (UINT)GetOptimumOnFootSendRate(iNumberOfPlayersInLocalRange)) {
					m_dwLastAimSendTick = dwThisTick;
					SendAimSyncData();
				}
			}

			m_bPassengerDriveByMode = false;
		}
		// PASSENGER CONDITIONS
		else if(m_pPlayerPed->IsInVehicle() && m_pPlayerPed->IsAPassenger())
		{
			if(m_bInRCMode) {
				m_bInRCMode = false;
				m_pPlayerPed->Add();
			}

			GTA_CONTROLSET * Controls = GameGetInternalKeys();
			int iWeapon = m_pPlayerPed->GetCurrentWeapon();
		
			// FOR ENTERING PASSENGER DRIVEBY MODE
			if(!m_bPassengerDriveByMode && Controls->wKeys1[18]) {
				// NOT IN DRIVEBY MODE AND HORN HELD 
				if(iWeapon == WEAPON_UZI || iWeapon == WEAPON_MP5 || iWeapon == WEAPON_TEC9) {
					if(m_pPlayerPed->StartPassengerDriveByMode()) {
						m_bPassengerDriveByMode = true;
					}	
				}
			}
			if((dwThisTick - m_dwLastSendTick) > (UINT)GetOptimumInCarSendRate(iNumberOfPlayersInLocalRange)) {
				m_dwLastSendTick = GetTickCount();
				SendPassengerFullSyncData(); // INCAR - PASSENGER
			}
		}
			
		m_ulThisSyncFrame++;
	}

	// HANDLE !IsActive spectating
	if(m_bIsSpectating && !m_bIsActive) {
		ProcessSpectating();
		return true;
	}

	// HANDLE THE 'WANTS ANOTHER CLASS BUTTON'
	if(!m_bWantsAnotherClass && GetAsyncKeyState(VK_F4))
	{
		m_bWantsAnotherClass = true;
		pChatWindow->AddInfoMessage("Returning to class selection after next death");
	}

	// HANDLE NEEDS TO RESPAWN
	if(m_bIsWasted && (m_pPlayerPed->GetActionTrigger() != ACTION_WASTED) &&
		(m_pPlayerPed->GetActionTrigger() != ACTION_DEATH) )
	{
		if( !m_bWantsAnotherClass &&
			pNetGame->GetGameState() == GAMESTATE_CONNECTED ) {

			//pGame->ToggleKeyInputsDisabled(TRUE);
			
			if (m_pPlayerPed->GetHealth() > 0.0f)
			{
				Spawn();
			}

		} else {
			m_bIsWasted = false;
			HandleClassSelection();
			m_bWantsAnotherClass = false;
		}

		return true;
	}

	// HAND CONTROL OVER TO THE GAMELOGIC
	if((m_pPlayerPed->GetActionTrigger() != ACTION_WASTED) &&
		(m_pPlayerPed->GetActionTrigger() != ACTION_DEATH) &&
		pNetGame->GetGameState() == GAMESTATE_CONNECTED &&
		!m_bIsActive && !m_bIsSpectating) {
	
		ProcessClassSelection();
	}

	return true;
}

//----------------------------------------------------------

void CLocalPlayer::HandlePassengerEntry()
{
	GTA_CONTROLSET *pControls = GameGetInternalKeys();
	CVehiclePool *pVehiclePool = pNetGame->GetVehiclePool();

	if(pControls->wKeys1[8] && !pControls->wKeys2[8]) { // RECRUIT KEY JUST DOWN.

		VEHICLEID ClosestVehicleID = pVehiclePool->FindNearestToLocalPlayerPed();

		if(ClosestVehicleID < MAX_VEHICLES && pVehiclePool->GetSlotState(ClosestVehicleID)) {
			CVehicle *pVehicle = pVehiclePool->GetAt(ClosestVehicleID);
			if(pVehicle->GetDistanceFromLocalPlayerPed() < 4.5f) {
				// If armed with the para, set to fists
				if(m_pPlayerPed->GetCurrentWeapon() == WEAPON_PARACHUTE) {
					m_pPlayerPed->SetArmedWeapon(0);
				}
				// Enter locally
				m_pPlayerPed->EnterVehicle(pVehicle->m_dwGTAId,TRUE);
				// Send Net Notification
				SendEnterVehicleNotification(ClosestVehicleID,TRUE);
			}
		}
	}
}

//----------------------------------------------------------

void CLocalPlayer::ApplySpecialAction(BYTE byteSpecialAction)
{
	switch(byteSpecialAction) {

		case SPECIAL_ACTION_USEJETPACK:
			if(!m_pPlayerPed->IsInJetpackMode()) m_pPlayerPed->StartJetpack();
			break;

		case SPECIAL_ACTION_DANCE1:
			m_pPlayerPed->StartDancing(0);
			break;

		case SPECIAL_ACTION_DANCE2:
			m_pPlayerPed->StartDancing(1);
			break;

		case SPECIAL_ACTION_DANCE3:
			m_pPlayerPed->StartDancing(2);
			break;

		case SPECIAL_ACTION_DANCE4:
			m_pPlayerPed->StartDancing(3);
			break;

		case SPECIAL_ACTION_HANDSUP:
			m_pPlayerPed->HandsUp();
			break;

		case SPECIAL_ACTION_USECELLPHONE:
			if(!m_pPlayerPed->IsInVehicle()) {
				m_pPlayerPed->ToggleCellphone(1);
			}
			break;

		case SPECIAL_ACTION_STOPUSECELLPHONE:
			if(m_pPlayerPed->IsCellphoneEnabled()) {
				m_pPlayerPed->ToggleCellphone(0);
			}
			break;

		case SPECIAL_ACTION_URINATE:
			if(!m_pPlayerPed->IsInVehicle()) {
				m_pPlayerPed->StartPissing();
			}
			break;
	}
}

//----------------------------------------------------------

BYTE CLocalPlayer::GetSpecialAction()
{
	if (m_pPlayerPed->IsDucking())
		return SPECIAL_ACTION_DUCK;

	if(m_pPlayerPed->IsInJetpackMode()) {
		return SPECIAL_ACTION_USEJETPACK;
	}

	if (m_pPlayerPed->IsEnteringVehicle())
		return SPECIAL_ACTION_ENTER_VEHICLE;

	if (m_pPlayerPed->IsExitingVehicle())
		return SPECIAL_ACTION_EXIT_VEHICLE;

	if(m_pPlayerPed->IsDancing()) {
		switch(m_pPlayerPed->m_iDanceStyle) {
			case 0:
				return SPECIAL_ACTION_DANCE1;
			case 1:
				return SPECIAL_ACTION_DANCE2;
			case 2:
				return SPECIAL_ACTION_DANCE3;
			case 3:
				return SPECIAL_ACTION_DANCE4;
		}
	}

	if(m_pPlayerPed->HasHandsUp()) {
		//pChatWindow->AddDebugMessage("SPECIAL_ACTION_HANDSUP");
		return SPECIAL_ACTION_HANDSUP;
	}

	if(m_pPlayerPed->IsCellphoneEnabled()) {
		return SPECIAL_ACTION_USECELLPHONE;
	}

	if(m_pPlayerPed->IsPissing()) {
		return SPECIAL_ACTION_URINATE;
	}

	return SPECIAL_ACTION_NONE;
}

//----------------------------------------------------------

void CLocalPlayer::UpdateSurfing()
{	
	VEHICLE_TYPE *Contact = m_pPlayerPed->GetGtaContactVehicle();

	if(!Contact) {
		m_bSurfingMode = false;
		m_vecLockedSurfingOffsets = 0.0f;
		m_SurfingID = INVALID_VEHICLE_ID;
		return;
	}

	VEHICLEID vehID = pNetGame->GetVehiclePool()->FindIDFromGtaPtr(Contact);

	if(vehID && vehID != INVALID_VEHICLE_ID) {

		CVehicle *pVehicle = pNetGame->GetVehiclePool()->GetAt(vehID);

		if( pVehicle && pVehicle->IsOccupied() && 
			pVehicle->GetDistanceFromLocalPlayerPed() < 5.0f ) {

			VECTOR vecSpeed;
			VECTOR vecTurn;
			MATRIX4X4 matPlayer;
			MATRIX4X4 matVehicle;
			VECTOR vecVehiclePlane;
			WORD lr, ud;

			pVehicle->GetMatrix(&matVehicle);
			m_pPlayerPed->GetMatrix(&matPlayer);

			m_bSurfingMode = true;
			m_SurfingID = vehID;

			m_vecLockedSurfingOffsets.X = matPlayer.pos.X - matVehicle.pos.X;
			m_vecLockedSurfingOffsets.Y = matPlayer.pos.Y - matVehicle.pos.Y;
			m_vecLockedSurfingOffsets.Z = matPlayer.pos.Z - matVehicle.pos.Z;

			vecSpeed = Contact->entity.vecMoveSpeed;
			vecTurn = Contact->entity.vecTurnSpeed;

			m_pPlayerPed->GetKeys(&lr,&ud);

			if(!lr && !ud) {
				// if they're not trying to translate, keep their
				// move and turn speeds identical to the surfing vehicle
				m_pPlayerPed->SetMoveSpeedVector(vecSpeed);
				m_pPlayerPed->SetTurnSpeedVector(vecTurn);
			}
			
			return;
		}
	}
	m_bSurfingMode = false;
	m_vecLockedSurfingOffsets = 0.0f;
	m_SurfingID = INVALID_VEHICLE_ID;
}

//----------------------------------------------------------

void CLocalPlayer::SendOnFootFullSyncData()
{
	RakNet::BitStream bsPlayerSync;
	MATRIX4X4 matPlayer;
	VECTOR vecMoveSpeed;
	WORD lrAnalog,udAnalog;
	UINT uiKeys = m_pPlayerPed->GetKeys(&lrAnalog,&udAnalog);

	ONFOOT_SYNC_DATA ofSync;

	m_pPlayerPed->GetMatrix(&matPlayer);
	m_pPlayerPed->GetMoveSpeedVector(&vecMoveSpeed);

	// GENERAL PLAYER SYNC DATA
	ofSync.lrAnalog = lrAnalog;
	ofSync.udAnalog = udAnalog;
	ofSync.uiKeys = uiKeys;
	ofSync.vecPos = matPlayer.pos;

	// Rotation stuff
	ofSync.fRotation = m_pPlayerPed->GetTargetRotation();
	ofSync.byteHealth = (BYTE)m_pPlayerPed->GetHealth();
	ofSync.byteArmour = (BYTE)m_pPlayerPed->GetArmour();
	
	ofSync.byteCurrentWeapon = m_pPlayerPed->GetCurrentWeapon();
	ofSync.byteSpecialAction = GetSpecialAction();
		
	ofSync.vecMoveSpeed = vecMoveSpeed;

	// For vehicle surfing
	VEHICLE_TYPE* vehContact = m_pPlayerPed->GetGtaContactVehicle();
	VEHICLEID VehId = 0xFFFF;
	CVehicle* pVehicle = NULL;

	if ( m_bSurfingMode ) {
		ofSync.vecSurfOffsets = m_vecLockedSurfingOffsets;
		ofSync.SurfVehicleId = m_SurfingID;
	} else {
		ofSync.vecSurfOffsets = 0.0f;
		ofSync.SurfVehicleId = 0;
	}

	bsPlayerSync.Write((BYTE)ID_PLAYER_SYNC);
	bsPlayerSync.Write((PCHAR)&ofSync,sizeof(ONFOOT_SYNC_DATA));
	pNetGame->GetRakClient()->Send(&bsPlayerSync,HIGH_PRIORITY,UNRELIABLE_SEQUENCED,0);
}

//----------------------------------------------------------

void CLocalPlayer::SendAimSyncData()
{
	RakNet::BitStream bsAimSync;
	AIM_SYNC_DATA aimSync;
	CAMERA_AIM * caAim = m_pPlayerPed->GetCurrentAim();
	
	aimSync.byteCamMode = m_pPlayerPed->GetCameraMode();

	aimSync.vecAimf1.X = caAim->f1x;
	aimSync.vecAimf1.Y = caAim->f1y;
	aimSync.vecAimf1.Z = caAim->f1z;
	aimSync.vecAimf2.X = caAim->f2x;
	aimSync.vecAimf2.Y = caAim->f2y;
	aimSync.vecAimf2.Z = caAim->f2z;
	aimSync.vecAimPos.X = caAim->pos1x;
	aimSync.vecAimPos.Y = caAim->pos1y;
	aimSync.vecAimPos.Z = caAim->pos1z;

	aimSync.fAimZ = m_pPlayerPed->GetAimZ();

	aimSync.ucAspectRatio = (unsigned char)((CGame::GetAspectRatio() - 1.0f) * 255.0f);
	
	aimSync.byteCamExtZoom = (BYTE)(m_pPlayerPed->GetCameraExtendedZoom() * 63.0f);
	
	WEAPON_SLOT_TYPE* pwstWeapon = m_pPlayerPed->GetCurrentWeaponSlot();
	if (pwstWeapon->dwState == 2)
		aimSync.byteWeaponState = WS_RELOADING;
	else
		aimSync.byteWeaponState = (pwstWeapon->dwAmmoInClip > 1) ? WS_MORE_BULLETS : pwstWeapon->dwAmmoInClip;

	bsAimSync.Write((BYTE)ID_AIM_SYNC);
	bsAimSync.Write((PCHAR)&aimSync,sizeof(AIM_SYNC_DATA));
	pNetGame->GetRakClient()->Send(&bsAimSync,HIGH_PRIORITY,UNRELIABLE_SEQUENCED,0);
}

//----------------------------------------------------------

void CLocalPlayer::SendInCarFullSyncData()
{
	RakNet::BitStream bsVehicleSync;
	CVehiclePool *pVehiclePool = pNetGame->GetVehiclePool();

	MATRIX4X4 matPlayer;
	VECTOR vecMoveSpeed;

	WORD lrAnalog,udAnalog;
	UINT uiKeys = m_pPlayerPed->GetKeys(&lrAnalog,&udAnalog);
	CVehicle *pGameVehicle=NULL;
	
	INCAR_SYNC_DATA icSync;

	if(m_pPlayerPed)
	{
		icSync.VehicleID = pVehiclePool->FindIDFromGtaPtr(m_pPlayerPed->GetGtaVehicle());
		
		if(icSync.VehicleID == INVALID_VEHICLE_ID) return; // not valid

		icSync.lrAnalog = lrAnalog;
		icSync.udAnalog = udAnalog;
		icSync.uiKeys = uiKeys;

		// get the vehicle matrix
		pGameVehicle = pVehiclePool->GetAt(icSync.VehicleID);
		if(!pGameVehicle) return;

		pGameVehicle->GetMatrix(&matPlayer);

		CompressNormalVector(&matPlayer.right,&icSync.cvecRoll);
		CompressNormalVector(&matPlayer.up,&icSync.cvecDirection);

		icSync.vecPos = matPlayer.pos;
			
		pGameVehicle->GetMoveSpeedVector(&vecMoveSpeed);

		icSync.vecMoveSpeed = vecMoveSpeed;

		icSync.fCarHealth = pGameVehicle->GetHealth();
		icSync.bytePlayerHealth = (BYTE)m_pPlayerPed->GetHealth();
		icSync.bytePlayerArmour = (BYTE)m_pPlayerPed->GetArmour();

		// Note: Train Speed and Tire Popping values are mutually exclusive, which means
		//       if one is set, the other one will be affected.

		if( pGameVehicle->GetModelIndex() == TRAIN_PASSENGER_LOCO ||
			pGameVehicle->GetModelIndex() == TRAIN_FREIGHT_LOCO ||
			pGameVehicle->GetModelIndex() == TRAIN_TRAM) {
				icSync.fTrainSpeed = pGameVehicle->GetTrainSpeed();
		} else {
			icSync.fTrainSpeed = 0.0f;
			if (pNetGame->m_bTirePopping) {
				if (pGameVehicle->GetVehicleSubtype() == VEHICLE_SUBTYPE_BIKE) {
					icSync.byteTires[0] = pGameVehicle->GetWheelPopped(0);
					icSync.byteTires[1] = pGameVehicle->GetWheelPopped(1);
					icSync.byteTires[2] = 0;
					icSync.byteTires[3] = 0;
				} else if ( pGameVehicle->GetVehicleSubtype() == VEHICLE_SUBTYPE_CAR) {
					icSync.byteTires[0] = pGameVehicle->GetWheelPopped(0);
					icSync.byteTires[1] = pGameVehicle->GetWheelPopped(1);
					icSync.byteTires[2] = pGameVehicle->GetWheelPopped(2);
					icSync.byteTires[3] = pGameVehicle->GetWheelPopped(3);
				}
			}
		}
	
		icSync.TrailerID = 0;
		VEHICLE_TYPE* vehTrailer = (VEHICLE_TYPE*)pGameVehicle->m_pVehicle->dwTrailer;
		if (vehTrailer != NULL)	{
			if ( ScriptCommand(&is_trailer_on_cab, 
				 pVehiclePool->FindGtaIDFromGtaPtr(vehTrailer), 
				 pGameVehicle->m_dwGTAId) )
			{
				icSync.TrailerID = pVehiclePool->FindIDFromGtaPtr(vehTrailer);
			} else {
				icSync.TrailerID = 0;
			}
		}

		// SPECIAL STUFF
		if(pGameVehicle->GetModelIndex() == HYDRA)
			icSync.dwHydraThrustAngle = pGameVehicle->GetHydraThrusters();
		else icSync.dwHydraThrustAngle = 0;
		
		// Some other SPECIAL sync stuff (these can be optimized for specific vehicles, someday!)
		if(pGameVehicle->IsSirenOn()) icSync.byteSirenOn = 1;
		else icSync.byteSirenOn = 0;

		if(pGameVehicle->GetLandingGearState() == LGS_UP) 
			icSync.byteLandingGearState = 0;
		else icSync.byteLandingGearState = 1;

		if (IS_FIRING(uiKeys)) { // firing
			BYTE byteCurrentWeapon = m_pPlayerPed->GetCurrentWeapon();
			if (byteCurrentWeapon == WEAPON_UZI || 
				byteCurrentWeapon == WEAPON_MP5 || 
				byteCurrentWeapon == WEAPON_TEC9) 
				icSync.byteCurrentWeapon = byteCurrentWeapon;
		}

		icSync.ucInfo = 0;
		if (pGameVehicle->IsOnItsSide())
			icSync.ucInfo |= 0x01;
		if (pGameVehicle->IsUpsideDown())
			icSync.ucInfo |= 0x02;
		if (pGameVehicle->IsSirenOn())
			icSync.ucInfo |= 0x04;
		if (pGameVehicle->IsWrecked())
			icSync.ucInfo |= 0x08;
		if (pGameVehicle->HasSunk())
			icSync.ucInfo |= 0x10;

		// send
		bsVehicleSync.Write((BYTE)ID_VEHICLE_SYNC);
		bsVehicleSync.Write((PCHAR)&icSync,sizeof(INCAR_SYNC_DATA));
		pNetGame->GetRakClient()->Send(&bsVehicleSync,HIGH_PRIORITY,UNRELIABLE_SEQUENCED,0);

		// For the tank/firetruck, we need some info on aiming
		if (pGameVehicle->HasTurret()) SendAimSyncData();		
			
		if (icSync.TrailerID && icSync.TrailerID < MAX_VEHICLES)
		{
			MATRIX4X4 matTrailer;
			TRAILER_SYNC_DATA trSync;
			CVehicle* pTrailer = pVehiclePool->GetAt(icSync.TrailerID);
			if (pTrailer)
			{
				pTrailer->GetMatrix(&matTrailer);
				
				CompressNormalVector(&matTrailer.right,&trSync.cvecRoll);
				CompressNormalVector(&matTrailer.up,&trSync.cvecDirection);
				
				trSync.vecPos = matTrailer.pos;
				
				pTrailer->GetMoveSpeedVector(&trSync.vecMoveSpeed);
				
				RakNet::BitStream bsTrailerSync;
				bsTrailerSync.Write((BYTE)ID_TRAILER_SYNC);
				bsTrailerSync.Write((PCHAR)&trSync, sizeof (TRAILER_SYNC_DATA));
				pNetGame->GetRakClient()->Send(&bsTrailerSync,HIGH_PRIORITY,UNRELIABLE_SEQUENCED,0);
			}
		}
	}
}

//----------------------------------------------------------

void CLocalPlayer::SendPassengerFullSyncData()
{
	RakNet::BitStream bsPassengerSync;
	CVehiclePool *pVehiclePool = pNetGame->GetVehiclePool();

	WORD lrAnalog,udAnalog;
	UINT uiKeys = m_pPlayerPed->GetKeys(&lrAnalog,&udAnalog);
	PASSENGER_SYNC_DATA psSync;
	MATRIX4X4 mat;

	psSync.VehicleID = pVehiclePool->FindIDFromGtaPtr(m_pPlayerPed->GetGtaVehicle());
	
	if(psSync.VehicleID == INVALID_VEHICLE_ID) return;  // not valid

	psSync.lrAnalog = lrAnalog;
	psSync.udAnalog = udAnalog;
	psSync.uiKeys = uiKeys;
	psSync.bytePlayerHealth = (BYTE)m_pPlayerPed->GetHealth();
	psSync.bytePlayerArmour = (BYTE)m_pPlayerPed->GetArmour();

	psSync.byteSeatFlags = m_pPlayerPed->GetVehicleSeatID();
	psSync.byteDriveBy = m_bPassengerDriveByMode;

	psSync.byteCurrentWeapon = m_pPlayerPed->GetCurrentWeapon();

	m_pPlayerPed->GetMatrix(&mat);
	psSync.vecPos = mat.pos;

	bsPassengerSync.Write((BYTE)ID_PASSENGER_SYNC);
	bsPassengerSync.Write((PCHAR)&psSync,sizeof(PASSENGER_SYNC_DATA));
	pNetGame->GetRakClient()->Send(&bsPassengerSync,HIGH_PRIORITY,UNRELIABLE_SEQUENCED,0);

	if(m_bPassengerDriveByMode)	SendAimSyncData();
}

//----------------------------------------------------------

int CLocalPlayer::GetOptimumInCarSendRate(int iPlayersEffected)
{
	CVehiclePool *pVehiclePool = pNetGame->GetVehiclePool();
	CVehicle	 *pGameVehicle=NULL;
	VECTOR		 vecMoveSpeed;
	VEHICLEID	 VehicleID=0;

	if(m_pPlayerPed) {

		VehicleID = pVehiclePool->FindIDFromGtaPtr(m_pPlayerPed->GetGtaVehicle());
		pGameVehicle = pVehiclePool->GetAt(VehicleID);

		if(pGameVehicle) {

			pGameVehicle->GetMoveSpeedVector(&vecMoveSpeed);

			if( (vecMoveSpeed.X == 0.0f) &&
				(vecMoveSpeed.Y == 0.0f) &&
				(vecMoveSpeed.Z == 0.0f) ) {

				if(pNetGame->IsLanMode()) return LANMODE_IDLE_INCAR_SENDRATE;
				else return (NETMODE_IDLE_INCAR_SENDRATE + (int)iPlayersEffected*NETMODE_SEND_MULTIPLIER);
			}
			else {
				if(pNetGame->IsLanMode()) return LANMODE_NORMAL_INCAR_SENDRATE;
				else return (m_sNormalIncarRate + (int)iPlayersEffected*NETMODE_SEND_MULTIPLIER);
			}
		}
	}
	return 1000;
}

//----------------------------------------------------------

int CLocalPlayer::GetOptimumOnFootSendRate(int iPlayersEffected)
{	
	VECTOR	 vecMoveSpeed;
	WORD lrAnalog,udAnalog;
	UINT uiKeys = m_pPlayerPed->GetKeys(&lrAnalog,&udAnalog);

	if(m_pPlayerPed) {

		m_pPlayerPed->GetMoveSpeedVector(&vecMoveSpeed);

		if( (vecMoveSpeed.X == 0.0f) &&
			(vecMoveSpeed.Y == 0.0f) &&
			(vecMoveSpeed.Z == 0.0f) &&
			!IS_TARGETING(uiKeys) ) {

			if(pNetGame->IsLanMode()) return LANMODE_IDLE_ONFOOT_SENDRATE;
			else return (NETMODE_IDLE_ONFOOT_SENDRATE + (int)iPlayersEffected*NETMODE_SEND_MULTIPLIER); // scale to number of players.
		}
		else {
			if(pNetGame->IsLanMode()) return LANMODE_NORMAL_ONFOOT_SENDRATE;
			else return (m_sNormalOnfootRate + (int)iPlayersEffected*NETMODE_SEND_MULTIPLIER); // scale to number of players.
		}
	}

	return 1000;
}

//----------------------------------------------------------

void CLocalPlayer::SendWastedNotification()
{
	RakNet::BitStream bsPlayerDeath;
	BYTE byteDeathReason;
	BYTE byteWhoWasResponsible;

	byteDeathReason = m_pPlayerPed->FindDeathReasonAndResponsiblePlayer(&byteWhoWasResponsible);
	
	bsPlayerDeath.Write(byteDeathReason);
	bsPlayerDeath.Write(byteWhoWasResponsible);
	pNetGame->GetRakClient()->RPC(RPC_Death,&bsPlayerDeath,HIGH_PRIORITY,RELIABLE_SEQUENCED,0,false);
}

//----------------------------------------------------------

void CLocalPlayer::RequestClass(int iClass)
{
	RakNet::BitStream bsSpawnRequest;
	bsSpawnRequest.Write(iClass);
	pNetGame->GetRakClient()->RPC(RPC_RequestClass,&bsSpawnRequest,HIGH_PRIORITY,RELIABLE,0,false);
}

//----------------------------------------------------------

void CLocalPlayer::RequestSpawn()
{
	pNetGame->Send(RPC_RequestSpawn, NULL);
}

//----------------------------------------------------------

void CLocalPlayer::SetSpawnInfo(PLAYER_SPAWN_INFO *pSpawn)
{
	memcpy(&m_SpawnInfo,pSpawn,sizeof(PLAYER_SPAWN_INFO));
	m_bHasSpawnInfo = true;
}

//----------------------------------------------------------

void CLocalPlayer::UpdateRemoteInterior(BYTE byteInterior)
{
	m_byteCurInterior = byteInterior;
	RakNet::BitStream bsUpdateInterior;
	bsUpdateInterior.Write(byteInterior);
	pNetGame->GetRakClient()->RPC(RPC_SetInteriorId,&bsUpdateInterior,HIGH_PRIORITY,RELIABLE,0,false);
}

//----------------------------------------------------------

bool CLocalPlayer::Spawn()
{
	if(!m_bHasSpawnInfo) return false;

	pGame->DisplayHud(true);
	m_pPlayerPed->TogglePlayerControllable(1);

	iTimesDataModified = 0;

	pGame->RefreshStreamingAt(m_SpawnInfo.vecPos.X,m_SpawnInfo.vecPos.Y);

	m_pPlayerPed->RestartIfWastedAt(&m_SpawnInfo.vecPos, m_SpawnInfo.fRotation);
	m_pPlayerPed->SetModelIndex(m_SpawnInfo.iSkin);
	m_pPlayerPed->ClearAllWeapons();
	m_pPlayerPed->ResetDamageEntity();

	SetTeam(m_SpawnInfo.byteTeam);

	if(m_SpawnInfo.iSpawnWeapons[2] != (-1))
		m_pPlayerPed->GiveWeapon(m_SpawnInfo.iSpawnWeapons[2],
		m_SpawnInfo.iSpawnWeaponsAmmo[2]);

	if(m_SpawnInfo.iSpawnWeapons[1] != (-1))
		m_pPlayerPed->GiveWeapon(m_SpawnInfo.iSpawnWeapons[1],
		m_SpawnInfo.iSpawnWeaponsAmmo[1]);

	if(m_SpawnInfo.iSpawnWeapons[0] != (-1))
		m_pPlayerPed->GiveWeapon(m_SpawnInfo.iSpawnWeapons[0],
		m_SpawnInfo.iSpawnWeaponsAmmo[0]);

	m_pPlayerPed->ResetForRespawn();

	pGame->SetMaxStats();
	pGame->DisableTrainTraffic();
	
	// No fading CCamera_Fade. (applied here because otherwise there's audio volume issues)
	//UnFuck(0x50AC20,3);
	*(PBYTE)0x50AC20 = 0xC2;
	*(PBYTE)0x50AC21 = 0x08;
	*(PBYTE)0x50AC22 = 0x00;
	
	if (m_pPlayerPed->IsInVehicle())
		m_pPlayerPed->RemoveFromVehicleAndPutAt(m_SpawnInfo.vecPos.X,
			m_SpawnInfo.vecPos.Y, m_SpawnInfo.vecPos.Z);
	else
		m_pPlayerPed->TeleportTo(m_SpawnInfo.vecPos.X,
			m_SpawnInfo.vecPos.Y, (m_SpawnInfo.vecPos.Z));

	m_pPlayerPed->SetTargetRotation(m_SpawnInfo.fRotation);
	
	pGame->GetCamera()->Restore(); // Restore camera without jump-cut
	//pGame->GetCamera();->SetBehindPlayer(); // will be handled by SetInitialState()

	if (!bFirstSpawn)
		m_pPlayerPed->SetInitialState();
	else
		bFirstSpawn = false;

	m_bIsWasted = false;
	m_bIsActive = true;
	m_bWaitingForSpawnRequestReply = false;

	// Let the rest of the network know we're spawning.
	pNetGame->GetRakClient()->RPC(RPC_Spawn,NULL,HIGH_PRIORITY,
		RELIABLE_SEQUENCED,0,false);

	m_iDisplayZoneTick = GetTickCount() + 1000;

	if (pSpawnScreen)
		pSpawnScreen->ToggleVisibility(false);

	m_bControlsVisible = false;
	
	return true;
}

//----------------------------------------------------------

void CLocalPlayer::Say(PCHAR szText)
{
	size_t uiTextLen = strlen(szText);

	RakNet::BitStream bsSend;
	bsSend.Write(uiTextLen);
	bsSend.Write(szText, uiTextLen);

	pNetGame->GetRakClient()->RPC(RPC_Chat,&bsSend,HIGH_PRIORITY,RELIABLE,0,false);
	
	// Comment by spookie:
	//   Local player chat is now sent to the server so it can be filtered by the
	//   scripts before it is displayed in the chat window.

	// Process chat message to chat window.
	//pChatWindow->AddChatMessage(pNetGame->GetPlayerPool()->GetLocalPlayerName(),
	//	GetPlayerColorAsARGB(),szText);
}

//----------------------------------------------------------

void CLocalPlayer::SendEnterVehicleNotification(VEHICLEID VehicleID, bool bPassenger)
{
	RakNet::BitStream bsSend;

	//pChatWindow->AddDebugMessage("Enter Vehicle: %u %d",VehicleID,bPassenger);

	bsSend.Write(VehicleID);
	bsSend.Write(bPassenger);

	pNetGame->GetRakClient()->RPC(RPC_EnterVehicle,&bsSend,HIGH_PRIORITY,RELIABLE_SEQUENCED,0,false);

	CVehiclePool *pVehiclePool = pNetGame->GetVehiclePool();
	CVehicle* pVehicle = pVehiclePool->GetAt(VehicleID);

	if (pVehicle && pVehicle->IsATrainPart()) {
		//DWORD dwVehicle = pVehicle->m_dwGTAId;
		//ScriptCommand(&camera_on_vehicle, dwVehicle, 3, 2);
		dwEnterVehTimeElasped = GetTickCount();
	}
}

//----------------------------------------------------------

void CLocalPlayer::SendExitVehicleNotification(VEHICLEID VehicleID)
{
	RakNet::BitStream bsSend;
	//pChatWindow->AddDebugMessage("Sent Exit: %u\n",byteVehicleID);

	CVehiclePool *pVehiclePool = pNetGame->GetVehiclePool();
	CVehicle* pVehicle = pVehiclePool->GetAt(VehicleID);
	
	if(pVehicle)
	{ 
		if (!m_pPlayerPed->IsAPassenger()) {
			// This allows the code to sync vehicles blowing up without them being occupied
			m_LastVehicle = VehicleID;
		}

		if ( pVehicle->IsATrainPart() )	{
			pGame->GetCamera()->SetBehindPlayer();
		}

		if(!pVehicle->IsRCVehicle()) {
			bsSend.Write(VehicleID);
			pNetGame->GetRakClient()->RPC(RPC_ExitVehicle,&bsSend,HIGH_PRIORITY,RELIABLE_SEQUENCED,0,false);
		}
	}
}

//----------------------------------------------------

void CLocalPlayer::SetPlayerColor(DWORD dwColor)
{
	SetRadarColor(pNetGame->GetPlayerPool()->GetLocalPlayerID(),dwColor);	
}

//----------------------------------------------------

DWORD CLocalPlayer::GetPlayerColorAsRGBA()
{
	return TranslateColorCodeToRGBA(pNetGame->GetPlayerPool()->GetLocalPlayerID());
}

//----------------------------------------------------

DWORD CLocalPlayer::GetPlayerColorAsARGB()
{
	return (TranslateColorCodeToRGBA(pNetGame->GetPlayerPool()->GetLocalPlayerID()) >> 8) | 0xFF000000;	
}

//----------------------------------------------------

void CLocalPlayer::ProcessOnFootWorldBounds()
{
	if(pGame->GetActiveInterior() != 0) return; // can't enforce inside interior

	if(m_pPlayerPed->EnforceWorldBoundries(pNetGame->m_WorldBounds[0],pNetGame->m_WorldBounds[1],
		pNetGame->m_WorldBounds[2],pNetGame->m_WorldBounds[3]))
	{
		m_pPlayerPed->SetArmedWeapon(0);
		pGame->DisplayGameText("Stay within the ~r~world boundries",1000,5);
	}
}

//----------------------------------------------------

void CLocalPlayer::ProcessInCarWorldBounds()
{
	CVehiclePool *pVehiclePool = pNetGame->GetVehiclePool();
	VEHICLEID VehicleID = (VEHICLEID)pVehiclePool->FindIDFromGtaPtr(m_pPlayerPed->GetGtaVehicle());
	CVehicle *pGameVehicle;

	if(pGame->GetActiveInterior() != 0) return; // can't enforce inside interior

	if(VehicleID != INVALID_VEHICLE_ID) {
		pGameVehicle = pVehiclePool->GetAt(VehicleID);
		if(!pGameVehicle) return;

		if( pGameVehicle->EnforceWorldBoundries(
			pNetGame->m_WorldBounds[0],pNetGame->m_WorldBounds[1],
			pNetGame->m_WorldBounds[2],pNetGame->m_WorldBounds[3]) )
		{
			pGame->DisplayGameText("Stay within the ~r~world boundries",1000,5);
		}
	}
}

//-----------------------------------------------------

void CLocalPlayer::SendStatsUpdate()
{
	RakNet::BitStream bsStats;
	int iMoney = pGame->GetLocalMoney();
	WORD wAmmo = m_pPlayerPed->GetAmmo();
	//ScriptCommand(&get_player_weapon_ammo, GetCurrentWeapon()

	bsStats.Write((BYTE)ID_STATS_UPDATE);
	bsStats.Write(iMoney);
	bsStats.Write(wAmmo);
	pNetGame->GetRakClient()->Send(&bsStats,HIGH_PRIORITY,UNRELIABLE,0);
}

//----------------------------------------------------------

void CLocalPlayer::ProcessClassSelection(int iControlID)
{
	DWORD			dwTicksSinceLastSelection;
	DWORD			dwTickNow;
	MATRIX4X4		matPlayer;

	dwTickNow = GetTickCount();
	pGame->DisplayHud(false);

	// DONT ALLOW ANY ACTIONS IF WE'RE STILL FADING OR WAITING.
	if((dwTickNow - m_dwInitialSelectionTick) < 2000) return;

	//ApplySpawnAnim(m_iCurSpawnAnimIndex);
	if (m_bControlsVisible)
	{
		if (!m_bWaitingForSpawnRequestReply &&
			m_bAllowedClass &&
			((GetAsyncKeyState(VK_SHIFT) & 0x8000) || iControlID == ID_CONTROL_SPAWN) &&
			!pCmdWindow->isEnabled())
		{
			RequestSpawn();
			m_bWaitingForSpawnRequestReply = true;
			return;
		}
		else
		{
			// GRAB PLAYER MATRIX FOR SOUND POSITION
			m_pPlayerPed->GetMatrix(&matPlayer);
			dwTicksSinceLastSelection = dwTickNow - m_dwLastSpawnSelectionTick; // used to delay reselection.

			if (dwTicksSinceLastSelection > 250) {
				// ALLOW ANOTHER SELECTION WITH THE LEFT KEY
				if ((GetAsyncKeyState(VK_LEFT) & 0x8000) || iControlID == ID_CONTROL_LEFT) { // LEFT ARROW
					m_dwLastSpawnSelectionTick = dwTickNow;

					if (m_iSelectedClass == 0) m_iSelectedClass = (pNetGame->m_iSpawnsAvailable - 1);
					else m_iSelectedClass--;

					pGame->PlaySoundFX(1053, matPlayer.pos.X, matPlayer.pos.Y, matPlayer.pos.Z);
					RequestClass(m_iSelectedClass);
					m_bAllowedClass = true;
				}
				// ALLOW ANOTHER SELECTION WITH THE RIGHT KEY
				else if ((GetAsyncKeyState(VK_RIGHT) & 0x8000) || iControlID == ID_CONTROL_RIGHT) { // RIGHT ARROW
					m_dwLastSpawnSelectionTick = dwTickNow;

					if (m_iSelectedClass == (pNetGame->m_iSpawnsAvailable - 1)) m_iSelectedClass = 0;
					else m_iSelectedClass++;

					pGame->PlaySoundFX(1052, matPlayer.pos.X, matPlayer.pos.Y, matPlayer.pos.Z);
					RequestClass(m_iSelectedClass);
					m_bAllowedClass = true;
				}
			}
		}
	}
	else
	{
		if (pSpawnScreen)
			pSpawnScreen->ToggleVisibility(true);

		m_bControlsVisible = true;
	}
}

//----------------------------------------------------------

void CLocalPlayer::HandleClassSelection()
{
	if(m_pPlayerPed) {
		m_pPlayerPed->SetInitialState();
		m_pPlayerPed->SetHealth(100.0f);
		m_pPlayerPed->TogglePlayerControllable(0);
	}
	RequestClass(m_iSelectedClass);
	m_dwInitialSelectionTick = m_dwLastSpawnSelectionTick = GetTickCount();
}

//----------------------------------------------------------

void CLocalPlayer::HandleClassSelectionOutcome(bool bOutcome)
{
	if(bOutcome) {
		if(m_pPlayerPed) {
			m_pPlayerPed->ClearAllWeapons();
			m_pPlayerPed->SetModelIndex(m_SpawnInfo.iSkin);
		}
		m_bAllowedClass = true;
	}
	else
		m_bAllowedClass = false;
}

//-----------------------------------------------------------
// Check all the players unarmed weapons to see if they've been changed and inform the server if they have

void CLocalPlayer::CheckWeapons()
{
	if (m_pPlayerPed->IsInVehicle()) return;
	BYTE i;
	BYTE byteCurWep = m_pPlayerPed->GetCurrentWeapon();
	bool bMSend = false;

	RakNet::BitStream bsWeapons;
	bsWeapons.Write((BYTE)ID_WEAPONS_UPDATE);

	for (i = 0; i < 13; i++)
	{
		if (m_byteLastWeapon[i] != byteCurWep)
		{
			//bsWeapons.Write(i);
			bool bSend = false;
			if (m_byteLastWeapon[i] != m_pPlayerPed->m_pPed->WeaponSlots[i].dwType)
			{
				// non-current weapon has changed
				m_byteLastWeapon[i] = (BYTE)m_pPlayerPed->m_pPed->WeaponSlots[i].dwType;
				bSend = true;
			}
			//bsWeapons.Write(m_byteLastWeapon[i]);
			if (m_dwLastAmmo[i] != m_pPlayerPed->m_pPed->WeaponSlots[i].dwAmmo)
			{
				// non-current ammo has changed
				m_dwLastAmmo[i] = m_pPlayerPed->m_pPed->WeaponSlots[i].dwAmmo;
				bSend = true;
			}
			//bsWeapons.Write(m_dwLastAmmo[i]);
			if (bSend)
			{
				//pChatWindow->AddDebugMessage("Id: %u, Weapon: %u, Ammo: %d\n", i, m_byteLastWeapon[i], m_dwLastAmmo[i]);
				bsWeapons.Write((BYTE)i);
				bsWeapons.Write((BYTE)m_byteLastWeapon[i]);
				bsWeapons.Write((WORD)m_dwLastAmmo[i]);
				bMSend = true;
			}
		}
	}
	if (bMSend)
	{
		pNetGame->GetRakClient()->Send(&bsWeapons,HIGH_PRIORITY,UNRELIABLE,0);
	}
}

//-----------------------------------------------------------

int iNumPlayersInRange=0;
int iCyclesUntilNextCount=0;

int CLocalPlayer::DetermineNumberOfPlayersInLocalRange()
{
	int iRet=0;
	BYTE x=0; float fDistance;
	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();

	// We only want to perform this operation
	// once every few cycles. Doing it every frame
	// would be a little bit too CPU intensive.

	if(iCyclesUntilNextCount) {
		iCyclesUntilNextCount--;
		return iNumPlayersInRange;
	}

	// This part is only processed when iCyclesUntilNextCount is 0
	iCyclesUntilNextCount = 30;
	iNumPlayersInRange = 0;

	if(pPlayerPool) {		
		while(x!=MAX_PLAYERS) {
			if(pPlayerPool->GetSlotState(x)) {
				fDistance = pPlayerPool->GetAt(x)->GetDistanceFromLocalPlayer();
				if(fDistance <= 125.0f) {
					iNumPlayersInRange++;
				}
			}
			x++;
		}
	}

	return iNumPlayersInRange;
}

//-----------------------------------------------------------

void CLocalPlayer::ProcessSpectating()
{
	RakNet::BitStream bsSpectatorSync;
	SPECTATOR_SYNC_DATA spSync;
	MATRIX4X4 matPos;

	WORD lrAnalog,udAnalog;
	UINT uiKeys = m_pPlayerPed->GetKeys(&lrAnalog,&udAnalog);
	pGame->GetCamera()->GetMatrix(&matPos);

	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
	CVehiclePool *pVehiclePool = pNetGame->GetVehiclePool();

	if(!pPlayerPool || !pVehiclePool) return;
	
	spSync.vecPos = matPos.pos;
	spSync.lrAnalog = lrAnalog;
	spSync.udAnalog = udAnalog;
	spSync.uiKeys = uiKeys;

	if((GetTickCount() - m_dwLastSendSpecTick) > 200) {
		m_dwLastSendSpecTick = GetTickCount();
		bsSpectatorSync.Write((BYTE)ID_SPECTATOR_SYNC);
		bsSpectatorSync.Write((PCHAR)&spSync,sizeof(SPECTATOR_SYNC_DATA));
		pNetGame->GetRakClient()->Send(&bsSpectatorSync,HIGH_PRIORITY,UNRELIABLE,0);
	}
	
	pGame->DisplayHud(false);

	m_pPlayerPed->SetHealth(100.0f);
	GetPlayerPed()->TeleportTo(spSync.vecPos.X, spSync.vecPos.Y, spSync.vecPos.Z + 20.0f);
	
	// HANDLE SPECTATE PLAYER LEFT THE SERVER
	if (m_byteSpectateType == SPECTATE_TYPE_PLAYER &&
		!pPlayerPool->GetSlotState((BYTE)m_SpectateID)) {
			m_byteSpectateType = SPECTATE_TYPE_NONE;
			m_bSpectateProcessed = false;
	}

	// HANDLE SPECTATE PLAYER IS NO LONGER ACTIVE (ie Died)
	if (m_byteSpectateType == SPECTATE_TYPE_PLAYER &&
		pPlayerPool->GetSlotState((BYTE)m_SpectateID) &&
		(!pPlayerPool->GetAt((BYTE)m_SpectateID)->IsActive() ||
		pPlayerPool->GetAt((BYTE)m_SpectateID)->GetState() == PLAYER_STATE_WASTED)) {
			m_byteSpectateType = SPECTATE_TYPE_NONE;
			m_bSpectateProcessed = false;
	}

	if (m_bSpectateProcessed) return;

	if (m_byteSpectateType == SPECTATE_TYPE_NONE)
	{
		GetPlayerPed()->RemoveFromVehicleAndPutAt(0.0f, 0.0f, 10.0f);
		pGame->GetCamera()->SetPosition(50.0f, 50.0f, 50.0f, 0.0f, 0.0f, 0.0f);
		pGame->GetCamera()->LookAtPoint(60.0f, 60.0f, 50.0f, 2);
		m_bSpectateProcessed = true;
	}
	else if (m_byteSpectateType == SPECTATE_TYPE_PLAYER)
	{
		DWORD dwGTAId = 0;
		CPlayerPed *pPlayerPed = NULL;

		if (pPlayerPool->GetSlotState((BYTE)m_SpectateID)) {
			pPlayerPed = pPlayerPool->GetAt((BYTE)m_SpectateID)->GetPlayerPed();
			if(pPlayerPed) {
				dwGTAId = pPlayerPed->m_dwGTAId;
				//pChatWindow->AddDebugMessage("Spectating Player: 0x%X", dwGTAId);
				ScriptCommand(&camera_on_actor, dwGTAId, m_byteSpectateMode, 2);
				m_bSpectateProcessed = true;
			}
		}
	}
	else if (m_byteSpectateType == SPECTATE_TYPE_VEHICLE)
	{
		CVehicle *pVehicle = NULL;
		DWORD dwGTAId = 0;

		if (pVehiclePool->GetSlotState((VEHICLEID)m_SpectateID)) {
			pVehicle = pVehiclePool->GetAt((VEHICLEID)m_SpectateID);
			if(pVehicle) {
				dwGTAId = pVehicle->m_dwGTAId;
				//pChatWindow->AddDebugMessage("Spectating Vehicle: 0x%X", dwGTAId);
				ScriptCommand(&camera_on_vehicle, dwGTAId, m_byteSpectateMode, 2);
				m_bSpectateProcessed = true;
			}
		}
	}
}

//-----------------------------------------------------------

void CLocalPlayer::ToggleSpectating(bool bToggle)
{
	if (m_bIsSpectating && !bToggle) {
		Spawn();
	}
	m_bIsSpectating = bToggle;
	m_byteSpectateType = SPECTATE_TYPE_NONE;
	m_SpectateID = 0xFFFFFFFF;
	m_bSpectateProcessed = false;
}

//-----------------------------------------------------------

void CLocalPlayer::SpectateVehicle(VEHICLEID VehicleID)
{
	CVehiclePool *pVehiclePool = pNetGame->GetVehiclePool();

	//pChatWindow->AddDebugMessage("SpectateVehicle(%u)",VehicleID);

	if (pVehiclePool && pVehiclePool->GetSlotState(VehicleID)) {
		m_byteSpectateType = SPECTATE_TYPE_VEHICLE;
		m_SpectateID = VehicleID;
		m_bSpectateProcessed = false;
	}
}

//-----------------------------------------------------------

void CLocalPlayer::SpectatePlayer(BYTE bytePlayerID)
{
	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();

	//pChatWindow->AddDebugMessage("SpectatePlayer(%u)",bytePlayerID);

	if (pPlayerPool && pPlayerPool->GetSlotState(bytePlayerID)) {
		if (pPlayerPool->GetAt(bytePlayerID)->GetState() != PLAYER_STATE_NONE
			&& pPlayerPool->GetAt(bytePlayerID)->GetState() != PLAYER_STATE_WASTED) {
			m_byteSpectateType = SPECTATE_TYPE_PLAYER;
			m_SpectateID = bytePlayerID;
			m_bSpectateProcessed = false;
		}
	}
}

//-----------------------------------------------------------

void CLocalPlayer::SendVehicleDamageStatus(VEHICLEID VehicleID)
{
	CVehicle* pVehicle;
	int iPanels, iDoors;
	unsigned char ucLights, ucWheels;

	if (pNetGame->GetVehiclePool())
	{
		pVehicle = pNetGame->GetVehiclePool()->GetAt(VehicleID);
		if (pVehicle)
		{
			iPanels = pVehicle->GetCarPanelsDamageStatus();
			iDoors = pVehicle->GetCarDoorsDamageStatus();
			ucLights = pVehicle->GetCarLightsDamageStatus();
			ucWheels = pVehicle->GetCarOrBikeWheelStatus();
			
			if (VehicleID == m_iVehicleDamageStatus.MonitoredVehicleID)
			{
				if (m_iVehicleDamageStatus.iPanels != iPanels ||
					m_iVehicleDamageStatus.iDoors != iDoors ||
					m_iVehicleDamageStatus.ucLights != ucLights ||
					m_iVehicleDamageStatus.ucWheels != ucWheels)
				{
					RakNet::BitStream bsSend;

					m_iVehicleDamageStatus.iPanels = iPanels;
					m_iVehicleDamageStatus.iDoors = iDoors;
					m_iVehicleDamageStatus.ucLights = ucLights;
					m_iVehicleDamageStatus.ucWheels = ucWheels;

					bsSend.Write(m_iVehicleDamageStatus.MonitoredVehicleID);
					bsSend.Write(m_iVehicleDamageStatus.iPanels);
					bsSend.Write(m_iVehicleDamageStatus.iDoors);
					bsSend.Write(m_iVehicleDamageStatus.ucLights);
					bsSend.Write(m_iVehicleDamageStatus.ucWheels);

					pNetGame->Send(RPC_VehicleDamage, &bsSend);
				}
			}
			else
			{
				m_iVehicleDamageStatus.iPanels = iPanels;
				m_iVehicleDamageStatus.iDoors = iDoors;
				m_iVehicleDamageStatus.ucLights = ucLights;
				m_iVehicleDamageStatus.ucWheels = ucWheels;
				m_iVehicleDamageStatus.MonitoredVehicleID = VehicleID;
			}
		}
	}
}

//-----------------------------------------------------------
