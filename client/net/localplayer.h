//----------------------------------------------------------
//
// SA:MP Multiplayer Modification For GTA:SA
// Copyright 2004-2005 SA:MP team
//
// Version: $Id: localplayer.h,v 1.27 2006/05/08 17:35:55 kyeman Exp $
//
//----------------------------------------------------------

#pragma once

enum eWeaponState
{
	WS_NO_BULLETS = 0,
	WS_LAST_BULLET = 1,
	WS_MORE_BULLETS = 2,
	WS_RELOADING = 3,
};

//----------------------------------------------------------

class CLocalPlayer
{
public:
	CPlayerPed				*m_pPlayerPed;
	char					m_szName[MAX_PLAYER_NAME];
	bool					m_bIsActive;
	bool					m_bIsWasted;
	bool					m_bWantsAnotherClass;
	int						m_iSelectedClass;
	bool					m_bWaitingForSpawnRequestReply;
	bool					m_bControlsVisible;
	int						m_iVirtualWorld;
	int						m_iScore;
	unsigned short			m_usPing;

	BYTE					m_byteSpectateMode;
	BYTE					m_byteSpectateType;
	DWORD					m_SpectateID; // Vehicle or player id
	bool					m_bSpectateProcessed;
	VEHICLEID				m_CurrentVehicle;
	VEHICLEID				m_LastVehicle;
	int						m_iDisplayZoneTick;
	
	BYTE					m_byteLastWeapon[13];
	DWORD					m_dwLastAmmo[13];

	short m_sNormalOnfootRate;
	short m_sNormalIncarRate;
private:

	PLAYER_SPAWN_INFO		m_SpawnInfo;
	bool					m_bHasSpawnInfo;
	ULONG					m_ulThisSyncFrame;
	ULONG					m_ulLastSyncFrame;
	bool					m_bPassengerDriveByMode;
	BYTE					m_byteCurInterior;
	bool					m_bInRCMode;

	// SPAWNING STUFF
	bool					m_bAllowedClass;
	DWORD					m_dwLastSpawnSelectionTick;// delays left and right selection
	DWORD					m_dwInitialSelectionTick;// delays initial selection
	bool					m_bIsSpectating;
	BYTE					m_byteTeam;

	bool					m_bSurfingMode;
	VECTOR					m_vecLockedSurfingOffsets;
	VEHICLEID				m_SurfingID;

	WORD					m_wLastKeys;
	DWORD					m_dwLastSendTick;
	DWORD					m_dwLastSendSpecTick;
	DWORD					m_dwLastAimSendTick;
	DWORD					m_dwLastStatsUpdateTick;
	DWORD					m_dwLastHeadUpdate;

	struct
	{
		VEHICLEID MonitoredVehicleID;
		int iPanels;
		int iDoors;
		unsigned char ucLights;
		unsigned char ucWheels;
	} m_iVehicleDamageStatus;

public:

	CLocalPlayer();
	~CLocalPlayer(){};

	void SetName(const char* szName);
	const char* GetName();

	bool IsActive() { return m_bIsActive; };
	bool IsWasted() { return m_bIsWasted; };

	void HandlePassengerEntry();
	bool Process();
	bool DestroyPlayer();
	
	BYTE GetSpecialAction();
	void ApplySpecialAction(BYTE byteSpecialAction);

	void UpdateSurfing();

	void SendOnFootFullSyncData();
	void SendInCarFullSyncData();
	void SendPassengerFullSyncData();
	void SendAimSyncData();
	void ResetAllSyncAttributes();

	void SendVehicleDamageStatus(VEHICLEID VehicleID);

	int  GetOptimumInCarSendRate(int iPlayersEffected);
	int  GetOptimumOnFootSendRate(int iPlayersEffected);
	
	void SendWastedNotification();
	
	void RequestClass(int iClass);
	void RequestSpawn();

	void SetSpawnInfo(PLAYER_SPAWN_INFO *pSpawn);

	bool Spawn();

	CPlayerPed * GetPlayerPed() { return m_pPlayerPed; };

	void Say(PCHAR szText);

	void SendExitVehicleNotification(VEHICLEID VehicleID);
	void SendEnterVehicleNotification(VEHICLEID VehicleID,bool bPassenger);
	
	void SetPlayerColor(DWORD dwColor);
	DWORD GetPlayerColorAsRGBA();
	DWORD GetPlayerColorAsARGB();
	void ProcessOnFootWorldBounds();
	void ProcessInCarWorldBounds();

	void SendStatsUpdate();
	void UpdateRemoteInterior(BYTE byteInterior);

	void HandleClassSelectionOutcome(bool bOutcome);
	void HandleClassSelection();
	void ProcessClassSelection(int iControlID = 0);
	
	void CheckWeapons();
	void SetVirtualWorld(int iWorld) { m_iVirtualWorld = iWorld; };
	int GetVirtualWorld() const { return m_iVirtualWorld; };

	void ToggleSpectating(bool bToggle);
	void SpectateVehicle(VEHICLEID VehicleID);
	void SpectatePlayer(BYTE bytePlayerID);
	void ProcessSpectating();
	bool IsSpectating() { return m_bIsSpectating; };
	void ReturnToClassSelection() { m_bWantsAnotherClass = true; };

	BYTE GetTeam() { return m_byteTeam; };
	void SetTeam(BYTE byteTeam) { m_byteTeam = byteTeam; };

	bool IsInRCMode() { return m_bInRCMode; };

	int DetermineNumberOfPlayersInLocalRange();
	
	void SendFPS();
};

//----------------------------------------------------------
