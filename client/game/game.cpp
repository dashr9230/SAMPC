//----------------------------------------------------------
//
// SA:MP Multiplayer Modification For GTA:SA
// Copyright 2004-2005 SA:MP team
//
// Version: $Id: game.cpp,v 1.51 2006/05/08 17:35:55 kyeman Exp $
//
//----------------------------------------------------------

#include "../main.h"
#include "game.h"
#include "util.h"
#include "keystuff.h"
#include "aimstuff.h"

void GameInstallHooks();
bool ApplyPreGamePatches();
void ApplyInGamePatches();

//char *szGameTextMessage;

static bool bInputsDisabled = false;
static int iInputDisableWaitFrames=0;

typedef void (*DrawZone_t)(float *fPos, DWORD *dwColor, BYTE byteMenu);

//-----------------------------------------------------------

CGame::CGame()
{
	DWORD dwOldPropect;
	VirtualProtect((LPVOID)0x00401000, 0x004F2FFF,
		PAGE_EXECUTE_READWRITE, &dwOldPropect);

	m_pGameCamera = new CCamera();
	m_pGamePlayer = NULL;
	m_bCheckpointsEnabled = false;
	m_bRaceCheckpointsEnabled = false;
	m_dwRaceCheckpointHandle = NULL;
}

//-----------------------------------------------------------

CPlayerPed *CGame::NewPlayer(int iPlayerID, int iSkin, float fPosX, float fPosY,
							  float fPosZ, float fRotation, BYTE byteCreateMarker)
{
	CPlayerPed *pPlayerNew = new CPlayerPed(iPlayerID,iSkin,fPosX,fPosY,fPosZ,fRotation,byteCreateMarker);
	return pPlayerNew;
}

//-----------------------------------------------------------

CVehicle *CGame::NewVehicle(int iType, float fPosX, float fPosY,
							 float fPosZ, float fRotation, PCHAR szNumberPlate)
{
	CVehicle *pVehicleNew = new	CVehicle(iType,fPosX,fPosY,fPosZ,fRotation,szNumberPlate);
	return pVehicleNew;
}

//-----------------------------------------------------------

CObject *CGame::NewObject(int iModel, float fPosX, float fPosY,
							float fPosZ, VECTOR vecRot)
{
	CObject *pObjectNew = new CObject(iModel,fPosX,fPosY,fPosZ,vecRot);
	return pObjectNew;
}

//-----------------------------------------------------------

int CGame::GetWeaponModelIDFromWeapon(int iWeaponID)
{
	return GameGetWeaponModelIDFromWeaponID(iWeaponID);
}

//-----------------------------------------------------------

bool CGame::IsKeyPressed(int iKeyIdentifier)
{
	GTA_CONTROLSET * pControlSet = GameGetInternalKeys();

	if(pControlSet->wKeys1[iKeyIdentifier]) return true;

	return false;
}

//-----------------------------------------------------------

float CGame::FindGroundZForCoord(float x, float y, float z)
{
	float fGroundZ;
	ScriptCommand(&get_ground_z, x, y, z, &fGroundZ);
	return fGroundZ;
}

//-----------------------------------------------------------

BYTE byteGetKeyStateFunc[] = { 0xE8,0x46,0xF3,0xFE,0xFF };

void CGame::ProcessInputDisabling()
{
	if(bInputsDisabled) {
		//UnFuck(0x541DF5,5);
		memset((PVOID)0x541DF5,0x90,5);	// disable call	
		//GameResetInternalKeys(); // set keys to 0
	} else {
		if(!iInputDisableWaitFrames) {
			//UnFuck(0x541DF5,5);
			memcpy((PVOID)0x541DF5,byteGetKeyStateFunc,5);
			//GameResetInternalKeys(); // set keys to 0
		} else {
			iInputDisableWaitFrames--;
		}
	}
}

//-----------------------------------------------------------

void CGame::ToggleKeyInputsDisabled(bool bDisable)
{
	if(bDisable) {
		bInputsDisabled = true;
	} else {
		bInputsDisabled = false;
		iInputDisableWaitFrames = 2;
	}
}

//-----------------------------------------------------------

void CGame::InitGame()
{
	// Create a buffer for game text.
	//szGameTextMessage = (char*)malloc(256);

	// Init the keystate stuff.
	GameKeyStatesInit();

	// Init the aim stuff.
	GameAimSyncInit();

	// Init radar colors
	GameResetRadarColors();

	if(!ApplyPreGamePatches()) {
		MessageBox(0,
			"I can't determine your GTA version.\r\nSA-MP only supports GTA:SA v1.0 USA/EU",
			"Version Error",MB_OK | MB_ICONEXCLAMATION);
		ExitProcess(1);
	}
}

//-----------------------------------------------------------

void CGame::StartGame()
{		
	OutputDebugString("CGame::StartGame start");

	ApplyInGamePatches();

	// Install all hooks
	GameInstallHooks();

	// Setup scripting
	InitScripting();

	*(PDWORD)ADDR_ENTRY = 8;
	*(PBYTE)ADDR_GAME_STARTED = 1;
	*(PBYTE)ADDR_MENU = 0;
	*(PBYTE)ADDR_STARTGAME = 0;

	OutputDebugString("CGame::StartGame end");
}

//-----------------------------------------------------------

bool CGame::IsMenuActive()
{
	if(*(PDWORD)ADDR_MENU) return true;
	return false;
}

//-----------------------------------------------------------
// Return TRUE if the world has been loaded.

bool CGame::IsGameLoaded()
{
	if(!(*(PBYTE)ADDR_GAME_STARTED)) return true;
	return false;
}

//-----------------------------------------------------------

void CGame::RequestModel(int iModelID)
{
	/*
	_asm push 2
	_asm push iModelID
	_asm mov edx, 0x4087E0
	_asm call edx
	_asm add esp, 8*/

	ScriptCommand(&request_model,iModelID);
}

//-----------------------------------------------------------

void CGame::LoadRequestedModels()
{
	/*
	_asm push 0
	_asm mov edx, 0x40EA10
	_asm call edx
	_asm add esp, 4*/

	ScriptCommand(&load_requested_models);
}

//-----------------------------------------------------------

bool CGame::IsModelLoaded(int iModelID)
{
	return ScriptCommand(&is_model_available,iModelID);
}

//-----------------------------------------------------------

void CGame::RemoveModel(int iModelID)
{
	/*
	_asm push iModelID
	_asm mov edx, 0x4089A0
	_asm call edx
	_asm add esp, 4*/

	if (IsModelLoaded(iModelID))
		ScriptCommand(&release_model,iModelID);
}

//-----------------------------------------------------------

void CGame::SetWorldTime(int iHour, int iMinute)
{
	*(PBYTE)0xB70152 = (BYTE)iMinute;
	*(PBYTE)0xB70153 = (BYTE)iHour;
}

//-----------------------------------------------------------

void CGame::GetWorldTime(int* iHour, int* iMinute)
{
	*iMinute = *(PBYTE)0xB70152;
	*iHour = *(PBYTE)0xB70153;
}

//-----------------------------------------------------------

void CGame::ToggleThePassingOfTime(BYTE byteOnOff)
{
	//UnFuck(0x52CF10,1);

	if(byteOnOff) {
		*(PBYTE)0x52CF10 = 0x56; // push esi
	}
	else {
		*(PBYTE)0x52CF10 = 0xC3; // ret
	}
}

//-----------------------------------------------------------

void CGame::SetWorldWeather(int iWeatherID)
{
	*(DWORD*)(0xC81320) = iWeatherID;
	*(DWORD*)(0xC8131C) = iWeatherID;
}

//-----------------------------------------------------------

void CGame::DisplayHud(bool bDisp)
{
	if(bDisp) {
		*(BYTE*)ADDR_ENABLE_HUD = 1;
		ToggleRadar(1);
	} else {
		*(BYTE*)ADDR_ENABLE_HUD = 0;
		ToggleRadar(0);
	}
}
//-----------------------------------------------------------

BYTE CGame::IsHudEnabled()
{
	return *(BYTE*)ADDR_ENABLE_HUD;
}

//-----------------------------------------------------------

void CGame::SetFrameLimiterOn(bool bLimiter)
{

}

//-----------------------------------------------------------

void CGame::SetMaxStats()
{
	// driving stat
	_asm mov eax, 0x4399D0
	_asm call eax

	// weapon stats
	_asm mov eax, 0x439940
	_asm call eax
}

//-----------------------------------------------------------

void CGame::DisableTrainTraffic()
{
	ScriptCommand(&enable_train_traffic,0);
}

//-----------------------------------------------------------

void CGame::RefreshStreamingAt(float x, float y)
{
	ScriptCommand(&refresh_streaming_at,x,y);
}

//-----------------------------------------------------------

void CGame::RequestAnimation(char *szAnimFile)
{
	ScriptCommand(&request_animation, szAnimFile);
}

//-----------------------------------------------------------

int CGame::IsAnimationLoaded(char *szAnimFile)
{
	return ScriptCommand(&is_animation_loaded,szAnimFile);
}

//-----------------------------------------------------------

void CGame::ReleaseAnimation(char *szAnimFile)
{
	if (IsAnimationLoaded(szAnimFile))
		ScriptCommand(&release_animation,szAnimFile);
}

//-----------------------------------------------------------

void CGame::ToggleRadar(int iToggle)
{
	*(PBYTE)0xBAA3FB = (BYTE)!iToggle;
}

//-----------------------------------------------------------

void CGame::DisplayGameText(char *szStr,int iTime,int iSize)
{
	ScriptCommand(&text_clear_all);

	//strcpy(szGameTextMessage,szStr);

	_asm push iSize
	_asm push iTime
	//_asm push szGameTextMessage
	_asm push szStr
	_asm mov eax, 0x69F2B0
	_asm call eax
	_asm add esp, 12
}

//-----------------------------------------------------------

void CGame::PlaySound(int iSound, float fX, float fY, float fZ)
{
	ScriptCommand(&play_sound, fX, fY, fZ, iSound);
}

//-----------------------------------------------------------

void CGame::SetCheckpointInformation(VECTOR *pos, VECTOR *extent)
{
	memcpy(&m_vecCheckpointPos,pos,sizeof(VECTOR));
	memcpy(&m_vecCheckpointExtent,extent,sizeof(VECTOR));
	if(m_dwCheckpointMarker) {
		DisableMarker(m_dwCheckpointMarker);
		m_dwCheckpointMarker = NULL;
		//pChatWindow->AddDebugMessage("Disabled checkpoint marker");

		m_dwCheckpointMarker = CreateRadarMarkerIcon(0, m_vecCheckpointPos.X,
			m_vecCheckpointPos.Y, m_vecCheckpointPos.Z);
		/*pChatWindow->AddDebugMessage("Created marker icon '%X' at %f %f %f",
			m_dwCheckpointMarker, m_vecCheckpointPos.X, m_vecCheckpointPos.Y, m_vecCheckpointPos.Z);*/
	}
}

//-----------------------------------------------------------

void CGame::SetRaceCheckpointInformation(BYTE byteType, VECTOR *pos, VECTOR *next, float fSize) //VECTOR *extent)
{
	memcpy(&m_vecRaceCheckpointPos,pos,sizeof(VECTOR));
	memcpy(&m_vecRaceCheckpointNext,next,sizeof(VECTOR));
	m_fRaceCheckpointSize = fSize;
	m_byteRaceType = byteType;
	//memcpy(&m_vecCheckpointExtent,extent,sizeof(VECTOR));
	//pChatWindow->AddDebugMessage("Called");
	if(m_dwRaceCheckpointMarker)
	{
		DisableMarker(m_dwRaceCheckpointMarker);
		//pChatWindow->AddDebugMessage("1");
		m_dwRaceCheckpointMarker = NULL;
		//pChatWindow->AddDebugMessage("Disabled checkpoint marker");

		m_dwRaceCheckpointMarker = CreateRadarMarkerIcon(0, m_vecRaceCheckpointPos.X,
			m_vecRaceCheckpointPos.Y, m_vecRaceCheckpointPos.Z);
		/*pChatWindow->AddDebugMessage("Created marker icon '%X' at %f %f %f",
			m_dwCheckpointMarker, m_vecCheckpointPos.X, m_vecCheckpointPos.Y, m_vecCheckpointPos.Z);*/
	}
	MakeRaceCheckpoint();
}

//-----------------------------------------------------------

void CGame::MakeRaceCheckpoint()
{
	//DWORD dwCheckpoint;
	DisableRaceCheckpoint();
	//ScriptCommand(&create_checkpoint2, m_vecRaceCheckpointPos.X, m_vecRaceCheckpointPos.Y, m_vecRaceCheckpointPos.Z, &m_dwRaceCheckpointHandle);
	ScriptCommand(&create_racing_checkpoint, (int)m_byteRaceType,
				m_vecRaceCheckpointPos.X, m_vecRaceCheckpointPos.Y, m_vecRaceCheckpointPos.Z,
				m_vecRaceCheckpointNext.X, m_vecRaceCheckpointNext.Y, m_vecRaceCheckpointNext.Z,
				m_fRaceCheckpointSize, &m_dwRaceCheckpointHandle);
	/*pChatWindow->AddDebugMessage("Created checkpoint '%X' at %f %f %f",
			m_dwRaceCheckpointHandle, m_vecRaceCheckpointPos.X, m_vecRaceCheckpointPos.Y, m_vecRaceCheckpointPos.Z);
	pChatWindow->AddDebugMessage("Type: %d Size: %f", m_byteRaceType, m_fRaceCheckpointSize);*/
	m_bRaceCheckpointsEnabled = true;
}

void CGame::DisableRaceCheckpoint()
{
	if (m_dwRaceCheckpointHandle)
	{
		ScriptCommand(&destroy_racing_checkpoint, m_dwRaceCheckpointHandle);
		m_dwRaceCheckpointHandle = NULL;
	}
	m_bRaceCheckpointsEnabled = false;
}

//-----------------------------------------------------------

void CGame::UpdateCheckpoints()
{
	if(m_bCheckpointsEnabled) {
		CPlayerPed *pPlayerPed = this->FindPlayerPed();
		if(pPlayerPed) {
			ScriptCommand(&is_actor_near_point_3d,pPlayerPed->m_dwGTAId,
				m_vecCheckpointPos.X,m_vecCheckpointPos.Y,m_vecCheckpointPos.Z,
				m_vecCheckpointExtent.X,m_vecCheckpointExtent.Y,m_vecCheckpointExtent.Z,1);
			if (!m_dwCheckpointMarker)
			{
				m_dwCheckpointMarker = CreateRadarMarkerIcon(0, m_vecCheckpointPos.X,
					m_vecCheckpointPos.Y, m_vecCheckpointPos.Z);
				/*pChatWindow->AddDebugMessage("Created marker icon '%X' at %f %f %f",
					m_dwCheckpointMarker, m_vecCheckpointPos.X, m_vecCheckpointPos.Y, m_vecCheckpointPos.Z);*/
			}
		}
	}
	else if(m_dwCheckpointMarker) {
		DisableMarker(m_dwCheckpointMarker);
		m_dwCheckpointMarker = NULL;
		//pChatWindow->AddDebugMessage("Disabled checkpoint marker");
	}
	
	if(m_bRaceCheckpointsEnabled) {
		CPlayerPed *pPlayerPed = this->FindPlayerPed();
		if(pPlayerPed)
		{
			//MakeRaceCheckpoint();
			if (!m_dwRaceCheckpointMarker)
			{
				m_dwRaceCheckpointMarker = CreateRadarMarkerIcon(0, m_vecRaceCheckpointPos.X,
					m_vecRaceCheckpointPos.Y, m_vecRaceCheckpointPos.Z);
				/*pChatWindow->AddDebugMessage("Created marker icon '%X' at %f %f %f",
					m_dwCheckpointMarker, m_vecCheckpointPos.X, m_vecCheckpointPos.Y, m_vecCheckpointPos.Z);*/
			}
		}
	}
	else if(m_dwRaceCheckpointMarker) {
		DisableMarker(m_dwRaceCheckpointMarker);
		DisableRaceCheckpoint();
		m_dwRaceCheckpointMarker = NULL;
		//pChatWindow->AddDebugMessage("Disabled checkpoint marker");
	}
}


//-----------------------------------------------------------

DWORD CGame::CreateRadarMarkerIcon(int iMarkerType, float fX, float fY, float fZ, int iColor)
{
	DWORD dwMarkerID;
	ScriptCommand(&create_radar_marker_without_sphere, fX, fY, fZ, iMarkerType, &dwMarkerID);
	ScriptCommand(&set_marker_color,dwMarkerID,iColor);
	ScriptCommand(&show_on_radar,dwMarkerID,3);
	return dwMarkerID;
}

//-----------------------------------------------------------

void CGame::DisableMarker(DWORD dwMarkerID)
{
	ScriptCommand(&disable_marker, dwMarkerID);
}

//-----------------------------------------------------------
// Get the current active interior

BYTE CGame::GetActiveInterior()
{
	DWORD dwRet;
	ScriptCommand(&get_active_interior,&dwRet);
	return (BYTE)dwRet;
}

//-----------------------------------------------------------

extern float fFarClip;

void CGame::UpdateFarClippingPlane()
{
	PED_TYPE *pPlayerPed = GamePool_FindPlayerPed();

	if(pPlayerPed) {
		if(GetActiveInterior() == 0) {
			fFarClip = 1250.0f - (pPlayerPed->entity.mat->pos.Z * 2.0f);
			if(fFarClip < 700.0f) {
				fFarClip = 700.0f;
			}
		}
		else {
			fFarClip = 400.0f;
		}
	}
	else {
		fFarClip = 1250.0f;
	}
}

//-----------------------------------------------------------

void CGame::AddToLocalMoney(int iAmount)
{
	ScriptCommand(&add_to_player_money,0,iAmount);
}

//-----------------------------------------------------------

void CGame::ResetLocalMoney()
{
	int iMoney = GetLocalMoney();
	if(!iMoney) return;

	if(iMoney < 0) {
		AddToLocalMoney(abs(iMoney));
	} else {
		AddToLocalMoney(-(iMoney));
	}
}

//-----------------------------------------------------------

int CGame::GetLocalMoney()
{	
	return *(int *)0xB7CE50;
}

//-----------------------------------------------------------

const PCHAR CGame::GetWeaponName(int iWeaponID)
{
	switch(iWeaponID) { 
      case WEAPON_BRASSKNUCKLE: 
         return "Brass Knuckles"; 
      case WEAPON_GOLFCLUB: 
         return "Golf Club"; 
      case WEAPON_NITESTICK: 
         return "Nite Stick"; 
      case WEAPON_KNIFE: 
         return "Knife"; 
      case WEAPON_BAT: 
         return "Baseball Bat"; 
      case WEAPON_SHOVEL: 
         return "Shovel"; 
      case WEAPON_POOLSTICK: 
         return "Pool Cue"; 
      case WEAPON_KATANA: 
         return "Katana"; 
      case WEAPON_CHAINSAW: 
         return "Chainsaw"; 
      case WEAPON_DILDO: 
         return "Dildo"; 
      case WEAPON_DILDO2: 
         return "Dildo"; 
      case WEAPON_VIBRATOR: 
         return "Vibrator"; 
      case WEAPON_VIBRATOR2: 
         return "Vibrator"; 
      case WEAPON_FLOWER: 
         return "Flowers"; 
      case WEAPON_CANE: 
         return "Cane"; 
      case WEAPON_GRENADE: 
         return "Grenade"; 
      case WEAPON_TEARGAS: 
         return "Teargas";
	  case WEAPON_MOLOTOV: 
         return "Molotov";
      case WEAPON_COLT45: 
         return "Colt 45"; 
      case WEAPON_SILENCED: 
         return "Silenced Pistol"; 
      case WEAPON_DEAGLE: 
         return "Desert Eagle"; 
      case WEAPON_SHOTGUN: 
         return "Shotgun"; 
      case WEAPON_SAWEDOFF: 
         return "Sawn-off Shotgun"; 
      case WEAPON_SHOTGSPA: // wtf? 
         return "Combat Shotgun"; 
      case WEAPON_UZI: 
         return "UZI"; 
      case WEAPON_MP5: 
         return "MP5"; 
      case WEAPON_AK47: 
         return "AK47"; 
      case WEAPON_M4: 
         return "M4"; 
      case WEAPON_TEC9: 
         return "TEC9"; 
      case WEAPON_RIFLE: 
         return "Rifle"; 
      case WEAPON_SNIPER: 
         return "Sniper Rifle"; 
      case WEAPON_ROCKETLAUNCHER: 
         return "Rocket Launcher"; 
      case WEAPON_HEATSEEKER: 
         return "Heat Seaker"; 
      case WEAPON_FLAMETHROWER: 
         return "Flamethrower"; 
      case WEAPON_MINIGUN: 
         return "Minigun"; 
      case WEAPON_SATCHEL: 
         return "Satchel Explosives"; 
      case WEAPON_BOMB: 
         return "Bomb"; 
      case WEAPON_SPRAYCAN: 
         return "Spray Can"; 
      case WEAPON_FIREEXTINGUISHER: 
         return "Fire Extinguisher"; 
      case WEAPON_CAMERA: 
         return "Camera"; 
	  case WEAPON_NIGHT_VIS_GOGGLES:
		 return "Night vision Goggles";
	  case WEAPON_THERMAL_GOGGLES:
		 return "Thermal Goggles";
      case WEAPON_PARACHUTE: 
         return "Parachute"; 
      case WEAPON_VEHICLE: 
         return "Vehicle"; 
      case WEAPON_DROWN: 
         return "Drowned"; 
      case WEAPON_COLLISION: 
         return "Splat";
	}

	return "";
}

//-----------------------------------------------------------

DWORD CGame::CreatePickup(int iModel, int iType, float fX, float fY, float fZ)
{
	DWORD hnd;

	if(!IsModelLoaded(iModel)) {
		RequestModel(iModel);
		LoadRequestedModels();
		while(!IsModelLoaded(iModel)) Sleep(5);
	}

	ScriptCommand(&create_pickup,iModel,iType,fX,fY,fZ,&hnd);
	return hnd;
}

//-----------------------------------------------------------

DWORD CGame::CreateWeaponPickup(int iModel, DWORD dwAmmo, float fX, float fY, float fZ)
{
	DWORD hnd;

	if(!IsModelLoaded(iModel)) {
		RequestModel(iModel);
		LoadRequestedModels();
		while(!IsModelLoaded(iModel)) Sleep(5);
	}

	ScriptCommand(&create_pickup_with_ammo, iModel, 4, dwAmmo, fX, fY, fZ, &hnd);
	return hnd;
}

//-----------------------------------------------------------

DWORD CGame::GetD3DDevice()
{ 
	DWORD pdwD3DDev=0;

	if(iGtaVersion == GTASA_VERSION_USA10) {
		_asm mov edx, ADDR_RENDERWARE_GETD3D_USA10
		_asm call edx
		_asm mov pdwD3DDev, eax
	} 
	else if (iGtaVersion == GTASA_VERSION_EU10) {
		_asm mov edx, ADDR_RENDERWARE_GETD3D_EU10
		_asm call edx
		_asm mov pdwD3DDev, eax
	}

	return pdwD3DDev;	
}

//-----------------------------------------------------------
// DOESN'T CURRENTLY WORK

void CGame::RestartEverything()
{
	//*(PBYTE)ADDR_MENU = 1;
	*(PBYTE)ADDR_MENU2 = 1;
	*(PBYTE)ADDR_MENU3 = 1;

	//(PBYTE)ADDR_GAME_STARTED = 0;
	//*(PBYTE)ADDR_MENU = 1;

	OutputDebugString("ShutDownForRestart");
	_asm mov edx, 0x53C550 ; internal_CGame_ShutDownForRestart
	_asm call edx

	OutputDebugString("Timers stopped");
	_asm mov edx, 0x561AA0 ; internal_CTimer_Stop
	_asm call edx

	OutputDebugString("ReInitialise");
	_asm mov edx, 0x53C680 ; internal_CGame_InitialiseWhenRestarting
	_asm call edx

	*(PBYTE)ADDR_GAME_STARTED = 1;

}

//-----------------------------------------------------------

DWORD CGame::GetWeaponInfo(int iWeapon, int iUnk)
{
	DWORD dwRet;

	_asm push iUnk
	_asm push iWeapon
	_asm mov edx, 0x743C60
	_asm call edx
	_asm pop ecx
	_asm pop ecx
	_asm mov dwRet, eax

	return dwRet;
}

//----------------------------------------------------

void CGame::SetGravity(float fGravity)
{
	//UnFuck(0x863984, 4);
	*(float*)0x863984 = fGravity;
}

// ---------------------------------------------------

void CGame::SetWantedLevel(BYTE byteLevel)
{
	*(BYTE*)0x58DB60 = byteLevel;
}

//-----------------------------------------------------------

void CGame::SetGameTextCount(WORD wCount)
{
	*(WORD*)0xA44B68 = wCount;
}

//-----------------------------------------------------------

void CGame::DrawGangZone(float fPos[], DWORD dwColor)
{
	((DrawZone_t)0x5853D0)(fPos, &dwColor, *(BYTE*)ADDR_MENU);
}

//-----------------------------------------------------------

void CGame::EnableClock(BYTE byteClock)
{
	BYTE byteClockData[] = {'%', '0', '2', 'd', ':', '%', '0', '2', 'd', 0};
	//UnFuck(0x859A6C,10);
	if (byteClock)
	{
		ToggleThePassingOfTime(1);
		memcpy((PVOID)0x859A6C, byteClockData, 10);
	}
	else
	{
		ToggleThePassingOfTime(0);
		memset((PVOID)0x859A6C,0,10);
	}
}

//-----------------------------------------------------------

void CGame::EnableZoneNames(BYTE byteEnable)
{
	ScriptCommand(&enable_zone_names, byteEnable);
}

//-----------------------------------------------------------

void CGame::EnableStuntBonus(bool bEnable)
{
	//UnFuck(0xA4A474,4);
	*(DWORD*)0xA4A474 = (int)bEnable;
}

void CGame::SetDrunkLevel(float fLevel)
{
	((void(__cdecl*)(float))0x71D730)(fLevel);
}
//-----------------------------------------------------------

void CGame::DisableEnterExits(bool bDisable)
{
	DWORD pEnExPool = *(DWORD *)0x96A7D8;
	DWORD pEnExEntries = *(DWORD *)pEnExPool;
	
	int iNumEnEx=0;
	int x=0;

	_asm mov ecx, pEnExPool
	_asm mov eax, [ecx+8]
	_asm mov iNumEnEx, eax

	BYTE *pEnExPoolSlot;
	while(x!=iNumEnEx) {
		pEnExPoolSlot = (((BYTE *)pEnExEntries) + (60*x));
		if (bDisable)
		{
			_asm mov eax, pEnExPoolSlot
			_asm and word ptr[eax + 48], 0
		}
		else
		{
			_asm mov eax, pEnExPoolSlot
			_asm or word ptr[eax + 48], 4000h
		}
		x++;
	}   
}

//-----------------------------------------------------------

void CGame::SetWeaponSkill(unsigned char ucSkill, unsigned int uiLevel)
{
	*(float*)(0xB79380 + (ucSkill * 4 + 276)) = (float)uiLevel;
}

void CGame::SetMaxHealth(float fMax)
{
	*(float*)0xB793E0 = fMax;
}

void CGame::SetBlurLevel(unsigned char ucLevel)
{
	*(unsigned char*)0x8D5104 = ucLevel;
}

void CGame::SetGameSpeed(float fSpeed)
{
	*(float*)0xB7CB64 = fSpeed;
}

float CGame::GetGameSpeed()
{
	return *(float*)0xB7CB64;
}

void CGame::DisableCamera(bool bDisable)
{
	memset((void*)0xB73424, 0, 8);

	if (bDisable) {
		memcpy((void*)0x53F41F, "\x33\xC0\x0F\x84", 4); // skip
		//memset((void*)0x53F417, 0x90, 5);

		_asm mov edx, 0x541BD0 // reset mouse
		_asm call edx
		_asm mov edx, 0x541DD0 // CPad::Update()
		_asm call edx

		*(BYTE*)0x531140 = 0xC3; // ret (keyboard event process)
	} else {
		memcpy((void*)0x53F41F, "\x85\xC0\x0F\x8C", 4);
		//BYTE bOriginal[] = { 0xE8, 0xB4, 0x7A, 0x20, 0x00 };
		//memcpy((void*)0x53F417, bOriginal, sizeof(bOriginal));
		*(BYTE*)0x531140 = 0x83; // sub (keyboard event process)
		SetCursor(NULL);
	}
	
}
