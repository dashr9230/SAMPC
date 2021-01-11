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

char *szGameTextMessage;

static bool bInputsDisabled = false;
static int iInputDisableWaitFrames=0;

PED_TYPE CrimeReportPed;

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
	m_byteRaceType = 0;
	m_dwCheckpointMarker = 0;
	m_dwRaceCheckpointMarker = 0;
	m_fRaceCheckpointSize = 0.0f;
	m_bDisableVehMapIcons = false;
	m_bMissionAudioLoaded = false;
	m_bDisableInteriorAmbient = false;
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

CPlayerPed* CGame::FindPlayerPed()
{
	if (m_pGamePlayer == NULL)
		m_pGamePlayer = new CPlayerPed();

	return m_pGamePlayer;
};

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
	szGameTextMessage = (char*)malloc(MAX_GAME_TEXT + 1);

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

// OPC: 0247 (0047EF90) request_model
void CGame::RequestModel(int iModelID)
{
	if (iModelID < 0)
		return;

	DWORD dwFunc = 0x4087E0;
	_asm {
		push 2
		push iModelID
		call dwFunc
		add esp, 8
	}
}

// OPC: 038B (00483DDB) load_requested_models
void CGame::LoadRequestedModels()
{
	DWORD dwFunc = 0x40EA10;
	_asm {
		push 0
		call dwFunc
		add esp, 4
	}
}

// OPC: 0248 (0047EFDE) is_model_available
// [0x8E4CC0 + 0x10] g_struModelInfo.iState (0=not loaded, 1=loaded, 2=requested)
bool CGame::IsModelLoaded(int iModelID)
{
	if (iModelID < 0)
		return false;

	DWORD dwFunc = 0x4044C0;
	bool bRet = false;
	_asm {
		push iModelID
		call dwFunc
		mov bRet, al
		add esp, 4
	}
	return bRet;
}

// OPC: 0249 (0047F03E) release_model
// sub_4089A0() has internal checks, so IsModelLoaded is not needed here
void CGame::RemoveModel(int iModelID)
{
	if (iModelID < 0)
		return;

	DWORD dwFunc = 0x4089A0;
	_asm {
		push iModelID
		call dwFunc
		add esp, 4
	}
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

// OPC: 04ED (0048C351) load_animation
void CGame::RequestAnimation(char *szAnimFile)
{
	DWORD dwFunc1 = 0x4D3990;
	DWORD dwFunc2 = 0x4087E0;
	_asm {
		push szAnimFile
		call dwFunc1
		test eax, eax
		jz sub_4D3990_failed
		mov edi, eax
		lea eax, [edi+63E7h]
		push 4
		push eax
		call dwFunc2
		add esp, 8
	sub_4D3990_failed:
		add esp, 4
	}
}

// OPC: 04EE (0048C391) is_animation_loaded
int CGame::IsAnimationLoaded(char *szAnimFile)
{
	DWORD dwFunc = 0x4D3940;
	char cRet = -1;
	_asm {
		push szAnimFile
		call dwFunc
		test eax, eax
		jz sub_4D3940_failed
		mov cl, [eax+10h]
		mov cRet, cl
	sub_4D3940_failed:
		add esp, 4	
	}
	return cRet;
}

// OPC: 04EF (0048C3D5) release_animation
// IsAnimationLoaded not needed here. sub_48B570() has internal check for loaded animation.
void CGame::ReleaseAnimation(char *szAnimFile)
{
	DWORD dwFunc1 = 0x4D3990;
	DWORD dwFunc2 = 0x48B570;
	int iModelId = 0;
	_asm {
		push szAnimFile
		call dwFunc1
		add esp, 4
		test eax, eax
		jz sub_4D3990_failed
		mov edi, eax
		push edi
		call dwFunc2
		add esp, 4
	sub_4D3990_failed:
		add esp, 4
	}
}

//-----------------------------------------------------------

void CGame::ToggleRadar(int iToggle)
{
	*(PBYTE)0xBAA3FB = (BYTE)!iToggle;
}

//-----------------------------------------------------------

void CGame::DisplayGameText(char *szStr,int iTime,int iSize)
{
	if (szGameTextMessage == NULL)
	{
		assert(0);
		return;
	}

	ScriptCommand(&text_clear_all);
	
	strcpy_s(szGameTextMessage, MAX_GAME_TEXT, szStr);

	_asm push iSize
	_asm push iTime
	_asm push szGameTextMessage
	_asm mov eax, 0x69F2B0
	_asm call eax
	_asm add esp, 12
}

void CGame::PlayAmbientSound(int iSound)
{
	DWORD dwFunc = 0x4D6D50;
	DWORD dwThis = 0x8AC15C;
	_asm {
		push iSound
		mov ecx, dwThis
		call dwFunc
		add esp, 4
	};
}

void CGame::StopAmbientSound()
{
	DWORD dwFunc = 0x4D6D60;
	DWORD dwThis = 0x8AC15C;
	_asm {
		mov ecx, dwThis
		call dwFunc
	}
}

//-----------------------------------------------------------

void CGame::PlaySoundFX(int iSound, float fX, float fY, float fZ)
{
	if (iSound) {
		if (iSound == 1) {
			m_bDisableInteriorAmbient = true;
		} else if (iSound >= 1000) {
			if (iSound >= 2000) {
				ScriptCommand(&clear_mission_audio, 1);
				ScriptCommand(&load_mission_audio, 1, iSound);
				ScriptCommand(&set_mission_audio_position, 1, fX, fY, fZ);
				m_bMissionAudioLoaded = true;
			}
			else {
				ScriptCommand(&play_sound, fX, fY, fZ, iSound);
			}
		} else {
			CGame::PlayAmbientSound(iSound);
		}
	} else {
		if (m_bMissionAudioLoaded) {
			ScriptCommand(&clear_mission_audio, 1);
			m_bMissionAudioLoaded = false;
		}
		CGame::StopAmbientSound();
		m_bDisableInteriorAmbient = false;
	}
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

DWORD CGame::CreateRadarMarkerIcon(int iMarkerType, float fX, float fY, float fZ, int iColor, int iStyle)
{
	DWORD dwMarkerID = 0;
	switch (iStyle) {
	case 1:
		ScriptCommand(&add_sprite_blip_for_coord, fX, fY, fZ, iMarkerType, &dwMarkerID);
		break;
	case 2:
		ScriptCommand(&create_radar_marker_icon, fX, fY, fZ, iMarkerType, &dwMarkerID);
		break;
	case 3:
		ScriptCommand(&create_icon_marker_sphere, fX, fY, fZ, iMarkerType, &dwMarkerID);
		break;
	default:
		ScriptCommand(&create_radar_marker_without_sphere, fX, fY, fZ, iMarkerType, &dwMarkerID);
	}
	if (dwMarkerID) {
		ScriptCommand(&set_marker_color, dwMarkerID, iColor);
		ScriptCommand(&show_on_radar, dwMarkerID, (iColor >= 1004) ? 3 : 2);
	}
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

void CGame::StartRadio(unsigned int uiStation)
{
	DWORD dwThis = 0xB6BC90;
	DWORD dwFunc = 0x507DC0;
	_asm
	{
		push 0
		push uiStation
		mov ecx, dwThis
		call dwFunc
	}
}

void CGame::StopRadio()
{
	DWORD dwThis = 0xB6BC90;
	DWORD dwFunc = 0x506F70;
	_asm
	{
		push 0
		push 0
		mov ecx, dwThis
		call dwFunc
	}
}

float CGame::GetRadioVolume()
{
	// 0xBA6748 + 0x50 (CMenuRenderer.byteRadioVolume) [Range: 0-64]
	// Game multiplies byteRadioVolume with 0.015625 to get in range with audio engine's volume control
	return *(float*)0xB5F8B8 + 0x410; // [Range: 0.0-1.0]
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

float CGame::GetFPS()
{
	return *(float*)0xB7CB50;
}

float CGame::GetAspectRatio()
{
	return *(float*)0xC3EFA4;
}
/*
	0x10150B78 unk_10150B78 (block starts)
	0x10151104 dword_10151104
	0x10150FE4 dword_10150FE4
*/
void CGame::PlayCrimeReport(int iCrimeID, VECTOR* vecPos, /*bool bNeedVehicle,*/
	int iVehicleType, int iVehicleCol1)
{
	PED_TYPE* pPed;
	CVehicle* pVehicle = NULL;
	DWORD dwPlayerInfoOffs;

	memset(&CrimeReportPed, 0, sizeof(PED_TYPE));

	//UnFuck(0x4E7529, 39);
	memset((void*)0x4E7529, 0x90, 39);

	pPed = GamePool_FindPlayerPed();

	if (/*bNeedVehicle &&*/ iVehicleType >= 400 && iVehicleType < 612)
	{
		pVehicle = new CVehicle(iVehicleType, 0.0f, 0.0f, 0.0f, 0.0f);
		if (pVehicle)
		{
			pVehicle->Add();
			pVehicle->SetColor(iVehicleCol1, -1);

			CrimeReportPed.pVehicle = (DWORD)pVehicle->m_pVehicle;
			CrimeReportPed.dwStateFlags |= 256;
		}
	}

	if (pPed)
	{
		dwPlayerInfoOffs = pPed->dwPlayerInfoOffset;

		_asm
		{
			mov ecx, dwPlayerInfoOffs
			mov eax, [ecx]
			lea ecx, [eax+21Ch]
			push vecPos
			push iCrimeID
			push 164
			mov edx, 0x4E71E0
			call edx
		}
	}

	if (pVehicle)
		delete pVehicle;
}

void CGame::FrameLimiter()
{
	int iFrame = pConfig->GetInt("fpslimit");
	if (iFrame >= 20 && iFrame <= 100)
		*(PDWORD)0xC1704C = (DWORD)(iFrame * 1.375f);
	else
		*(PDWORD)0xC1704C = (DWORD)(50 * 1.375f);
}