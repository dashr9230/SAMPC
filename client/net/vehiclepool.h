//----------------------------------------------------------
//
// SA:MP Multiplayer Modification For GTA:SA
// Copyright 2004-2005 SA:MP team
//
// Version: $Id: vehiclepool.h,v 1.10 2006/05/07 17:32:29 kyeman Exp $
//
//----------------------------------------------------------

#pragma once

typedef struct _VEHICLE_SPAWN_INFO
{
	int iVehicleType;
	VECTOR vecPos;
	float fRotation;
	int iColor1;
	int iColor2;
	int iObjective;
	int iDoorsLocked;
	int iInterior;
} VEHICLE_SPAWN_INFO;

//----------------------------------------------------

class CVehiclePool
{
public:
	
	bool				m_bVehicleSlotState[MAX_VEHICLES];
	CVehicle			*m_pVehicles[MAX_VEHICLES];

	bool				m_bIsActive[MAX_VEHICLES];
	bool				m_bIsWasted[MAX_VEHICLES];
	VEHICLE_SPAWN_INFO	m_SpawnInfo[MAX_VEHICLES];
	VEHICLE_OPEN_CLOSE_FLAG m_Windows[MAX_VEHICLES];
	VEHICLE_OPEN_CLOSE_FLAG m_Doors[MAX_VEHICLES];

	int					m_iRespawnDelay[MAX_VEHICLES];
	int					m_iVirtualWorld[MAX_VEHICLES];
	CHAR				m_charNumberPlate[MAX_VEHICLES][9];
	bool				m_bHasSiren[MAX_VEHICLES];

	CVehiclePool();
	~CVehiclePool();

	bool New(VEHICLEID VehicleID, int iVehicleType,
			 VECTOR * vecPos, float fRotation,
			 int iColor1, int iColor2,
			 VECTOR * vecSpawnPos, float fSpawnRotation,
			 int iInterior, PCHAR szNumberPlate);

	bool Delete(VEHICLEID VehicleID);	
	
	// Retrieve a vehicle
	CVehicle* GetAt(VEHICLEID VehicleID) {
		if(VehicleID >= MAX_VEHICLES || !m_bVehicleSlotState[VehicleID]) { return NULL; }
		return m_pVehicles[VehicleID];
	};

	// Find out if the slot is inuse.
	bool GetSlotState(VEHICLEID VehicleID) {
		if(VehicleID >= MAX_VEHICLES) { return false; }
		return m_bVehicleSlotState[VehicleID];
	};

	bool Spawn( VEHICLEID VehicleID, int iVehicleType,
				VECTOR * vecPos, float fRotation,
				int iColor1, int iColor2, int iInterior, PCHAR szNumberPlate, 
				int iObjective = 0, int iDoorsLocked = 0 );

	void ProcessForVirtualWorld(VEHICLEID vehicleId, int iPlayerWorld);
	void Process();
	
	void NotifyVehicleDeath(VEHICLEID VehicleID);

	int FindNearestToLocalPlayerPed();

	void AssignSpecialParamsToVehicle(VEHICLEID VehicleID, BYTE byteObjective, BYTE byteDoorsLocked);
	
	int GetVehicleVirtualWorld(VEHICLEID VehicleID) const {
		if (VehicleID >= MAX_VEHICLES) { return 0; }
		return m_iVirtualWorld[VehicleID];		
	};
	
	void SetVehicleVirtualWorld(VEHICLEID VehicleID, int iVirtualWorld) {
		if (VehicleID >= MAX_VEHICLES) return;
		m_iVirtualWorld[VehicleID] = iVirtualWorld;
	};
	
	void SetForRespawn(VEHICLEID VehicleID, int iRespawnDelay = 1);
	void LinkToInterior(VEHICLEID VehicleID, int iInterior);

	VEHICLEID FindIDFromGtaPtr(VEHICLE_TYPE * pGtaVehicle);
	int FindGtaIDFromID(int iID);
	int FindGtaIDFromGtaPtr(VEHICLE_TYPE * pGtaVehicle);
};

//----------------------------------------------------
