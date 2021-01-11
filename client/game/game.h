//----------------------------------------------------------
//
// SA:MP Multiplayer Modification For GTA:SA
// Copyright 2004-2005 SA:MP team
//
// Version: $Id: game.h,v 1.26 2006/05/07 15:38:35 kyeman Exp $
//
//----------------------------------------------------------

#pragma once

#include "address.h"
#include "common.h"
#include "debug.h"
#include "vehicle.h"
#include "playerped.h"
#include "object.h"
#include "camera.h"
#include "scripting.h"
#include "menu.h"
#include "textdraw.h"
#include "actor.h"

#define NO_TEAM 255

typedef int (_cdecl *CWorld_ProcessLineOfSight_t)(
	VECTOR *vecShotOrigin,
	VECTOR *vecShotVector,
	VECTOR *colPoint,
	DWORD *pHitEntity,
	BOOL bCheckBuildings,
	BOOL bCheckVehicles,
	BOOL bCheckPeds,
	BOOL bCheckObjects,
	BOOL bCheckDummies,
	BOOL bSeeThroughStuff,
	BOOL bIgnoreSomeObjectsForCamera,
	void* pUnknownPtr
);

//-----------------------------------------------------------

class CGame
{
private:

	CCamera			*m_pGameCamera;
	CPlayerPed		*m_pGamePlayer;

	VECTOR			m_vecCheckpointPos;
	VECTOR			m_vecCheckpointExtent;
	bool			m_bCheckpointsEnabled;
	DWORD			m_dwCheckpointMarker;

	VECTOR			m_vecRaceCheckpointPos;
	VECTOR			m_vecRaceCheckpointNext;
	//VECTOR			m_vecRaceCheckpointExtent;
	float			m_fRaceCheckpointSize;
	BYTE			m_byteRaceType;
	bool			m_bRaceCheckpointsEnabled;
	DWORD			m_dwRaceCheckpointMarker;
	DWORD			m_dwRaceCheckpointHandle;
	bool			m_bMissionAudioLoaded;

public:
	bool m_bDisableVehMapIcons;
	bool m_bDisableInteriorAmbient;

	CPlayerPed *NewPlayer(int iPlayerID, int iSkin,float fPosX,float fPosY,float fPosZ,float fRotation,BYTE byteCreateMarker = 1);
	CVehicle *NewVehicle(int iType,float fPosX,float fPosY,float fPosZ,float fRotation, PCHAR szNumberPlate);
	CObject *NewObject(int iModel, float fPosX, float fPosY,float fPosZ, VECTOR vecRot);
	int		GetWeaponModelIDFromWeapon(int iWeaponID);
	bool	IsKeyPressed(int iKeyIdentifier);
	float	FindGroundZForCoord(float x, float y, float z);
	void	ToggleKeyInputsDisabled(bool bDisable);
	void	StartGame();
	void	InitGame();
	bool	IsMenuActive();
	bool	IsGameLoaded();
	static void RequestModel(int iModelID);
	static void LoadRequestedModels();
	static bool IsModelLoaded(int iModelID);
	static void RemoveModel(int iModelID);
	void	SetWorldTime(int iHour, int iMinute);
	void	GetWorldTime(int* iHour, int* iMinute);
	void	ToggleThePassingOfTime(BYTE byteOnOff);
	void	SetWorldWeather(int iWeatherID);
	void	DisplayHud(bool bDisp);
	BYTE	IsHudEnabled();
	void	SetFrameLimiterOn(bool bLimiter);
	void	SetMaxStats();
	void	DisableTrainTraffic();
	void	RefreshStreamingAt(float x, float y);
	static void RequestAnimation(char *szAnimFile);
	static int IsAnimationLoaded(char *szAnimFile);
	static void ReleaseAnimation(char *szAnimFile);
	void	ToggleRadar(int iToggle);
	void	DisplayGameText(char *szStr,int iTime,int iSize);
	static void PlayAmbientSound(int iSound);
	static void StopAmbientSound();
	void	PlaySoundFX(int iSound, float fX, float fY, float fZ);
	void	SetGravity(float fGravity);
	void	EnableClock(BYTE byteClock);
	void	EnableZoneNames(BYTE byteEnable);
	void	SetWantedLevel(BYTE byteLevel);
	void	SetGameTextCount(WORD wCount);
	void	DrawGangZone(float fPos[], DWORD dwColor);
	void    EnableStuntBonus(bool bEnable);
	void	SetDrunkLevel(float fLevel);
	void   UpdateCheckpoints();
	void   ToggleCheckpoints(bool bEnabled){ m_bCheckpointsEnabled = bEnabled; };
	void   SetCheckpointInformation(VECTOR *pos, VECTOR *extent);
	
	void	MakeRaceCheckpoint();
	void	DisableRaceCheckpoint();
	void   ToggleRaceCheckpoints(bool bEnabled){ m_bRaceCheckpointsEnabled = bEnabled; };
	void   SetRaceCheckpointInformation(BYTE byteType, VECTOR *pos, VECTOR *next, float fSize);
	
	DWORD	CreateRadarMarkerIcon(int iMarkerType, float fX, float fY, float fZ, int iColor = 201, int iStyle = 0);
	void	DisableMarker(DWORD dwMarkerID);

	void   AddToLocalMoney(int iAmount);
	void   ResetLocalMoney();
	int	   GetLocalMoney();

	BYTE   GetActiveInterior();
	void   UpdateFarClippingPlane();
		
	DWORD	GetD3DDevice();

	void	SetD3DDevice(DWORD pD3DDevice) { *(DWORD *)ADDR_ID3D9DEVICE = pD3DDevice; };
	DWORD	GetD3D() { return *(DWORD *)ADDR_ID3D9DEVICE; };
	void	SetD3D(DWORD pD3D) {	*(DWORD *)ADDR_ID3D9 = pD3D; };
	HWND	GetMainWindowHwnd() { return *(HWND *)ADDR_HWND; };

	void	RestartEverything();
	void	ProcessInputDisabling();

	//-----------------------------------------------------------

	CCamera     *GetCamera() {	return m_pGameCamera; };

	void ClearScanCodes() {
		_asm mov eax, 0x563470
		_asm call eax
	};
		
	CPlayerPed* FindPlayerPed();

	DWORD CreatePickup(int iModel, int iType, float fX, float fY, float fZ);
	DWORD CreateWeaponPickup(int iModel, DWORD dwAmmo, float fX, float fY, float fZ);

	int GetScreenWidth() { return *(int*)0xC17044; };
	int GetScreenHeight() { return *(int*)0xC17048; };
	float GetHudVertScale() { return *(float *)0x859524; };
	float GetHudHorizScale() { return *(float *)0x859520; };

	void SetWeaponSkill(unsigned char ucSkill, unsigned int uiLevel);
	DWORD GetWeaponInfo(int iWeapon, int iUnk);
	void DisableEnterExits(bool bDisable = true);

	void SetMaxHealth(float fMax);
	void SetBlurLevel(unsigned char ucLevel);

	static void StartRadio(unsigned int uiStation);
	static void StopRadio();
	static float GetRadioVolume();

	void SetGameSpeed(float fSpeed);
	float GetGameSpeed();

	CGame();
	//~CGame() {};

	void DisableCamera(bool bDisable);

	static float GetFPS();
	static float GetAspectRatio();

	static void PlayCrimeReport(int iCrimeID, VECTOR* vecPos, int iVehicleType, int iVehicleCol1);

	void FrameLimiter();
};

//-----------------------------------------------------------
