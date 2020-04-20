/*

	SA:MP Multiplayer Modification
	Copyright 2004-2005 SA:MP Team

	file:
		scrcore.cpp
	desc:
		Scripting custom functions

    Version: $Id: scrcustom.cpp,v 1.60 2006/05/20 08:28:04 kyeman Exp $

*/

#include "main.h"

char* GetScriptName(AMX* amx);

#define CHECK_PARAMS(amx,fn,c) \
	{ \
		if ((params[0]) != ((c) * (sizeof(cell)))) \
		{ \
			logprintf("[%s.amx] " fn ": Bad parameter count (Count is %d, Should be %d)", \
				(GetScriptName(amx)), params[0] / sizeof(cell), c); \
			return 0; \
		} \
	}

#define CHECK_PARAMS_BETWEEN(amx,fn,l,h) \
	{ \
		if ((params[0]) < ((l) * sizeof(cell))) { \
			logprintf("[%s.amx] " fn ": Bad parameter count (%d < %d)", \
				(GetScriptName(amx)), ((params[0]) / (sizeof(cell))), (l)); \
			return 0; \
		} else if ((params[0]) > ((h) * (sizeof(cell)))) { \
			logprintf("[%s.amx] " fn ": Bad parameter count (%d > %d)", \
				(GetScriptName(amx)), ((params[0]) / (sizeof(cell))), (h)); \
			return 0; \
		} \
	}

#define DEFINE_NATIVE(func) {#func, n_##func}

char* format_amxstring(AMX *amx, cell *params, int parm, int &len);
int set_amxstring(AMX *amx,cell amx_addr,const char *source,int max);
bool ContainsInvalidNickChars(char* szString);

extern BOOL bGameModeFinished;
extern CNetGame* pNetGame;

//----------------------------------------------------------------------------------

static cell AMX_NATIVE_CALL n_GameModeExit(AMX *amx, cell *params)
{
	if(pNetGame->SetNextScriptFile(NULL)) {
		bGameModeFinished = TRUE;
	} else {
		logprintf("The gamemode finished and I couldn't start another script.");
		fcloseall();
		exit(1);
	}
	return 0;
}


//----------------------------------------------------------------------------------

static cell AMX_NATIVE_CALL n_SetGameModeText(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "SetGameModeText", 1);

	char* szGameModeText;
	amx_StrParam(amx, params[1], szGameModeText);
	pConsole->SetStringVariable("gamemodetext", szGameModeText);

	return 0;
}

//----------------------------------------------------------------------------------

static cell AMX_NATIVE_CALL n_SetTeamCount(AMX *amx, cell *params)
{
	return 0;
}

//----------------------------------------------------------------------------------

static cell AMX_NATIVE_CALL n_AddPlayerClass(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "AddPlayerClass", 11);
	PLAYER_SPAWN_INFO Spawn;

	Spawn.byteTeam = 255; // Auto team assignment for the old AddPlayerClass
	Spawn.iSkin = (int)params[1];
	Spawn.vecPos.X = amx_ctof(params[2]);
	Spawn.vecPos.Y = amx_ctof(params[3]);
	Spawn.vecPos.Z = amx_ctof(params[4]);
	Spawn.fRotation = amx_ctof(params[5]);

	// WEAPONS 
	Spawn.iSpawnWeapons[0] = (int)params[6];
	Spawn.iSpawnWeaponsAmmo[0] = (int)params[7];
	Spawn.iSpawnWeapons[1] = (int)params[8];
	Spawn.iSpawnWeaponsAmmo[1] = (int)params[9];
	Spawn.iSpawnWeapons[2] = (int)params[10];
	Spawn.iSpawnWeaponsAmmo[2] = (int)params[11];

	return pNetGame->AddSpawn(&Spawn);
}

//----------------------------------------------------------------------------------

static cell AMX_NATIVE_CALL n_AddPlayerClassEx(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "AddPlayerClassEx", 12);
	PLAYER_SPAWN_INFO Spawn;

	// BASE INFO
	Spawn.byteTeam = (BYTE)params[1];
	Spawn.iSkin = (int)params[2];
	Spawn.vecPos.X = amx_ctof(params[3]);
	Spawn.vecPos.Y = amx_ctof(params[4]);
	Spawn.vecPos.Z = amx_ctof(params[5]);
	Spawn.fRotation = amx_ctof(params[6]);

	// WEAPONS 
	Spawn.iSpawnWeapons[0] = (int)params[7];
	Spawn.iSpawnWeaponsAmmo[0] = (int)params[8];
	Spawn.iSpawnWeapons[1] = (int)params[9];
	Spawn.iSpawnWeaponsAmmo[1] = (int)params[10];
	Spawn.iSpawnWeapons[2] = (int)params[11];
	Spawn.iSpawnWeaponsAmmo[2] = (int)params[12];

	return pNetGame->AddSpawn(&Spawn);
}

//----------------------------------------------------------------------------------

static cell AMX_NATIVE_CALL n_AddStaticVehicle(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "AddStaticVehicle", 7);

	VECTOR vecPos;
	vecPos.X = amx_ctof(params[2]);
	vecPos.Y = amx_ctof(params[3]);
	vecPos.Z = amx_ctof(params[4]);

	VEHICLEID ret = pNetGame->GetVehiclePool()->New((int)params[1], &vecPos, amx_ctof(params[5]),
		(int)params[6], (int)params[7], 120000);

	return ret;
}

//----------------------------------------------------------------------------------

static cell AMX_NATIVE_CALL n_AddStaticVehicleEx(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "AddStaticVehicleEx", 8);

	VECTOR vecPos;
	vecPos.X = amx_ctof(params[2]);
	vecPos.Y = amx_ctof(params[3]);
	vecPos.Z = amx_ctof(params[4]);

	VEHICLEID ret = pNetGame->GetVehiclePool()->New((int)params[1], &vecPos, amx_ctof(params[5]),
		(int)params[6], (int)params[7], ((int)params[8]) * 1000);

	return ret;
}

//----------------------------------------------------------------------------------

static cell AMX_NATIVE_CALL n_SetVehicleToRespawn(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "SetVehicleToRespawn", 1);

	CVehicle* pVehicle;
	pVehicle = pNetGame->GetVehiclePool()->GetAt((VEHICLEID)params[1]);

	if (pVehicle != NULL) {
		pVehicle->Respawn();
		RakNet::BitStream bsVehicle;
		bsVehicle.Write((VEHICLEID)params[1]);
		pNetGame->GetRakServer()->RPC(RPC_ScrRespawnVehicle , &bsVehicle, HIGH_PRIORITY, 
			RELIABLE, 0, UNASSIGNED_PLAYER_ID, true, false);
		return 1;
	}
	
	return 0;
}

//----------------------------------------------------------------------------------
// native LinkVehicleToInterior
static cell AMX_NATIVE_CALL n_LinkVehicleToInterior(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "LinkVehicleToInterior", 2);
	CVehicle* pVehicle;
	pVehicle = pNetGame->GetVehiclePool()->GetAt((VEHICLEID)params[1]);

	if (pVehicle != NULL)
	{
		pVehicle->SetVehicleInterior(params[2]);
		RakNet::BitStream bsData;
		bsData.Write((VEHICLEID)params[1]);
		bsData.Write((BYTE)params[2]);

		RakServerInterface* pRak = pNetGame->GetRakServer();
		pRak->RPC(RPC_ScrLinkVehicle , &bsData, HIGH_PRIORITY, 
			RELIABLE, 0, UNASSIGNED_PLAYER_ID, true, false);
		return 1;
	}
	return 0;
}

//----------------------------------------------------------------------------------
// native AddVehicleComponent(vehicleid, componentid);
static cell AMX_NATIVE_CALL n_AddVehicleComponent(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "AddVehicleComponent", 2);

	CVehicle* pVehicle = pNetGame->GetVehiclePool()->GetAt((VEHICLEID)params[1]);
	if (!pVehicle)
		return 0;

	RakNet::BitStream bsData;
	bsData.Write((BYTE)INVALID_PLAYER_ID);
	bsData.Write((int)EVENT_TYPE_CARCOMPONENT);
	bsData.Write((DWORD)params[1]);
	bsData.Write((DWORD)params[2]);
	bsData.Write((DWORD)0);

	RakServerInterface* pRak = pNetGame->GetRakServer();
	pRak->RPC(RPC_ScmEvent , &bsData, HIGH_PRIORITY, 
		RELIABLE, 0, UNASSIGNED_PLAYER_ID, true, false);

	int iCompId = Utils::GetTypeByComponentId(params[2]);
	if (iCompId == -1)
		return 0;

	pVehicle->m_CarModInfo.ucCarMod[iCompId] = (unsigned char)(params[2] - 1000);

	return 1;
}

//----------------------------------------------------------------------------------
// native RemoveVehicleComponent(vehicleid, componentid);
static cell AMX_NATIVE_CALL n_RemoveVehicleComponent(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "RemoveVehicleComponent", 2);

	CVehicle* pVehicle = pNetGame->GetVehiclePool()->GetAt((VEHICLEID)params[1]);
	if (!pVehicle)
		return 0;

	RakNet::BitStream bsData;
	bsData.Write((VEHICLEID)params[1]);
	bsData.Write((DWORD)params[2]);

	RakServerInterface* pRak = pNetGame->GetRakServer();
	pRak->RPC(RPC_ScrRemoveComponent , &bsData, HIGH_PRIORITY, 
		RELIABLE, 0, UNASSIGNED_PLAYER_ID, true, false);

	int iCompId = Utils::GetTypeByComponentId(params[2]);
	if (iCompId == -1)
		return 0;

	if (pVehicle->m_CarModInfo.ucCarMod[iCompId] == (params[2] - 1000))
	{
		pVehicle->m_CarModInfo.ucCarMod[iCompId] = 0;
	}
	return 1;
}

//----------------------------------------------------------------------------------
// native ChangeVehicleColor(vehicleid, color1, color2);
static cell AMX_NATIVE_CALL n_ChangeVehicleColor(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "ChangeVehicleColor", 3);

	CVehicle* pVehicle = pNetGame->GetVehiclePool()->GetAt((VEHICLEID)params[1]);
	
	if (!pVehicle)
		return 0;

	RakNet::BitStream bsData;
	bsData.Write((BYTE)INVALID_PLAYER_ID);
	bsData.Write((int)EVENT_TYPE_CARCOLOR);
	bsData.Write((DWORD)params[1]);
	bsData.Write((DWORD)params[2]);
	bsData.Write((DWORD)params[3]);

	RakServerInterface* pRak = pNetGame->GetRakServer();
	pRak->RPC(RPC_ScmEvent , &bsData, HIGH_PRIORITY, 
		RELIABLE, 0, UNASSIGNED_PLAYER_ID, true, false);

	pVehicle->m_CarModInfo.iColor0 = (int)params[2];
	pVehicle->m_CarModInfo.iColor1 = (int)params[3];

	return 1;
}

//----------------------------------------------------------------------------------
// native ChangeVehiclePaintjob(vehicleid, paintjobid);
static cell AMX_NATIVE_CALL n_ChangeVehiclePaintjob(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "ChangeVehiclePaintjob", 2);

	CVehicle* pVehicle = pNetGame->GetVehiclePool()->GetAt((VEHICLEID)params[1]);
	if (!pVehicle)
		return 1;

	RakNet::BitStream bsData;
	bsData.Write((BYTE)INVALID_PLAYER_ID);
	bsData.Write((int)EVENT_TYPE_PAINTJOB);
	bsData.Write((DWORD)params[1]);
	bsData.Write((DWORD)params[2]);
	bsData.Write((DWORD)0);

	RakServerInterface* pRak = pNetGame->GetRakServer();
	pRak->RPC(RPC_ScmEvent , &bsData, HIGH_PRIORITY, 
		RELIABLE, 0, UNASSIGNED_PLAYER_ID, true, false);

	pVehicle->m_CarModInfo.bytePaintJob = (BYTE)params[2];

	return 1;
}

//----------------------------------------------------------------------------------
// native SetVehicleNumberPlate(vehicleid, numberplate[]);
#define VALID(x) (((x) > 0x2F && (x) < 0x3A) || ((x > 0x40) && (x < 0x5B))) ? (x) : (((x > 0x60) && (x < 0x7B)) ? ((x) - 0x20) : ('_'))
// Not sure if that's faster than a bounds check
static cell AMX_NATIVE_CALL n_SetVehicleNumberPlate(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "SetVehicleNumberPlate", 2);

	if (pNetGame->GetVehiclePool()->GetSlotState((VEHICLEID)params[1]))
	{
		char *szInput;
		// 10 char buffer to store full plate
		CHAR szPlate[9] = "";
		amx_StrParam(amx, params[2], szInput);
		if (szInput)
		{
			int len = strlen(szInput), i = 0;
			for ( ; i < 8; i++)
			{
				if (i >= len) szPlate[i] = '_'; // Pad if required
				else szPlate[i] = VALID(szInput[i]); // Else store the uppercase version, number or _ if invalid
			}
			szPlate[8] = 0;
		}
		pNetGame->GetVehiclePool()->GetAt((VEHICLEID)params[1])->SetNumberPlate(szPlate);
		return 1;
	}
	return 0;
}

//----------------------------------------------------------------------------------
// native GetVehicleModel(vehicleid);
static cell AMX_NATIVE_CALL n_GetVehicleModel(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "GetVehicleModel", 1);

	CVehiclePool *pVehiclePool = pNetGame->GetVehiclePool();
	if (pVehiclePool && pVehiclePool->GetSlotState((VEHICLEID)params[1])) {
		return pVehiclePool->GetAt((VEHICLEID)params[1])->m_SpawnInfo.iVehicleType;
	}
	return 0;
}

static cell n_GetVehicleInterior(AMX* amx, cell* params)
{
	CHECK_PARAMS(amx, "GetVehicleInterior", 1);

	CVehiclePool* pVehiclePool = pNetGame->GetVehiclePool();
	CVehicle* pVehicle = (pVehiclePool) ? (pVehiclePool->GetAt(params[1])) : (0);
	if (pVehicle != 0)
	{
		return pVehicle->m_SpawnInfo.iInterior;
	}
	return -1;
}

static cell n_GetVehicleColor(AMX* amx, cell* params)
{
	CHECK_PARAMS(amx, "GetVehicleColor", 3);

	CVehiclePool* pVehiclePool = pNetGame->GetVehiclePool();
	CVehicle* pVehicle = (pVehiclePool) ? (pVehiclePool->GetAt(params[1])) : (0);
	if (pVehicle != 0)
	{
		cell* cptr;
		if (amx_GetAddr(amx, params[2], &cptr) == AMX_ERR_NONE)
			*cptr = pVehicle->m_SpawnInfo.iColor1;
		if (amx_GetAddr(amx, params[3], &cptr) == AMX_ERR_NONE)
			*cptr = pVehicle->m_SpawnInfo.iColor2;
		return 1;
	}
	return 0;
}

// native GetVehiclePaintjob(vehicleid);
static cell n_GetVehiclePaintjob(AMX* amx, cell* params)
{
	CHECK_PARAMS(amx, "GetVehiclePaintjob", 1);

	CVehicle* pVehicle = pNetGame->GetVehiclePool()->GetAt(params[1]);
	if (pVehicle)
	{
		return pVehicle->m_CarModInfo.bytePaintJob;
	}
	return -1;
}

//----------------------------------------------------------------------------------
// native AddStaticPickup(model,type,Float:X,Float:Y,Float:Z);

static cell AMX_NATIVE_CALL n_AddStaticPickup(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "AddStaticPickup", 5);
	VECTOR vecPos;
	vecPos.X = amx_ctof(params[3]);
	vecPos.Y = amx_ctof(params[4]);
	vecPos.Z = amx_ctof(params[5]);

	if (pNetGame->GetPickupPool()->New(params[1],params[2],vecPos.X,vecPos.Y,vecPos.Z,1) != -1) return 1;
	return 0;
}

//----------------------------------------------------------------------------------
// native CreatePickup(model, type, Float:X, Float:Y, Float:Z);

static cell AMX_NATIVE_CALL n_CreatePickup(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "CreatePickup", 5);
	VECTOR vecPos;
	vecPos.X = amx_ctof(params[3]);
	vecPos.Y = amx_ctof(params[4]);
	vecPos.Z = amx_ctof(params[5]);

	return pNetGame->GetPickupPool()->New(params[1],params[2],vecPos.X,vecPos.Y,vecPos.Z);
}

//----------------------------------------------------------------------------------
// native DestroyPickup(pickup);

static cell AMX_NATIVE_CALL n_DestroyPickup(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "DestroyPickup", 1);
	return pNetGame->GetPickupPool()->Destroy(params[1]);
}

// native DestroyAllPickups();
static cell n_DestroyAllPickups(AMX* amx, cell* params)
{
	for (unsigned int uiIndex = 0; uiIndex < MAX_PICKUPS; uiIndex++)
	{
		pNetGame->GetPickupPool()->Destroy(uiIndex);
	}
	return 1;
}

// native IsValidPickup(pickupid);
static cell n_IsValidPickup(AMX* amx, cell* params)
{
	CHECK_PARAMS(amx, "IsValidPickup", 1);

	return (cell)pNetGame->GetPickupPool()->IsValid(params[1]);
}

// native IsStaticPickup(pickupid);
static cell n_IsPickupStatic(AMX* amx, cell* params)
{
	CHECK_PARAMS(amx, "IsStaticPickup", 1);

	return (cell)pNetGame->GetPickupPool()->IsStatic(params[1]);
}

// native GetPickupPos(pickupid, &Float:x, &Float:y, &Float:z);
static cell n_GetPickupPos(AMX* amx, cell* params)
{
	CHECK_PARAMS(amx, "GetPickupPos", 4);
	if (pNetGame->GetPickupPool()->IsValid(params[1]))
	{
		PICKUP pickup = pNetGame->GetPickupPool()->Get(params[1]);

		cell* cptr;
		amx_GetAddr(amx, params[2], &cptr);
		*cptr = amx_ftoc(pickup.fX);
		amx_GetAddr(amx, params[3], &cptr);
		*cptr = amx_ftoc(pickup.fY);
		amx_GetAddr(amx, params[4], &cptr);
		*cptr = amx_ftoc(pickup.fZ);
		return 1;
	}
	return 0;
}

// native GetPickupModel(pickupid);
static cell n_GetPickupModel(AMX* amx, cell* params)
{
	CHECK_PARAMS(amx, "GetPickupModel", 1);
	if (pNetGame->GetPickupPool()->IsValid(params[1]))
	{
		return pNetGame->GetPickupPool()->Get(params[1]).iModel;
	}
	return -1;
}

// native GetPickupType(pickupid);
static cell n_GetPickupType(AMX* amx, cell* params)
{
	CHECK_PARAMS(amx, "GetPickupType", 1);
	if (pNetGame->GetPickupPool()->IsValid(params[1]))
	{
		return pNetGame->GetPickupPool()->Get(params[1]).iType;
	}
	return -1;
}

// native GetPickupCount();
static cell n_GetPickupCount(AMX* amx, cell* params)
{
	return pNetGame->GetPickupPool()->GetCount();
}

// native GetPlayerWorldBounds(playerid,&Float:x_max,&Float:y_max,&Float:x_min,&Float:y_min);
static cell n_GetPlayerWorldBounds(AMX* amx, cell* params)
{
	CHECK_PARAMS(amx, "GetPlayerWorldBounds", 5);
	
	CPlayer* pPlayer = pNetGame->GetPlayerPool()->GetAt(params[1]);
	if (pPlayer == NULL)
		return 0;

	cell* cptr;
	amx_GetAddr(amx, params[2], &cptr);
	*cptr = amx_ftoc(pPlayer->m_fWorldBounds[0]);
	amx_GetAddr(amx, params[3], &cptr);
	*cptr = amx_ftoc(pPlayer->m_fWorldBounds[1]);
	amx_GetAddr(amx, params[4], &cptr);
	*cptr = amx_ftoc(pPlayer->m_fWorldBounds[2]);
	amx_GetAddr(amx, params[5], &cptr);
	*cptr = amx_ftoc(pPlayer->m_fWorldBounds[3]);

	return 1;
}

// native SetPlayerWorldBounds(playerid,Float:x_max,Float:y_max,Float:x_min,Float:y_min);
static cell AMX_NATIVE_CALL n_SetPlayerWorldBounds(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "SetPlayerWorldBounds", 5);

	RakNet::BitStream bsBounds;
	
	CPlayer* pPlayer = pNetGame->GetPlayerPool()->GetAt((BYTE)params[1]);
	if(!pPlayer) return 0;

	// Update player's new bound 
	pPlayer->m_fWorldBounds[0] = amx_ctof(params[2]);
	pPlayer->m_fWorldBounds[1] = amx_ctof(params[3]);
	pPlayer->m_fWorldBounds[2] = amx_ctof(params[4]);
	pPlayer->m_fWorldBounds[3] = amx_ctof(params[5]);
	
	bsBounds.Write(pPlayer->m_fWorldBounds[0]);
	bsBounds.Write(pPlayer->m_fWorldBounds[1]);
	bsBounds.Write(pPlayer->m_fWorldBounds[2]);
	bsBounds.Write(pPlayer->m_fWorldBounds[3]);

	pNetGame->SendToPlayer(params[1], RPC_ScrSetWorldBounds, &bsBounds);
	return 1;
}

//----------------------------------------------------------------------------------

// native Kick(playerid)
static cell AMX_NATIVE_CALL n_Kick(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "Kick", 1);

	if (pNetGame->GetPlayerPool()->GetSlotState((BYTE)params[1])) {
		pNetGame->KickPlayer((BYTE)params[1]);
		return 1;
	}
	return 0;
}

//----------------------------------------------------------------------------------

// native Ban(playerid)
static cell AMX_NATIVE_CALL n_Ban(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "Ban", 1);

	if (pNetGame->GetPlayerPool()->GetSlotState((BYTE)params[1])) {
		RakServerInterface* pRak = pNetGame->GetRakServer();
		PlayerID Player = pRak->GetPlayerIDFromIndex(params[1]);

		in_addr in;
		in.s_addr = Player.binaryAddress;
		pNetGame->AddBan(pNetGame->GetPlayerPool()->GetPlayerName((BYTE)params[1]), inet_ntoa(in), "INGAME BAN");
		pNetGame->KickPlayer((BYTE)params[1]);
		return 1;
	}
	return 0;
}

//----------------------------------------------------------------------------------

// native BanEx(playerid, reason)
static cell AMX_NATIVE_CALL n_BanEx(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "BanEx", 2);

	if (pNetGame->GetPlayerPool()->GetSlotState((BYTE)params[1])) {
		RakServerInterface* pRak = pNetGame->GetRakServer();
		PlayerID Player = pRak->GetPlayerIDFromIndex(params[1]);

		in_addr in;
		in.s_addr = Player.binaryAddress;

		char *szReason;
		amx_StrParam(amx, params[2], szReason);

		pNetGame->AddBan(pNetGame->GetPlayerPool()->GetPlayerName((BYTE)params[1]), inet_ntoa(in), szReason);
		pNetGame->KickPlayer((BYTE)params[1]);
		return 1;
	}
	return 0;
}

static cell n_RemoveBan(AMX* amx, cell* params)
{
	CHECK_PARAMS(amx, "RemoveBan", 1);

	char* szIP;
	amx_StrParam(amx, params[1], szIP);
	if (szIP)
	{
		pNetGame->RemoveBan(szIP);
		return 1;
	}
	return 0;
}

static cell n_IsBanned(AMX* amx, cell* params)
{
	CHECK_PARAMS(amx, "IsBanned", 1);
	char* szIP;
	amx_StrParam(amx, params[1], szIP);
	if (pNetGame->GetRakServer() && szIP)
	{
		return (cell)pNetGame->GetRakServer()->IsBanned(szIP);
	}
	return 0;
}

// native BlockIpAddress(ip_address[], timems)
static cell n_BlockIpAddress(AMX* amx, cell* params)
{
	CHECK_PARAMS(amx, "BlockIpAddress", 2);

	char* szIP;
	amx_StrParam(amx, params[1], szIP);
	if (pNetGame->GetRakServer() && szIP && 0 <= params[2])
	{
		pNetGame->GetRakServer()->AddToBanListEx(szIP, params[2]);
		return 1;
	}
	return 0;
}

// native UnBlockIpAddress(ip_address[])
static cell n_UnBlockIpAddress(AMX* amx, cell* params)
{
	CHECK_PARAMS(amx, "UnBlockIpAddress", 1);

	char* szIP;
	amx_StrParam(amx, params[1], szIP);
	if (pNetGame->GetRakServer() && szIP)
	{
		pNetGame->GetRakServer()->RemoveFromBanList(szIP);
		return 1;
	}
	return 0;
}

//----------------------------------------------------------------------------------
// native IsPlayerAdmin(playerid)
static cell AMX_NATIVE_CALL n_IsPlayerAdmin(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "IsPlayerAdmin", 1);
	CPlayerPool* pPlayerPool = pNetGame->GetPlayerPool();

	if (pPlayerPool->GetSlotState((BYTE)params[1]))
	{
		return pPlayerPool->IsAdmin((BYTE)params[1]);
	}
	return 0;
}

// native SetPlayerAdmin(playerid, toggle)
static cell n_SetPlayerAdmin(AMX* amx, cell* params)
{
	CHECK_PARAMS(amx, "SetPlayerAdmin", 2);

	CPlayerPool* pPool = pNetGame->GetPlayerPool();
	if (pPool && pPool->GetSlotState(params[1]))
	{
		pPool->SetAdmin(params[1], !!params[2]);
		return 1;
	}
	return 0;
}

static cell n_GetPlayerIDFromName(AMX* amx, cell* params)
{
	CHECK_PARAMS(amx, "GetPlayerIDFromName", 1);

	char* szSearchName = 0;
	amx_StrParam(amx, params[1], szSearchName);

	for (size_t uiIndex = 0; uiIndex < MAX_PLAYERS; uiIndex++)
	{
		if (!pNetGame->GetPlayerPool()->GetSlotState((unsigned char)uiIndex))
			continue;
		
		char* szPlayerName = pNetGame->GetPlayerPool()->GetPlayerName((unsigned char)uiIndex);
		if (szSearchName && Util_stristr(szPlayerName, szSearchName))
			return uiIndex;
	}
	return -1;
}

static cell n_GetPlayerCount(AMX* amx, cell* params)
{
	CPlayerPool* pPlayerPool = pNetGame->GetPlayerPool();
	if (pPlayerPool)
		return pPlayerPool->GetPlayerCount();
	return 0;
}

static cell n_GetPlayerPoolSize(AMX* amx, cell* params)
{
	CPlayerPool* pPlayerPool = pNetGame->GetPlayerPool();
	if (pPlayerPool)
		return pPlayerPool->GetLastPlayerId();
	return 0;
}

// native GetPlayerVersion(playerid, version[], len)
static cell n_GetPlayerVersion(AMX* amx, cell* params)
{
	CHECK_PARAMS(amx, "GetPlayerVersion", 3);

	CPlayer* pPlayer = pNetGame->GetPlayerPool()->GetAt(params[1]);
	if (pPlayer)
	{
		return set_amxstring(amx, params[2], pPlayer->m_szClientVersion, params[3]);
	}
	return 0;
}

//----------------------------------------------------------------------------------
// native SetSpawnInfo(playerid, team, skin, Float:x, Float:y, Float:z, Float:rotation, weapon1, weapon1_ammo, weapon2, weapon2_ammo, weapon3, weapon3_ammo)
static cell AMX_NATIVE_CALL n_SetSpawnInfo(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "SetSpawnInfo", 13);

	CPlayer *pPlayer = pNetGame->GetPlayerPool()->GetAt((BYTE)params[1]);
	if (pPlayer)
	{
		PLAYER_SPAWN_INFO SpawnInfo;
		SpawnInfo.byteTeam = (BYTE)params[2];
		SpawnInfo.iSkin = (int)params[3];
		SpawnInfo.vecPos.X = amx_ctof(params[4]);
		SpawnInfo.vecPos.Y = amx_ctof(params[5]);
		SpawnInfo.vecPos.Z = amx_ctof(params[6]);
		SpawnInfo.fRotation = amx_ctof(params[7]);
		SpawnInfo.iSpawnWeapons[0] = (int)params[8];
		SpawnInfo.iSpawnWeaponsAmmo[0] = (int)params[9];
		SpawnInfo.iSpawnWeapons[1] = (int)params[10];
		SpawnInfo.iSpawnWeaponsAmmo[1] = (int)params[11];
		SpawnInfo.iSpawnWeapons[2] = (int)params[12];
		SpawnInfo.iSpawnWeaponsAmmo[2] = (int)params[13];

		pPlayer->SetSpawnInfo(&SpawnInfo);
		RakNet::BitStream bsData;
		bsData.Write((PCHAR)&SpawnInfo, sizeof(PLAYER_SPAWN_INFO));
		RakServerInterface *pRak = pNetGame->GetRakServer();
		pRak->RPC(RPC_ScrSetSpawnInfo , &bsData, HIGH_PRIORITY, RELIABLE, 0, pRak->GetPlayerIDFromIndex((int)params[1]), false, false);
		return 1;
	} else {
		return 0;
	}
}

//----------------------------------------------------------------------------------
// native SetSpawnInfo(playerid)
static cell AMX_NATIVE_CALL n_SpawnPlayer(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "SpawnPlayer", 1);

	if (pNetGame->GetPlayerPool()->GetSlotState((BYTE)params[1]))
	{

		RakNet::BitStream bsData;
		RakServerInterface *pRak = pNetGame->GetRakServer();
		bsData.Write(2); // 2 - overwrite default behaviour
		pRak->RPC(RPC_RequestSpawn , &bsData, HIGH_PRIORITY, RELIABLE, 0, pRak->GetPlayerIDFromIndex((int)params[1]), false, false);
		return 1;
	} else {
		return 0;
	}
}

// native ForceClassSelection(playerid);
static cell AMX_NATIVE_CALL n_ForceClassSelection(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "ForceClassSelection", 1);
	if (pNetGame->GetPlayerPool()->GetSlotState((BYTE)params[1]))
	{
		RakNet::BitStream bsData;
		RakServerInterface *pRak = pNetGame->GetRakServer();
		pRak->RPC(RPC_ScrForceSpawnSelection , &bsData, HIGH_PRIORITY, RELIABLE, 0, pRak->GetPlayerIDFromIndex((int)params[1]), false, false);
		return 1;
	} else {
		return 0;
	}
}

//----------------------------------------------------------------------------------
// native GetPlayerTeam(playerid)
static cell AMX_NATIVE_CALL n_GetPlayerTeam(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "GetPlayerTeam", 1);

	CPlayerPool* pPlayerPool = pNetGame->GetPlayerPool();

	if(pPlayerPool->GetSlotState(BYTE(params[1])))
	{
		return pPlayerPool->GetAt(BYTE(params[1]))->GetTeam();
	} else {
		return -1;
	}
}

//----------------------------------------------------------------------------------

//native SetPlayerName(playerid, name[])
static cell AMX_NATIVE_CALL n_SetPlayerName(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "SetPlayerName", 2);

	if (pNetGame->GetPlayerPool()->GetSlotState(BYTE(params[1])))
	{
		char *szNewNick;
		char szOldNick[MAX_PLAYER_NAME+1];
		amx_StrParam(amx, params[2], szNewNick);

		if(ContainsInvalidNickChars(szNewNick)) return -1;

		BYTE bytePlayerID = (BYTE)params[1];
		size_t uiNickLen = strlen(szNewNick);
		BYTE byteSuccess;

		if(uiNickLen > MAX_PLAYER_NAME) return -1;

		strncpy(szOldNick,pNetGame->GetPlayerPool()->GetPlayerName(bytePlayerID),MAX_PLAYER_NAME);
		
		if (uiNickLen == 0 || pNetGame->GetPlayerPool()->IsNickInUse(szNewNick)) byteSuccess = 0;
		else byteSuccess = 1;

		RakNet::BitStream bsData;
		bsData.Write(bytePlayerID); // player id
		bsData.Write(uiNickLen); // nick length
		bsData.Write(szNewNick, uiNickLen); // name
		bsData.Write(byteSuccess); // if the nickname was rejected

		if (byteSuccess != 0)
		{
			pNetGame->GetPlayerPool()->SetPlayerName(bytePlayerID, szNewNick);

			if(pConsole->GetIntVariable("chatlogging"))
				logprintf("[nick] %s nick changed to %s", szOldNick, szNewNick);

			pNetGame->GetRakServer()->RPC(RPC_ScrSetPlayerName , &bsData, HIGH_PRIORITY, RELIABLE, 0, UNASSIGNED_PLAYER_ID, true, false);

		}

		return byteSuccess;
	}
	else
	{
		return 0;
	}
}

//----------------------------------------------------------------------------------

// native SetPlayerSkin(playerid, skin)
static cell AMX_NATIVE_CALL n_SetPlayerSkin(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "SetPlayerSkin", 2);

	if(pNetGame->GetPlayerPool()->GetSlotState(BYTE(params[1])))
	{
		CPlayer *pPlayer = pNetGame->GetPlayerPool()->GetAt((BYTE)params[1]);

		// Check to make sure the player has spawned before setting the skin remotely
		if (pPlayer->m_bHasSpawnInfo)
		{
			RakNet::BitStream bsData;
			bsData.Write((int)params[1]); // player id
			bsData.Write((int)params[2]); // skin id
			RakServerInterface *pRak = pNetGame->GetRakServer();
			pRak->RPC(RPC_ScrSetPlayerSkin , &bsData, HIGH_PRIORITY, RELIABLE, 0, UNASSIGNED_PLAYER_ID, true, false);
		} 
		pPlayer->m_SpawnInfo.iSkin = (int)params[2];

		return 1;
	}
	else
	{
		return 0;
	}
}

//----------------------------------------------------------------------------------

// native GetPlayerSkin(playerid)
static cell AMX_NATIVE_CALL n_GetPlayerSkin(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "GetPlayerSkin", 1);

	CPlayer *pPlayer = pNetGame->GetPlayerPool()->GetAt((BYTE)params[1]);
	if (pPlayer)
	{
		return pPlayer->m_SpawnInfo.iSkin;
	}
	return 0;
}

static cell n_SetPlayerFightingStyle(AMX* amx, cell* params)
{
	CHECK_PARAMS(amx, "SetPlayerFightingStyle", 3);
	CPlayerPool* pPool = pNetGame->GetPlayerPool();
	if (pPool && pPool->GetSlotState(params[1]))
	{
		RakNet::BitStream out;
		out.Write<int>(4);
		out.Write((unsigned char)params[2]);
		out.Write((unsigned char)params[3]);
		pNetGame->SendToPlayer(params[1], RPC_ScrSetPlayer, &out);
		return 1;
	}
	return 0;
}

// native SetPlayerMaxHealth(playerid, Float:max_health)
static cell n_SetPlayerMaxHealth(AMX* amx, cell* params)
{
	CHECK_PARAMS(amx, "SetPlayerMaxHealth", 2);
	CPlayerPool* pPool = pNetGame->GetPlayerPool();
	if (pPool && pPool->GetSlotState(params[1]))
	{
		RakNet::BitStream out;
		float fVal = amx_ctof(params[2]);
		out.Write<int>(5);
		out.Write(fVal);
		pNetGame->SendToPlayer(params[1], RPC_ScrSetPlayer, &out);
		return 1;
	}
	return 0;
}
// native SetPlayerBlurLevel(playerid,level)
static cell n_SetPlayerBlurLevel(AMX* amx,cell *params)
{
	CHECK_PARAMS(amx, "SetPlayerBlurLevel",2);
	CPlayerPool* pPool = pNetGame->GetPlayerPool();
	if (pPool && pPool->GetSlotState(params[1]))
	{
		RakNet::BitStream out;
		out.Write<int>(6);
		out.Write((unsigned char)params[2]);
		pNetGame->SendToPlayer(params[1], RPC_ScrSetPlayer, &out);
		return 1;
	}
}
static cell n_InterpolateCameraPos(AMX* amx, cell* params)
{
	CHECK_PARAMS(amx, "InterpolateCameraPos", 9);
	CPlayerPool* pPool = pNetGame->GetPlayerPool();
	if (pPool && pPool->GetSlotState(params[1]))
	{
		RakNet::BitStream out;
		
		out.Write<BYTE>(1);
		out.Write<float>(amx_ctof(params[2]));
		out.Write<float>(amx_ctof(params[3]));
		out.Write<float>(amx_ctof(params[4]));

		out.Write<float>(amx_ctof(params[5]));
		out.Write<float>(amx_ctof(params[6]));
		out.Write<float>(amx_ctof(params[7]));

		out.Write<float>((float)params[8]);
		out.Write<BYTE>(params[9]);

		return pNetGame->SendToPlayer(params[1], RPC_ScrInterpolateCamera, &out);
	}
	return 0;
}

static cell n_InterpolateCameraLookAt(AMX* amx, cell* params)
{
	CHECK_PARAMS(amx, "InterpolateCameraLookAt", 9);
	CPlayerPool* pPool = pNetGame->GetPlayerPool();
	if (pPool && pPool->GetSlotState(params[1]))
	{
		RakNet::BitStream out;

		out.Write<BYTE>(0);
		out.Write<float>(amx_ctof(params[2]));
		out.Write<float>(amx_ctof(params[3]));
		out.Write<float>(amx_ctof(params[4]));

		out.Write<float>(amx_ctof(params[5]));
		out.Write<float>(amx_ctof(params[6]));
		out.Write<float>(amx_ctof(params[7]));

		out.Write<float>((float)params[8]);
		out.Write<BYTE>(params[9]);

		return pNetGame->SendToPlayer(params[1], RPC_ScrInterpolateCamera, &out);
	}
	return 0;
}

//----------------------------------------------------------------------------------

// native SetPlayerVirtualWorld(playerid, worldid)

static cell AMX_NATIVE_CALL n_SetPlayerVirtualWorld(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "SetPlayerVirtualWorld", 2);

	if (pNetGame->GetPlayerPool()->GetSlotState((BYTE)params[1]))
	{
		pNetGame->GetPlayerPool()->SetPlayerVirtualWorld((BYTE)params[1], (BYTE)params[2]);
		return 1;
	}
	return 0;
}

//----------------------------------------------------------------------------------

// native GetPlayerVirtualWorld(playerid)

static cell AMX_NATIVE_CALL n_GetPlayerVirtualWorld(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "GetPlayerVirtualWorld", 1);

	if (pNetGame->GetPlayerPool()->GetSlotState((BYTE)params[1]))
	{
		return pNetGame->GetPlayerPool()->GetPlayerVirtualWorld((BYTE)params[1]);
	}
	return 0;
}

//----------------------------------------------------------------------------------

// native SetVehicleVirtualWorld(vehicleid, worldid)

static cell AMX_NATIVE_CALL n_SetVehicleVirtualWorld(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "SetVehicleVirtualWorld", 2);

	if (pNetGame->GetVehiclePool()->GetSlotState((VEHICLEID)params[1]))
	{
		pNetGame->GetVehiclePool()->SetVehicleVirtualWorld((VEHICLEID)params[1], (BYTE)params[2]);
		return 1;
	}
	return 0;
}

//----------------------------------------------------------------------------------

// native GetVehicleVirtualWorld(vehicleid)

static cell AMX_NATIVE_CALL n_GetVehicleVirtualWorld(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "GetVehicleVirtualWorld", 1);

	if (pNetGame->GetVehiclePool()->GetSlotState((VEHICLEID)params[1]))
	{
		return pNetGame->GetVehiclePool()->GetVehicleVirtualWorld((VEHICLEID)params[1]);
	}
	return 0;
}

// native RepairVehicle(vehicleid)
static cell n_RepairVehicle(AMX* amx, cell* params)
{
	CHECK_PARAMS(amx, "RepairVehicle", 1);

	CVehiclePool* pPool = pNetGame->GetVehiclePool();
	if (pPool && pPool->GetSlotState(params[1]))
	{
		RakNet::BitStream bs;

		bs.Write<int>(VEHICLE_OP_REPAIR);
		bs.Write(params[1]);

		return (cell)pNetGame->GetRakServer()->RPC(RPC_ScrSetVehicle, &bs, HIGH_PRIORITY,
			RELIABLE, 0, UNASSIGNED_PLAYER_ID, true, false);
	}
	return 0;
}

// native SetVehicleParamsCarWindows(vehicleid, driver, passenger, backleft, backright)
static cell n_SetVehicleParamsCarWindows(AMX* amx, cell* params)
{
	CHECK_PARAMS(amx, "SetVehicleParamsCarWindows", 5);

	CVehiclePool* pVehiclePool = pNetGame->GetVehiclePool();
	if (pVehiclePool && pVehiclePool->GetSlotState(params[1]))
	{
		RakNet::BitStream out;

		out.Write<int>(VEHICLE_OP_WINDOW);
		out.Write(params[1]); // vehicleid
		out.Write(params[2]); // driver
		out.Write(params[3]); // passenger
		out.Write(params[4]); // back-left
		out.Write(params[5]); // back-right

		return (cell)pNetGame->GetRakServer()->RPC(RPC_ScrSetVehicle, &out, HIGH_PRIORITY,
			RELIABLE, 0, UNASSIGNED_PLAYER_ID, true, false);
	}
	return 0;
}

// native ToggleTaxiLight(vehicleid, toggle)
static cell n_ToggleTaxiLight(AMX* amx, cell* params)
{
	CHECK_PARAMS(amx, "ToggleTaxiLight", 2);

	CVehiclePool* pVehiclePool = pNetGame->GetVehiclePool();
	if (pVehiclePool && pVehiclePool->GetSlotState(params[1]))
	{
		RakNet::BitStream out;

		out.Write<int>(VEHICLE_OP_TAXI_LIGHT);
		out.Write(params[1]); // vehicleid
		out.Write(params[2]); // toggle

		return (cell)pNetGame->GetRakServer()->RPC(RPC_ScrSetVehicle, &out, HIGH_PRIORITY,
			RELIABLE, 0, UNASSIGNED_PLAYER_ID, true, false);
	}
	return 0;
}

// native SetVehicleEngineState(vehicleid, engine_state)
static cell n_SetVehicleEngineState(AMX* amx, cell* params)
{
	CHECK_PARAMS(amx, "SetVehicleEngineState", 2);

	CVehiclePool* pVehiclePool = pNetGame->GetVehiclePool();
	if (pVehiclePool && pVehiclePool->GetSlotState(params[1]))
	{
		RakNet::BitStream out;

		out.Write<int>(4);
		out.Write(params[1]); // vehicleid
		out.Write(params[2]); // toggle

		return (cell)pNetGame->GetRakServer()->RPC(RPC_ScrSetVehicle, &out, HIGH_PRIORITY,
			RELIABLE, 0, UNASSIGNED_PLAYER_ID, true, false);
	}
	return 0;
}

// native GetVehicleVelocity(vehicleid, &Float:x, &Float:y, &Float:z)
static cell n_GetVehicleVelocity(AMX* amx, cell* params)
{
	CHECK_PARAMS(amx, "GetVehicleVelocity", 4);

	CVehicle* pVehicle = NULL;
	CVehiclePool* pPool = pNetGame->GetVehiclePool();
	if (0 <= params[1] && pPool != NULL && (pVehicle = pPool->GetAt(params[1])) != NULL)
	{
		cell* cptr;
		amx_GetAddr(amx, params[2], &cptr);
		*cptr = amx_ftoc(pVehicle->m_vecMoveSpeed.X);
		amx_GetAddr(amx, params[3], &cptr);
		*cptr = amx_ftoc(pVehicle->m_vecMoveSpeed.Y);
		amx_GetAddr(amx, params[4], &cptr);
		*cptr = amx_ftoc(pVehicle->m_vecMoveSpeed.Z);

		return 1;
	}
	return 0;
}

// native SetVehicleVelocity(vehicleid, Float:X, Float:Y, Float:Z)
static cell n_SetVehicleVelocity(AMX* amx, cell* params)
{
	CHECK_PARAMS(amx, "SetVehicleVelocity", 4);

	CVehiclePool* pPool = pNetGame->GetVehiclePool();
	if (0 <= params[1] && pPool != NULL && pPool->GetSlotState(params[1]))
	{
		RakNet::BitStream out;
		float fX, fY, fZ;

		fX = amx_ctof(params[2]);
		fY = amx_ctof(params[3]);
		fZ = amx_ctof(params[4]);

		out.Write(params[1]);
		out.Write(fX);
		out.Write(fY);
		out.Write(fZ);

		bool bRet = false;
		if (pNetGame->GetRakServer())
			bRet = pNetGame->GetRakServer()->RPC(RPC_ScrVehicleVelocity, &out,
				HIGH_PRIORITY, RELIABLE, 0, UNASSIGNED_PLAYER_ID, true, false);

		return bRet;
	}
	return 0;
}

//----------------------------------------------------------------------------------
// native TogglePlayerSpectating(playerid, toggle);
static cell AMX_NATIVE_CALL n_TogglePlayerSpectating(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "TogglePlayerSpectating", 2);
	if (pNetGame->GetPlayerPool()->GetSlotState((BYTE)params[1]))
	{
		CPlayer *pPlayer = pNetGame->GetPlayerPool()->GetAt((BYTE)params[1]);
		
		pPlayer->m_SpectateID = 0xFFFFFFFF;
		pPlayer->m_byteSpectateType = SPECTATE_TYPE_NONE;

		RakNet::BitStream bsParams;
		bsParams.Write((BOOL)params[2]); // toggle
		RakServerInterface* pRak = pNetGame->GetRakServer();
		pRak->RPC(RPC_ScrTogglePlayerSpectating , &bsParams, HIGH_PRIORITY, RELIABLE, 0, pRak->GetPlayerIDFromIndex((BYTE)params[1]), false, false);
		return 1;
	}
	return 0;
}

//----------------------------------------------------------------------------------
// native PlayerSpectateVehicle(playerid, vehicleid, mode = SPECTATE_MODE_NORMAL);
static cell AMX_NATIVE_CALL n_PlayerSpectateVehicle(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "PlayerSpectateVehicle", 3);

	//printf("n_PlayerSpectateVehicle(%u,%u,%u)",params[1],params[2],params[3]);

	if (pNetGame->GetPlayerPool()->GetSlotState((BYTE)params[1])
		&& pNetGame->GetVehiclePool()->GetSlotState((VEHICLEID)params[2]))
	{
		CPlayer *pPlayer = pNetGame->GetPlayerPool()->GetAt((BYTE)params[1]);
		pPlayer->m_SpectateID = (VEHICLEID)params[2];
		pPlayer->m_byteSpectateType = SPECTATE_TYPE_VEHICLE;

		RakNet::BitStream bsParams;
		bsParams.Write((VEHICLEID)params[2]); // vehicleid
		bsParams.Write((BYTE)params[3]); // mode
		RakServerInterface* pRak = pNetGame->GetRakServer();
		pRak->RPC(RPC_ScrPlayerSpectateVehicle , &bsParams, HIGH_PRIORITY, RELIABLE, 0, pRak->GetPlayerIDFromIndex((BYTE)params[1]), false, false);
		return 1;
	}
	return 0;
}

//----------------------------------------------------------------------------------
// native PlayerSpectatePlayer(playerid, vehicleid, mode = SPECTATE_MODE_NORMAL);
static cell AMX_NATIVE_CALL n_PlayerSpectatePlayer(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "PlayerSpectatePlayer", 3);
	if (pNetGame->GetPlayerPool()->GetSlotState((BYTE)params[1])
		&& pNetGame->GetPlayerPool()->GetSlotState((BYTE)params[2]))
	{
		CPlayer *pPlayer = pNetGame->GetPlayerPool()->GetAt((BYTE)params[1]);
		pPlayer->m_SpectateID = (BYTE)params[2];
		pPlayer->m_byteSpectateType = SPECTATE_TYPE_PLAYER;

		RakNet::BitStream bsParams;
		bsParams.Write((BYTE)params[2]); // playerid
		bsParams.Write((BYTE)params[3]); // mode
		RakServerInterface* pRak = pNetGame->GetRakServer();
		pRak->RPC(RPC_ScrPlayerSpectatePlayer , &bsParams, HIGH_PRIORITY, RELIABLE, 0, pRak->GetPlayerIDFromIndex((BYTE)params[1]), false, false);
		return 1;
	}
	return 0;
}

//----------------------------------------------------------------------------------

// native SetPlayerTeam(playerid, team)
static cell AMX_NATIVE_CALL n_SetPlayerTeam(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "SetPlayerTeam", 2);

	//CPlayerPool* pPlayerPool = pNetGame->GetPlayerPool();

	if(pNetGame->GetPlayerPool()->GetSlotState(BYTE(params[1])))
	{
		//pPlayerPool->SetTeam(params[1], params[2]);

		RakNet::BitStream bsParams;
		bsParams.Write((BYTE)params[1]); // playerid
		bsParams.Write((BYTE)params[2]); // team id
		RakServerInterface* pRak = pNetGame->GetRakServer();
		pRak->RPC(RPC_ScrSetPlayerTeam , &bsParams, HIGH_PRIORITY, RELIABLE, 0, UNASSIGNED_PLAYER_ID, true, false);
		return 1;
	} else {
		return 0;
	}
}

//----------------------------------------------------------------------------------

// native GetPlayerPos(playerid, &Float:x, &Float:y, &Float:z)
static cell AMX_NATIVE_CALL n_GetPlayerPos(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "GetPlayerPos", 4);

	CPlayer* pPlayer = pNetGame->GetPlayerPool()->GetAt((BYTE)params[1]);

	if (pPlayer)
	{
		cell* cptr;
		amx_GetAddr(amx, params[2], &cptr);
		*cptr = amx_ftoc(pPlayer->m_vecPos.X);
		amx_GetAddr(amx, params[3], &cptr);
		*cptr = amx_ftoc(pPlayer->m_vecPos.Y);
		amx_GetAddr(amx, params[4], &cptr);
		*cptr = amx_ftoc(pPlayer->m_vecPos.Z);

		return 1;
	} else {
		return 0;
	}
}

//----------------------------------------------------------------------------------

// native SetPlayerPos(playerid, Float:x, Float:y, Float:z)
static cell AMX_NATIVE_CALL n_SetPlayerPos(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "SetPlayerPos", 4);

	if(pNetGame->GetPlayerPool()->GetSlotState(BYTE(params[1])))
	{
		VECTOR vecPos;
		vecPos.X = amx_ctof(params[2]);
		vecPos.Y = amx_ctof(params[3]);
		vecPos.Z = amx_ctof(params[4]);

		RakNet::BitStream bsParams;
		bsParams.Write(vecPos.X);	// X
		bsParams.Write(vecPos.Y);	// Y
		bsParams.Write(vecPos.Z);	// Z

		RakServerInterface* pRak = pNetGame->GetRakServer();
		pRak->RPC(RPC_ScrSetPlayerPos , &bsParams, HIGH_PRIORITY, RELIABLE, 0,
			pRak->GetPlayerIDFromIndex(params[1]), false, false);

		return 1;
	}
	else
	{
		return 0;
	}
}

//----------------------------------------------------------------------------------

// native SetPlayerPosFindZ(playerid, Float:x, Float:y, Float:z)
static cell AMX_NATIVE_CALL n_SetPlayerPosFindZ(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "SetPlayerPosFindZ", 4);

	if(pNetGame->GetPlayerPool()->GetSlotState(BYTE(params[1])))
	{
		VECTOR vecPos;
		vecPos.X = amx_ctof(params[2]);
		vecPos.Y = amx_ctof(params[3]);
		vecPos.Z = amx_ctof(params[4]);

		RakNet::BitStream bsParams;
		bsParams.Write(vecPos.X);	// X
		bsParams.Write(vecPos.Y);	// Y
		bsParams.Write(vecPos.Z);	// Z

		RakServerInterface* pRak = pNetGame->GetRakServer();
		pRak->RPC(RPC_ScrSetPlayerPosFindZ , &bsParams, HIGH_PRIORITY, RELIABLE, 0,
			pRak->GetPlayerIDFromIndex(params[1]), false, false);

		return 1;
	}
	else
	{
		return 0;
	}
}

//----------------------------------------------------------------------------------
// native GetPlayerHealth(playerid, &Float:health)
static cell AMX_NATIVE_CALL n_GetPlayerHealth(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "GetPlayerHealth", 2);

	CPlayer* pPlayer = pNetGame->GetPlayerPool()->GetAt((BYTE)params[1]);

	if (pPlayer)
	{
		cell* cptr;
		amx_GetAddr(amx, params[2], &cptr);
		*cptr = amx_ftoc(pPlayer->m_fHealth);

		return 1;
	} else {
		return 0;
	}
}

//----------------------------------------------------------------------------------
// native SetPlayerHealth(playerid,Float:health)

static cell AMX_NATIVE_CALL n_SetPlayerHealth(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "SetPlayerHealth", 2);

	if(pNetGame->GetPlayerPool()->GetSlotState(BYTE(params[1])))
	{
		float fHealth = amx_ctof(params[2]);

		RakNet::BitStream bsHealth;
		bsHealth.Write(fHealth);

		//logprintf("Setting health of %d to %f:", params[1], fHealth);
		
		pNetGame->GetRakServer()->RPC(RPC_ScrSetPlayerHealth , &bsHealth, HIGH_PRIORITY, 
			RELIABLE, 0, pNetGame->GetRakServer()->GetPlayerIDFromIndex(params[1]), false, false);

		return 1;
	}
	else
	{
		return 0;
	}
}

//----------------------------------------------------------------------------------

// native PutPlayerInVehicle(playerid, vehicleid, seatid)
static cell AMX_NATIVE_CALL n_PutPlayerInVehicle(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "PutPlayerInVehicle", 3);

	if(pNetGame->GetPlayerPool()->GetSlotState(params[1]) && pNetGame->GetVehiclePool()->GetSlotState((VEHICLEID)params[2]))
	{
		RakNet::BitStream bsParams;
		bsParams.Write((VEHICLEID)params[2]);	// vehicleid
		bsParams.Write((BYTE)params[3]);	// seatid

		if((BYTE)params[3] == 0) { // driver
			pNetGame->GetVehiclePool()->GetAt((VEHICLEID)params[2])->m_byteDriverID = (BYTE)params[1];
		}

		RakServerInterface *pRak = pNetGame->GetRakServer();
		pRak->RPC(RPC_ScrPutPlayerInVehicle , &bsParams, HIGH_PRIORITY,
			RELIABLE_ORDERED, 0, pRak->GetPlayerIDFromIndex(params[1]), false, false);

		return 1;
	}
	else
	{
		return 0;
	}
}


//----------------------------------------------------------------------------------

// native RemovePlayerFromVehicle(playerid)
static cell AMX_NATIVE_CALL n_RemovePlayerFromVehicle(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "RemovePlayerFromVehicle", 1);

	if (!pNetGame->GetPlayerPool()->GetSlotState(params[1])) return 0;
	RakNet::BitStream bsParams;

	RakServerInterface *pRak = pNetGame->GetRakServer();
	pNetGame->GetRakServer()->RPC(RPC_ScrRemovePlayerFromVehicle , &bsParams, HIGH_PRIORITY,
		RELIABLE, 0, pRak->GetPlayerIDFromIndex(params[1]), false, false);

	return 1;
}

//----------------------------------------------------------------------------------

// native IsPlayerInVehicle(playerid, vehicleid)
static cell AMX_NATIVE_CALL n_IsPlayerInVehicle(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "IsPlayerInVehicle", 2);

	CPlayer* pPlayer = pNetGame->GetPlayerPool()->GetAt((BYTE)params[1]);
	if (!pPlayer) return 0;
	BYTE byteState = pPlayer->GetState();
	if ((byteState == PLAYER_STATE_DRIVER) || (byteState == PLAYER_STATE_PASSENGER))
	{
		if (pPlayer->m_VehicleID == params[2])
		{
			return 1;
		}
	}

	return 0;
}

//----------------------------------------------------------------------------------

// native IsPlayerInAnyVehicle(playerid)
static cell AMX_NATIVE_CALL n_IsPlayerInAnyVehicle(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "IsPlayerInAnyVehicle", 1);

	CPlayer* pPlayer = pNetGame->GetPlayerPool()->GetAt((BYTE)params[1]);
	if (!pPlayer) return 0;
	BYTE byteState = pPlayer->GetState();

	if ((byteState == PLAYER_STATE_DRIVER) || (byteState == PLAYER_STATE_PASSENGER))
	{
		return 1;
	}

	return 0;
}

//----------------------------------------------------------------------------------

// native GetPlayerName(playerid, const name[], len)
static cell AMX_NATIVE_CALL n_GetPlayerName(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "GetPlayerName", 3);
	BYTE bytePlayerID = (BYTE)params[1];
	if (bytePlayerID > MAX_PLAYERS || !pNetGame->GetPlayerPool()->GetSlotState(bytePlayerID)) return 0;
	return set_amxstring(amx, params[2], pNetGame->GetPlayerPool()->
		GetPlayerName(bytePlayerID), params[3]);
}

//----------------------------------------------------------------------------------

// native GetWeaponName(weaponid, const name[], len)
static cell AMX_NATIVE_CALL n_GetWeaponName(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "GetWeaponName", 3);
	if(params[1] > WEAPON_COLLISION) return 0;

	return set_amxstring(amx,params[2],pNetGame->GetWeaponName(params[1]),params[3]);
}

static cell n_GetVehiclePoolSize(AMX* amx, cell* params)
{
	CVehiclePool* pVehiclePool = pNetGame->GetVehiclePool();
	if (pVehiclePool)
		return pVehiclePool->GetVehicleLastId();
	return 0;
}

// native IsValidVehicle(vehicleid);
static cell n_IsValidVehicle(AMX* amx, cell* params)
{
	CHECK_PARAMS(amx, "IsValidVehicle", 1);

	CVehiclePool* pVehiclePool = pNetGame->GetVehiclePool();
	if (pVehiclePool && pVehiclePool->GetSlotState((VEHICLEID)params[1]))
		return 1;
	else
		return 0;
}

// native CreateVehicle(vehicletype, Float:x, Float:y, Float:z, Float:rotation, color1, color2, respawndelay)
static cell AMX_NATIVE_CALL n_CreateVehicle(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "CreateVehicle", 8);

	VECTOR vecPos;
	vecPos.X = amx_ctof(params[2]);
	vecPos.Y = amx_ctof(params[3]);
	vecPos.Z = amx_ctof(params[4]);

	VEHICLEID VehicleID = pNetGame->GetVehiclePool()->New((int)params[1], &vecPos, 
		amx_ctof(params[5]), (int)params[6], (int)params[7], ((int)params[8]) * 1000);

	if (VehicleID != 0xFFFF)
	{
		for(int x = 0; x < MAX_PLAYERS;x++) {	
			if (pNetGame->GetPlayerPool()->GetSlotState(x)) 	{
				pNetGame->GetVehiclePool()->GetAt(VehicleID)->SpawnForPlayer(x);
			}
		}
	}

	return VehicleID;
}

//----------------------------------------------------

// native DestroyVehicle(vehicleid)

static cell AMX_NATIVE_CALL n_DestroyVehicle(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "DestroyVehicle", 1);

	CVehiclePool*	pVehiclePool = pNetGame->GetVehiclePool();
	if (pVehiclePool->GetAt((VEHICLEID)params[1]))
	{
		pVehiclePool->Delete((VEHICLEID)params[1]);
		RakNet::BitStream bsParams;

		bsParams.Write((VEHICLEID)params[1]);

		pNetGame->GetRakServer()->RPC(RPC_VehicleDestroy , &bsParams, HIGH_PRIORITY, RELIABLE, 0, UNASSIGNED_PLAYER_ID, true, false);
		return 1;
	}
	return 0;
}

//----------------------------------------------------------------------------------

// native GetVehiclePos(vehicleid, &Float:x, &Float:y, &Float:z)
static cell AMX_NATIVE_CALL n_GetVehiclePos(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "GetVehiclePos", 4);

	CVehicle* pVehicle = pNetGame->GetVehiclePool()->GetAt((VEHICLEID)params[1]);

	if (pVehicle)
	{
		cell* cptr;
		amx_GetAddr(amx, params[2], &cptr);
		*cptr = amx_ftoc(pVehicle->m_matWorld.pos.X);
		amx_GetAddr(amx, params[3], &cptr);
		*cptr = amx_ftoc(pVehicle->m_matWorld.pos.Y);
		amx_GetAddr(amx, params[4], &cptr);
		*cptr = amx_ftoc(pVehicle->m_matWorld.pos.Z);

		return 1;
	} else {
		return 0;
	}
}

//----------------------------------------------------------------------------------

// native SetVehiclePos(vehicleid, Float:x, Float:y, Float:z)
static cell AMX_NATIVE_CALL n_SetVehiclePos(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "SetVehiclePos", 4);

	CVehicle* pVehicle = pNetGame->GetVehiclePool()->GetAt((VEHICLEID)params[1]);

	if (pVehicle)
	{
		if (pVehicle->m_byteDriverID != INVALID_ID)
		{
			RakNet::BitStream bsParams;
			bsParams.Write((VEHICLEID)params[1]);
			bsParams.Write(amx_ctof(params[2]));
			bsParams.Write(amx_ctof(params[3]));
			bsParams.Write(amx_ctof(params[4]));
			RakServerInterface* pRak = pNetGame->GetRakServer();
			pRak->RPC(RPC_ScrSetVehiclePos , &bsParams, HIGH_PRIORITY, 
				RELIABLE, 0, pRak->GetPlayerIDFromIndex(pVehicle->m_byteDriverID), false, false);
		}

		pVehicle->m_matWorld.pos.X = amx_ctof(params[2]);
		pVehicle->m_matWorld.pos.Y = amx_ctof(params[3]);
		pVehicle->m_matWorld.pos.Z = amx_ctof(params[4]);

		return 1;
	} else {
		return 0;
	}
}

// native IsVehicleOnItsSide(vehicleid)
static cell n_IsVehicleOnItsSide(AMX* amx, cell* params)
{
	CHECK_PARAMS(amx, "IsVehicleOnItsSide", 1);
	CVehiclePool* pPool = pNetGame->GetVehiclePool();
	CVehicle* pVehicle = NULL;
	if (pPool != NULL && (pVehicle = pPool->GetAt(params[1])) != NULL)
	{
		return pVehicle->m_bOnItsSide;
	}
	return 0;
}

// native IsVehicleUpsideDown(vehicleid)
static cell n_IsVehicleUpsideDown(AMX* amx, cell* params)
{
	CHECK_PARAMS(amx, "IsVehicleUpsideDown", 1);
	CVehiclePool* pPool = pNetGame->GetVehiclePool();
	CVehicle* pVehicle = NULL;
	if (pPool != NULL && (pVehicle = pPool->GetAt(params[1])) != NULL)
	{
		return pVehicle->m_bUpsideDown;
	}
	return 0;
}

// native GetVehicleSirenState(vehicle)
static cell n_GetVehicleSirenState(AMX* amx, cell* params)
{
	CHECK_PARAMS(amx, "GetVehicleSirenState", 1);
	CVehiclePool* pPool = pNetGame->GetVehiclePool();
	CVehicle* pVehicle = NULL;
	if (pPool != NULL && (pVehicle = pPool->GetAt(params[1])) != NULL)
	{
		return pVehicle->m_bSirenOn;
	}
	return -1;
}

// native IsVehicleWrecked(vehicle)
static cell n_IsVehicleWrecked(AMX* amx, cell* params)
{
	CHECK_PARAMS(amx, "IsVehicleWrecked", 1);
	CVehiclePool* pPool = pNetGame->GetVehiclePool();
	CVehicle* pVehicle = NULL;
	if (pPool != NULL && (pVehicle = pPool->GetAt(params[1])) != NULL)
	{
		return pVehicle->m_bWrecked;
	}
	return 0;
}

// native IsVehicleInWater(vehicle)
static cell n_IsVehicleSunked(AMX* amx, cell* params)
{
	CHECK_PARAMS(amx, "IsVehicleSunked", 1);
	CVehiclePool* pPool = pNetGame->GetVehiclePool();
	CVehicle* pVehicle = NULL;
	if (pPool != NULL && (pVehicle = pPool->GetAt(params[1])) != NULL)
	{
		return pVehicle->m_bSunked;
	}
	return 0;
}

// native SetVehicleLightState(vehicleid, light_state)
static cell n_SetVehicleLightState(AMX* amx, cell* params)
{
	CHECK_PARAMS(amx, "SetVehicleLightState", 2);
	CVehiclePool* pPool = pNetGame->GetVehiclePool();
	if (0 < params[1] && pPool && pPool->GetSlotState(params[1]))
	{
		unsigned char ucState = (params[2]) ? (1) : (0);
		RakNet::BitStream out;

		out.Write<int>(5);
		out.Write((VEHICLEID)params[1]);
		out.Write(ucState);

		return pNetGame->GetRakServer()->RPC(RPC_ScrSetVehicle, &out, HIGH_PRIORITY, RELIABLE, 0, UNASSIGNED_PLAYER_ID, true, false);
	}
	return 0;
}

// native SetVehicleRespawnDelay(vehicleid, respawn_delay)
static cell n_SetVehicleRespawnDelay(AMX* amx, cell* params)
{
	CHECK_PARAMS(amx, "SetVehicleRespawnDelay", 2);
	CVehiclePool* pPool = pNetGame->GetVehiclePool();
	CVehicle* pVehicle;
	if (0 < params[1] && pPool && (pVehicle = pPool->GetAt(params[1])) != NULL && -1 <= params[2])
	{
		pVehicle->m_SpawnInfo.iRespawnDelay = (int)params[2];
		return 1;
	}
	return 0;
}

// native GetVehicleRespawnDelay(vehicleid)
static cell n_GetVehicleRespawnDelay(AMX* amx, cell* params)
{
	CHECK_PARAMS(amx, "GetVehicleRespawnDelay", 1);
	CVehiclePool* pPool = pNetGame->GetVehiclePool();
	CVehicle* pVehicle;
	if (0 < params[1] && pPool && (pVehicle = pPool->GetAt(params[1])) != NULL)
	{
		return pVehicle->m_SpawnInfo.iRespawnDelay;
	}
	return 0;
}

// native SetVehicleSpawnPos(vehicleid, Float:x, Float:y, Float:z)
static cell n_SetVehicleSpawnPos(AMX* amx, cell* params)
{
	CHECK_PARAMS(amx, "SetVehicleSpawnPos", 4);
	CVehiclePool* pPool = pNetGame->GetVehiclePool();
	CVehicle* pVehicle;
	if (0 < params[1] && pPool && (pVehicle = pPool->GetAt(params[1])) != NULL)
	{
		pVehicle->m_SpawnInfo.vecPos.X = amx_ctof(params[2]);
		pVehicle->m_SpawnInfo.vecPos.Y = amx_ctof(params[3]);
		pVehicle->m_SpawnInfo.vecPos.Z = amx_ctof(params[4]);
		return 1;
	}
	return 0;
}

// native GetVehicleSpawnPos(vehicleid, &Float:x, &Float:y, &Float:z)
static cell n_GetVehicleSpawnPos(AMX* amx, cell* params)
{
	CHECK_PARAMS(amx, "GetVehicleSpawnPos", 4);
	CVehiclePool* pPool = pNetGame->GetVehiclePool();
	CVehicle* pVehicle;
	if (0 < params[1] && pPool && (pVehicle = pPool->GetAt(params[1])) != NULL)
	{
		cell* cptr;
		amx_GetAddr(amx, params[2], &cptr);
		*cptr = amx_ftoc(pVehicle->m_SpawnInfo.vecPos.X);
		amx_GetAddr(amx, params[3], &cptr);
		*cptr = amx_ftoc(pVehicle->m_SpawnInfo.vecPos.Y);
		amx_GetAddr(amx, params[4], &cptr);
		*cptr = amx_ftoc(pVehicle->m_SpawnInfo.vecPos.Z);
		return 1;
	}
	return 0;
}

// native GetVehicleComponentInSlot(vehicleid, slot)
static cell n_GetVehicleComponentInSlot(AMX* amx, cell* params)
{
	CHECK_PARAMS(amx, "GetVehicleComponentInSlot", 2);
	CVehicle* pVehicle;
	CVehiclePool* pPool = pNetGame->GetVehiclePool();
	if (pPool && (pVehicle = pPool->GetAt(params[1])) != NULL)
	{
		if ((0 <= params[2] && params[2] < 17) && 
			pVehicle->m_CarModInfo.ucCarMod[params[2]] != 0)
		{
			return pVehicle->m_CarModInfo.ucCarMod[params[2]] + 1000;
		}
	}
	return 0;
}

// native GetVehicleComponentType(component)
static cell n_GetVehicleComponentType(AMX* amx, cell* params)
{
	CHECK_PARAMS(amx, "GetVehicleComponentType", 1);
	return Utils::GetTypeByComponentId(params[1]);
}

//----------------------------------------------------------------------------------

// native SendClientMessage(playerid, color, const message[])
static cell AMX_NATIVE_CALL n_SendClientMessage(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "SendClientMessage", 3);
	if (!pNetGame->GetPlayerPool()->GetSlotState(params[1])) return 0;
	PlayerID pidPlayer = pNetGame->GetRakServer()->GetPlayerIDFromIndex(params[1]);
	char* szMessage;
	amx_StrParam(amx, params[3], szMessage);
	pNetGame->SendClientMessage(pidPlayer,params[2],szMessage);

	return 1;
}

//----------------------------------------------------------------------------------

// native SendPlayerMessageToPlayer(playerid, senderid, const message[])
static cell AMX_NATIVE_CALL n_SendPlayerMessageToPlayer(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "SendPlayerMessageToPlayer", 3);
	if (pNetGame->GetPlayerPool()->GetSlotState((BYTE)params[1])
		&& pNetGame->GetPlayerPool()->GetSlotState((BYTE)params[2]))
	{	
		char* szMessage;
		amx_StrParam(amx, params[3], szMessage);
		size_t uiLen = strlen(szMessage);
		
		RakNet::BitStream bsSend;
		bsSend.Write((BYTE)params[2]);
		bsSend.Write(uiLen);
		bsSend.Write(szMessage, uiLen);
		pNetGame->GetRakServer()->RPC(RPC_Chat, &bsSend, HIGH_PRIORITY, RELIABLE, 0,
			pNetGame->GetRakServer()->GetPlayerIDFromIndex((BYTE)params[1]), false, false);
		return 1;
	}
	return 0;
}

//----------------------------------------------------------------------------------

// native SendPlayerMessageToAll(senderid, const message[])
static cell AMX_NATIVE_CALL n_SendPlayerMessageToAll(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "SendPlayerMessageToAll", 2);
	if (pNetGame->GetPlayerPool()->GetSlotState((BYTE)params[1]))
	{	
		char* szMessage;
		amx_StrParam(amx, params[2], szMessage);
		size_t uiLen = strlen(szMessage);
		
		RakNet::BitStream bsSend;
		bsSend.Write((BYTE)params[1]);
		bsSend.Write(uiLen);
		bsSend.Write(szMessage, uiLen);
		pNetGame->GetRakServer()->RPC(RPC_Chat, &bsSend, HIGH_PRIORITY, RELIABLE, 0, UNASSIGNED_PLAYER_ID, true, false);
		return 1;
	}
	return 0;
}

//----------------------------------------------------------------------------------

// native SendClientMessageToAll(color, const message[])
static cell AMX_NATIVE_CALL n_SendClientMessageToAll(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "SendClientMessageToAll", 2);
	char* szMessage;
	amx_StrParam(amx, params[2], szMessage);
	pNetGame->SendClientMessageToAll(params[1],szMessage);

	return 1;
}

//----------------------------------------------------------------------------------

// native SendDeathMessage(killer,killee,weapon)
static cell AMX_NATIVE_CALL n_SendDeathMessage(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "SendDeathMessage", 3);
	RakServerInterface* pRak = pNetGame->GetRakServer();
	RakNet::BitStream bsDM;
	bsDM.Write((BYTE)params[1]);
	bsDM.Write((BYTE)params[2]);
	bsDM.Write((BYTE)params[3]);

	pRak->RPC(RPC_ScrDeathMessage , &bsDM, HIGH_PRIORITY, 
		RELIABLE, 0, UNASSIGNED_PLAYER_ID, true, false);

	return 1;
}

// native SendDeathMessageToPlayer(playerid, killer, killee, weapon)
static cell n_SendDeathMessageToPlayer(AMX* amx, cell* params)
{
	CHECK_PARAMS(amx, "SendDeathMessageToPlayer", 4);

	if (!pNetGame->GetPlayerPool()->GetSlotState(params[1]))
		return 0;

	RakNet::BitStream out;
	out.Write((BYTE)params[2]); // killer
	out.Write((BYTE)params[3]); // killee
	out.Write((BYTE)params[4]); // weapon

	return (cell)pNetGame->SendToPlayer(params[1], RPC_ScrDeathMessage, &out);
}

//----------------------------------------------------------------------------------

// native SetPlayerColor(playerid, color)
static cell AMX_NATIVE_CALL n_SetPlayerColor(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "SetPlayerColor", 2);
	CPlayer* pPlayer = pNetGame->GetPlayerPool()->GetAt((BYTE)params[1]);
	if(pPlayer) {
		pPlayer->SetPlayerColor(params[2]);
		return 1;
	}	

	return 0;
}

//----------------------------------------------------------------------------------

// native GetPlayerColor(playerid)
static cell AMX_NATIVE_CALL n_GetPlayerColor(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "GetPlayerColor", 1);

	CPlayer* pPlayer = pNetGame->GetPlayerPool()->GetAt((BYTE)params[1]);
	if(pPlayer) {
		return pPlayer->GetPlayerColor();
	}	

	return 0;
}

//----------------------------------------------------------------------------------

// native GetPlayerVehicleID(playerid)
static cell AMX_NATIVE_CALL n_GetPlayerVehicleID(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "GetPlayerVehicleID", 1);

	CPlayer *pPlayer = pNetGame->GetPlayerPool()->GetAt((BYTE)params[1]);
	if(pPlayer) {
		return pPlayer->m_VehicleID;
	}
	return 0;
}

//----------------------------------------------------------------------------------

// native SetPlayerCheckpoint(playerid, Float:x, Float:y, Float:z, Float:size)
static cell AMX_NATIVE_CALL n_SetPlayerCheckpoint(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "SetPlayerCheckpoint", 5);

	CPlayer *pPlayer = pNetGame->GetPlayerPool()->GetAt((BYTE)params[1]);
	if (pPlayer)
	{
		pPlayer->SetCheckpoint(amx_ctof(params[2]), amx_ctof(params[3]),
			amx_ctof(params[4]), amx_ctof(params[5]));

		return 1;
	}
	return 0;
}

//----------------------------------------------------------------------------------

// native DisablePlayerCheckpoint(playerid)
static cell AMX_NATIVE_CALL n_DisablePlayerCheckpoint(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "DisablePlayerCheckpoint", 1);

	CPlayer *pPlayer = pNetGame->GetPlayerPool()->GetAt((BYTE)params[1]);
	if (pPlayer)
	{
		pPlayer->ToggleCheckpoint(FALSE);

		return 1;
	}
	return 0;
}

//----------------------------------------------------------------------------------

// native IsPlayerInCheckpoint(playerid)
static cell AMX_NATIVE_CALL n_IsPlayerInCheckpoint(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "IsPlayerInCheckpoint", 1);

	CPlayer *pPlayer = pNetGame->GetPlayerPool()->GetAt((BYTE)params[1]);
	if (pPlayer)
	{
		return pPlayer->IsInCheckpoint();
	}
	return 0;
}

//----------------------------------------------------------------------------------

// native SetPlayerRaceCheckpoint(playerid, tpye, Float:x, Float:y, Float:z, Float:nx, Float:ny, Float:nz, Float:size)
static cell AMX_NATIVE_CALL n_SetPlayerRaceCheckpoint(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "SetPlayerRaceCheckpoint", 9);

	CPlayer *pPlayer = pNetGame->GetPlayerPool()->GetAt((BYTE)params[1]);
	if (pPlayer)
	{
		pPlayer->SetRaceCheckpoint(params[2], amx_ctof(params[3]),
			amx_ctof(params[4]), amx_ctof(params[5]), amx_ctof(params[6]),
			amx_ctof(params[7]), amx_ctof(params[8]), amx_ctof(params[9]));

		return 1;
	}
	return 0;
}

//----------------------------------------------------------------------------------

// native DisablePlayerRaceCheckpoint(playerid)
static cell AMX_NATIVE_CALL n_DisablePlayerRaceCheckpoint(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "DisablePlayerRaceCheckpoint", 1);

	CPlayer *pPlayer = pNetGame->GetPlayerPool()->GetAt((BYTE)params[1]);
	if (pPlayer)
	{
		pPlayer->ToggleRaceCheckpoint(FALSE);

		return 1;
	}
	return 0;
}

//----------------------------------------------------------------------------------

// native IsPlayerInRaceCheckpoint(playerid)
static cell AMX_NATIVE_CALL n_IsPlayerInRaceCheckpoint(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "IsPlayerInRaceCheckpoint", 1);

	CPlayer *pPlayer = pNetGame->GetPlayerPool()->GetAt((BYTE)params[1]);
	if (pPlayer)
	{
		return pPlayer->IsInRaceCheckpoint();
	}
	return 0;
}

//----------------------------------------------------------------------------------

// native GameTextForAll(strtext,displaytime,style)
static cell AMX_NATIVE_CALL n_GameTextForAll(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "GameTextForAll", 3);
	char *szMessage;
	int iLength;
	int iTime;
	int iStyle;

	amx_StrParam(amx, params[1], szMessage);
	iTime = params[2];
	iStyle = params[3];
	iLength = strlen(szMessage);

	if(!iLength) return 0;

	RakNet::BitStream bsParams;
	bsParams.Write(iStyle);
	bsParams.Write(iTime);
	bsParams.Write(iLength);
	bsParams.Write(szMessage,iLength);
	pNetGame->GetRakServer()->RPC(RPC_ScrDisplayGameText , &bsParams, HIGH_PRIORITY, 
		RELIABLE, 0, UNASSIGNED_PLAYER_ID, true, false);
	
	return 1;
}

//----------------------------------------------------------------------------------

// native GameTextForPlayer(playerid,strtext,displaytime,style)
static cell AMX_NATIVE_CALL n_GameTextForPlayer(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "GameTextForPlayer", 4);
	if (!pNetGame->GetPlayerPool()->GetSlotState((BYTE)params[1])) return 0;
	char *szMessage;
	int iLength;
	int iTime;
	int iStyle;

	amx_StrParam(amx, params[2], szMessage);
	iTime = params[3];
	iStyle = params[4];
	iLength = strlen(szMessage);

	if(!iLength) return 0;

	RakNet::BitStream bsParams;
	bsParams.Write(iStyle);
	bsParams.Write(iTime);
	bsParams.Write(iLength);
	bsParams.Write(szMessage,iLength);
	pNetGame->GetRakServer()->RPC(RPC_ScrDisplayGameText , &bsParams, HIGH_PRIORITY, 
		RELIABLE, 0, pNetGame->GetRakServer()->GetPlayerIDFromIndex(params[1]), false, false);

	return 1;
}

//----------------------------------------------------------------------------------

// native SetPlayerInterior(playerid,interiorid)
static cell AMX_NATIVE_CALL n_SetPlayerInterior(AMX *amx, cell *params)
{	
	CHECK_PARAMS(amx, "SetPlayerInterior", 2);
	if (!pNetGame->GetPlayerPool()->GetSlotState((BYTE)params[1])) return 0;
	RakNet::BitStream bsParams;
	BYTE byteInteriorID = (BYTE)params[2];
	bsParams.Write(byteInteriorID);

	pNetGame->GetRakServer()->RPC(RPC_ScrSetInterior , &bsParams, HIGH_PRIORITY, 
		RELIABLE, 0, pNetGame->GetRakServer()->GetPlayerIDFromIndex(params[1]), false, false);

	return 1;
}

//----------------------------------------------------------------------------------

// native GetPlayerInterior(playerid,interiorid)
static cell AMX_NATIVE_CALL n_GetPlayerInterior(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "GetPlayerInterior", 1);

	CPlayer *pPlayer = pNetGame->GetPlayerPool()->GetAt((BYTE)params[1]);
	if (pPlayer)
	{
		return pPlayer->m_iInteriorId;
	}
	return 0;
}

//----------------------------------------------------------------------------------
// native SetPlayerSpecialAction(playerid,actionid)

static cell AMX_NATIVE_CALL n_SetPlayerSpecialAction(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "SetPlayerSpecialAction", 2);
	if (!pNetGame->GetPlayerPool()->GetSlotState((BYTE)params[1])) return 0;
	RakNet::BitStream bsParams;
	bsParams.Write((BYTE)params[2]);
	pNetGame->GetRakServer()->RPC(RPC_ScrSetSpecialAction ,&bsParams, HIGH_PRIORITY, 
		RELIABLE, 0, pNetGame->GetRakServer()->GetPlayerIDFromIndex(params[1]), false, false);
	return 1;
}

//----------------------------------------------------------------------------------
// native GetPlayerSpecialAction(playerid)

static cell AMX_NATIVE_CALL n_GetPlayerSpecialAction(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "GetPlayerSpecialAction", 1);
	if (!pNetGame->GetPlayerPool()->GetSlotState((BYTE)params[1])) return SPECIAL_ACTION_NONE;
	CPlayer *pPlayer = pNetGame->GetPlayerPool()->GetAt((BYTE)params[1]);
	if(pPlayer) {
		if(pPlayer->GetState() == PLAYER_STATE_ONFOOT) {
			return pPlayer->GetSpecialAction();
		}
	}	
	return SPECIAL_ACTION_NONE;
}

// native GetPlayerDrunkLevel(playerid)
static cell AMX_NATIVE_CALL n_GetPlayerDrunkLevel(AMX* amx, cell* params)
{
	CHECK_PARAMS(amx, "GetPlayerDrunkLevel", 1);

	CPlayer* pPlayer = pNetGame->GetPlayerPool()->GetAt(params[1]);
	if (!pPlayer)
		return 0;

	return pPlayer->m_iDrunkLevel;
}

// native SetPlayerDrunkLevel(playerid, level)
static cell AMX_NATIVE_CALL n_SetPlayerDrunkLevel(AMX* amx, cell* params)
{
	CHECK_PARAMS(amx, "SetPlayerDrunkLevel", 2);

	CPlayer* pPlayer = pNetGame->GetPlayerPool()->GetAt(params[1]);
	if (!pPlayer)
		return 0;

	RakNet::BitStream out;
	out.Write<int>(1); // OP code
	out.Write((float)params[2]);
	pNetGame->SendToPlayer(params[1], RPC_ScrSetPlayer, &out);

	pPlayer->m_iDrunkLevel = params[2];
	return 1;
}

//----------------------------------------------------------------------------------

// native SetPlayerCameraPos(playerid, Float:x, Float:y, Float:z, Float:rx, Float:ry, Float:rz)
static cell AMX_NATIVE_CALL n_SetPlayerCameraPos(AMX *amx, cell *params)
{	
	CHECK_PARAMS(amx, "SetPlayerCameraPos", 7);

	if (!pNetGame->GetPlayerPool()->GetSlotState((BYTE)params[1])) return 0;

	RakNet::BitStream out;
	VECTOR vecPos, vecRot;

	vecPos.X = amx_ctof(params[2]);
	vecPos.Y = amx_ctof(params[3]);
	vecPos.Z = amx_ctof(params[4]);

	vecRot.X = amx_ctof(params[5]);
	vecRot.Y = amx_ctof(params[6]);
	vecRot.Z = amx_ctof(params[7]);

	out.Write(vecPos);
	out.Write(vecRot);

	pNetGame->SendToPlayer(params[1], RPC_ScrSetCameraPos, &out);
	return 1;
}

//----------------------------------------------------------------------------------

// native SetPlayerCameraLookAt(playerid,x,y,z)
static cell AMX_NATIVE_CALL n_SetPlayerCameraLookAt(AMX *amx, cell *params)
{	
	CHECK_PARAMS(amx, "SetPlayerCameraLookAt", 4);
	if (!pNetGame->GetPlayerPool()->GetSlotState((BYTE)params[1])) return 0;
	RakNet::BitStream bsParams;

	VECTOR vecPos;
	vecPos.X = amx_ctof(params[2]);
	vecPos.Y = amx_ctof(params[3]);
	vecPos.Z = amx_ctof(params[4]);
	
	bsParams.Write(vecPos.X);
	bsParams.Write(vecPos.Y);
	bsParams.Write(vecPos.Z);

	pNetGame->GetRakServer()->RPC(RPC_ScrSetCameraLookAt , &bsParams, HIGH_PRIORITY, 
		RELIABLE, 0, pNetGame->GetRakServer()->GetPlayerIDFromIndex(params[1]), false, false);

	return 1;
}

//----------------------------------------------------------------------------------

// native SetCameraBehindPlayer(playerid)
static cell AMX_NATIVE_CALL n_SetCameraBehindPlayer(AMX *amx, cell *params)
{	
	CHECK_PARAMS(amx, "SetCameraBehindPlayer", 1);
	if (!pNetGame->GetPlayerPool()->GetSlotState((BYTE)params[1])) return 0;
	RakNet::BitStream bsParams;
	RakServerInterface* pRak = pNetGame->GetRakServer();
	pRak->RPC(RPC_ScrSetCameraBehindPlayer , &bsParams, HIGH_PRIORITY, RELIABLE, 0,
		pRak->GetPlayerIDFromIndex(params[1]), false, false);

	return 1;
}

//----------------------------------------------------------------------------------

// native TogglePlayerControllable(playerid, toggle);
static cell AMX_NATIVE_CALL n_TogglePlayerControllable(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "TogglePlayerControllable", 2);
	if (!pNetGame->GetPlayerPool()->GetSlotState((BYTE)params[1])) return 0;

	RakNet::BitStream bsParams;
	RakServerInterface* pRak = pNetGame->GetRakServer();
	bsParams.Write((BYTE)params[2]);
	pRak->RPC(RPC_ScrTogglePlayerControllable , &bsParams, HIGH_PRIORITY, RELIABLE, 0,
		pRak->GetPlayerIDFromIndex(params[1]), false, false);

	return 1;
}

//----------------------------------------------------------------------------------

// native SetVehicleParametersForPlayer(vehicleid,playerid,objective,doorslocked)
static cell AMX_NATIVE_CALL n_SetVehicleParamsForPlayer(AMX *amx, cell *params)
{	
	CHECK_PARAMS(amx, "SetVehicleParametersForPlayer", 4);
	if (!pNetGame->GetPlayerPool()->GetSlotState((BYTE)params[2]) ||
		!pNetGame->GetVehiclePool()->GetSlotState((VEHICLEID)params[1])) return 0;
	RakNet::BitStream bsParams;
	//VehicleID = (VEHICLEID)params[1];
	//byteObjectiveVehicle = (BYTE)params[3];
	//byteDoorsLocked = (BYTE)params[4];

	bsParams.Write((VEHICLEID)params[1]);
	bsParams.Write((BYTE)params[3]);
	bsParams.Write((BYTE)params[4]);

	pNetGame->GetRakServer()->RPC(RPC_ScrVehicleParams , &bsParams, HIGH_PRIORITY, 
		RELIABLE, 0, pNetGame->GetRakServer()->GetPlayerIDFromIndex(params[2]), false, false);

	return 1;
}

//----------------------------------------------------------------------------------
// native SetPlayerScore(playerid,score)

static cell AMX_NATIVE_CALL n_SetPlayerScore(AMX *amx, cell *params)
{	
	CHECK_PARAMS(amx, "SetPlayerScore", 2);
	BYTE bytePlayerID = (BYTE)params[1];
	int iScore = (int)params[2];

	if(pNetGame->GetPlayerPool()->GetSlotState(bytePlayerID)) {
		pNetGame->GetPlayerPool()->SetPlayerScore(bytePlayerID,iScore);
		return 1;
	}

	return 0;
}

//----------------------------------------------------------------------------------
// native GetPlayerScore(playerid)

static cell AMX_NATIVE_CALL n_GetPlayerScore(AMX *amx, cell *params)
{	
	CHECK_PARAMS(amx, "GetPlayerScore", 1);
	BYTE bytePlayerID = (BYTE)params[1];
	if (!pNetGame->GetPlayerPool()->GetSlotState(bytePlayerID)) return 0;

	return pNetGame->GetPlayerPool()->GetPlayerScore(bytePlayerID);
}

//----------------------------------------------------------------------------------
// native GivePlayerMoney(playerid,money)

static cell AMX_NATIVE_CALL n_GivePlayerMoney(AMX *amx, cell *params)
{	
	CHECK_PARAMS(amx, "GivePlayerMoney", 2);
	RakNet::BitStream bsMoney;
	bsMoney.Write((int)params[2]);

	CPlayerPool *pPool = pNetGame->GetPlayerPool();

	if( pPool->GetSlotState((BYTE)params[1]) ) {
		pPool->SetPlayerMoney((BYTE)params[1], pPool->GetPlayerMoney((BYTE)params[1]) + params[2]);

		pNetGame->GetRakServer()->RPC(RPC_ScrHaveSomeMoney , &bsMoney, HIGH_PRIORITY, 
			RELIABLE, 0, pNetGame->GetRakServer()->GetPlayerIDFromIndex(params[1]), false, false);
		return 1;
	}

	return 0;
}

static cell n_SetPlayerMoney(AMX* amx, cell* params)
{
	CHECK_PARAMS(amx, "SetPlayerMoney", 2);
	CPlayerPool* pPlayerPool = pNetGame->GetPlayerPool();
	if (pPlayerPool && pPlayerPool->GetSlotState(params[1]))
	{
		pPlayerPool->SetPlayerMoney(params[1], params[2]);
		return 1;
	}
	return 0;
}

//----------------------------------------------------------------------------------
// native GetPlayerMoney(playerid)

static cell AMX_NATIVE_CALL n_GetPlayerMoney(AMX *amx, cell *params)
{	
	CHECK_PARAMS(amx, "GetPlayerMoney", 1);
	BYTE bytePlayerID = (BYTE)params[1];
	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();

	if(pPlayerPool->GetSlotState(bytePlayerID)) {
		return pPlayerPool->GetPlayerMoney(bytePlayerID);
	}

	return 0;
}

//----------------------------------------------------------------------------------
// native ResetPlayerMoney(playerid)

static cell AMX_NATIVE_CALL n_ResetPlayerMoney(AMX *amx, cell *params)
{		
	CHECK_PARAMS(amx, "ResetPlayerMoney", 1);
	RakNet::BitStream bsMoney;

	BYTE bytePlayerID = (BYTE)params[1];
	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();

	if(pPlayerPool->GetSlotState(bytePlayerID)) {
		pPlayerPool->SetPlayerMoney(bytePlayerID,0);
		pNetGame->GetRakServer()->RPC(RPC_ScrResetMoney , &bsMoney, HIGH_PRIORITY, 
			RELIABLE, 0, pNetGame->GetRakServer()->GetPlayerIDFromIndex(bytePlayerID), false, false);
		return 1;
	}

	return 0;
}

//----------------------------------------------------------------------------------
// native GetPlayerAmmo(playerid)

static cell AMX_NATIVE_CALL n_GetPlayerAmmo(AMX *amx, cell *params)
{	
	CHECK_PARAMS(amx, "GetPlayerAmmo", 1);
	BYTE bytePlayerID = (BYTE)params[1];
	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();

	if(pPlayerPool->GetSlotState(bytePlayerID)) {
		return pPlayerPool->GetPlayerAmmo(bytePlayerID);
	}

	return 0;
}

//----------------------------------------------------------------------------------
// native SetPlayerAmmo(playerid, weaponslot, ammo)

static cell AMX_NATIVE_CALL n_SetPlayerAmmo(AMX *amx, cell *params)
{	
	CHECK_PARAMS(amx, "SetPlayerAmmo", 3);
	BYTE bytePlayerID = (BYTE)params[1];
	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();

	if(pPlayerPool->GetSlotState(bytePlayerID))
	{	
		RakNet::BitStream bsAmmo;
		bsAmmo.Write((BYTE)params[2]);
		bsAmmo.Write((WORD)params[3]);

		pNetGame->GetRakServer()->RPC(RPC_ScrSetWeaponAmmo , &bsAmmo, HIGH_PRIORITY, 
			RELIABLE, 0, pNetGame->GetRakServer()->GetPlayerIDFromIndex(bytePlayerID), false, false);
		return 1;
	}
	return 0;
}

//----------------------------------------------------------------------------------
// native GetPlayerWeaponData(playerid,slot,  &weapon, &ammo)

static cell AMX_NATIVE_CALL n_GetPlayerWeaponData(AMX *amx, cell *params)
{	
	CHECK_PARAMS(amx, "GetPlayerWeaponData", 4);
	BYTE bIndex = (BYTE)params[2];
	if (bIndex >= 13) return 0;

	BYTE bytePlayerID = (BYTE)params[1];
	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
	//cell cWeapons[13];
	//cell cAmmo[13];
	

	if(pPlayerPool->GetSlotState(bytePlayerID)) {
		//return pPlayerPool->GetPlayerAmmo(bytePlayerID);
		//WEAPON_SLOT_TYPE* WeaponSlots = pPlayerPool->GetAt(bytePlayerID)->GetWeaponSlotsData();
		CPlayer *pPlayer =  pPlayerPool->GetAt(bytePlayerID);
		/*BYTE i;
		for (i = 0; i < 13; i++)
		{
			cWeapons[i] = (cell)pPlayer->m_dwSlotWeapon[i];
			cAmmo[i] = (cell)pPlayer->m_dwSlotAmmo[i];
		}*/
		cell* cptr;
		
		amx_GetAddr(amx, params[3], &cptr);
		*cptr = (cell)pPlayer->m_byteSlotWeapon[bIndex];
		amx_GetAddr(amx, params[4], &cptr);
		*cptr = (cell)pPlayer->m_dwSlotAmmo[bIndex];
		/*amx_GetAddr(amx, params[4], &cptr);
		*cptr = (int)((in.s_addr & 0x0000FF00) >> 8);
		amx_GetAddr(amx, params[5], &cptr);
		*cptr = (int)((in.s_addr & 0x000000FF));
		params[3] = (cell)pPlayer->m_dwSlotWeapon[bIndex]; //(cell)&cWeapons[0];
		params[4] = (cell)pPlayer->m_dwSlotAmmo[bIndex]; //(cell)&cAmmo[0];*/
		
		return 1;
	}

	return 0;
}

//----------------------------------------------------------------------------------
// native IsPlayerConnected(playerid)

static cell AMX_NATIVE_CALL n_IsPlayerConnected(AMX *amx, cell *params)
{	
	CHECK_PARAMS(amx, "IsPlayerConnected", 1);
	BYTE bytePlayerID = (BYTE)params[1];
	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();

	if (pPlayerPool->GetSlotState(bytePlayerID)) {
		return 1;
	}

	return 0;
}

//----------------------------------------------------------------------------------
// native GetPlayerState(playerid)

static cell AMX_NATIVE_CALL n_GetPlayerState(AMX *amx, cell *params)
{	
	CHECK_PARAMS(amx, "GetPlayerState", 1);
	BYTE bytePlayerID = (BYTE)params[1];
	CPlayer *pPlayer = pNetGame->GetPlayerPool()->GetAt(bytePlayerID);

	if(pPlayer) {
		return pPlayer->GetState();
	}

	return PLAYER_STATE_NONE;
}

//----------------------------------------------------------------------------------
// native SetPlayerFacingAngle(playerid,Float:ang);

static cell AMX_NATIVE_CALL n_SetPlayerFacingAngle(AMX *amx, cell *params)
{	
	CHECK_PARAMS(amx, "SetPlayerFacingAngle", 2);

	if (!pNetGame->GetPlayerPool()->GetSlotState((BYTE)params[1])) return 0;

	RakNet::BitStream bsFace;
	float fFace = amx_ctof(params[2]);
	bsFace.Write(fFace);
	pNetGame->GetRakServer()->RPC(RPC_ScrSetPlayerFacingAngle , &bsFace, HIGH_PRIORITY, 
		RELIABLE, 0, pNetGame->GetRakServer()->GetPlayerIDFromIndex(params[1]), false, false);

	return 1;
}

//----------------------------------------------------------------------------------
// native GetPlayerFacingAngle(playerid,&Float:ang);

static cell AMX_NATIVE_CALL n_GetPlayerFacingAngle(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "GetPlayerFacingAngle", 2);

	CPlayer* pPlayer = pNetGame->GetPlayerPool()->GetAt((BYTE)params[1]);

	if (pPlayer)
	{
		cell* cptr;
		amx_GetAddr(amx, params[2], &cptr);
		*cptr = amx_ftoc(pPlayer->m_fRotation);

		return 1;
	} else {
		return 0;
	}
}

//----------------------------------------------------------------------------------
// native ResetPlayerWeapons(playerid);

static cell AMX_NATIVE_CALL n_ResetPlayerWeapons(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "ResetPlayerWeapons", 1);

	if (!pNetGame->GetPlayerPool()->GetSlotState((BYTE)params[1])) return 0;

	RakNet::BitStream bsData;
	pNetGame->GetRakServer()->RPC(RPC_ScrResetPlayerWeapons , &bsData, HIGH_PRIORITY, 
		RELIABLE, 0, pNetGame->GetRakServer()->GetPlayerIDFromIndex(params[1]), false, false);
	return 1;
}

//----------------------------------------------------------------------------------
// native GivePlayerWeapon(playerid,weaponid,ammo);

static cell AMX_NATIVE_CALL n_GivePlayerWeapon(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "GivePlayerWeapon", 3);

	if (!pNetGame->GetPlayerPool()->GetSlotState((BYTE)params[1])) return 0;


	RakNet::BitStream bsData;
	bsData.Write((int)params[2]); // weaponid
	bsData.Write((int)params[3]); // ammo
	pNetGame->GetRakServer()->RPC(RPC_ScrGivePlayerWeapon , &bsData, HIGH_PRIORITY, 
		RELIABLE, 0, pNetGame->GetRakServer()->GetPlayerIDFromIndex(params[1]), false, false);
	return 1;
}

//----------------------------------------------------------------------------------
// native SetWorldTime(hour)

static cell AMX_NATIVE_CALL n_SetWorldTime(AMX *amx, cell *params)
{	
	CHECK_PARAMS(amx, "SetWorldTime", 1);
	BYTE byteHour = (BYTE)params[1];
	pNetGame->SetWorldTime(byteHour);
	return 1;
}

//----------------------------------------------------------------------------------
// native SetPlayerTime(playerid, hour, min)

static cell AMX_NATIVE_CALL n_SetPlayerTime(AMX *amx, cell *params)
{	
	CHECK_PARAMS(amx, "SetPlayerTime", 3);

	CPlayer* pPlayer = pNetGame->GetPlayerPool()->GetAt((BYTE)params[1]);
	if(!pPlayer) return 0;	
	
	pPlayer->SetTime((BYTE)params[2], (BYTE)params[3]);

	return 1;
}

//----------------------------------------------------------------------------------
// native GetPlayerTime(playerid, &hour, &minute)

static cell AMX_NATIVE_CALL n_GetPlayerTime(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "GetPlayerTime", 3);

	CPlayer* pPlayer = pNetGame->GetPlayerPool()->GetAt((BYTE)params[1]);

	if (pPlayer)
	{
		int iTime;
		if (pPlayer->m_byteTime)
		{
			iTime = (int)pPlayer->m_fGameTime;
		}
		else
		{
			iTime = ((int)pNetGame->m_byteWorldTime) * 60;
		}

		cell* cptr;
			
		amx_GetAddr(amx, params[2], &cptr);
		*cptr = (cell)(iTime / 60);
		amx_GetAddr(amx, params[3], &cptr);
		*cptr = (cell)(iTime % 60);

		return 1;
	} else {
		return 0;
	}
}

//----------------------------------------------------------------------------------
// native TogglePlayerClock(playerid, toggle)

static cell AMX_NATIVE_CALL n_TogglePlayerClock(AMX *amx, cell *params)
{	
	CHECK_PARAMS(amx, "TogglePlayerClock", 2);

	CPlayer* pPlayer = pNetGame->GetPlayerPool()->GetAt((BYTE)params[1]);
	if(!pPlayer) return 0;	
	
	pPlayer->SetClock((BYTE)params[2]);
	return 1;
}

//----------------------------------------------------------------------------------

// native print(const string[])
static cell AMX_NATIVE_CALL n_print(AMX* amx, cell* params)
{
	CHECK_PARAMS(amx, "print", 1);

	char* msg;
	amx_StrParam(amx, params[1], msg);
	logprintf("%s",msg);
	return 0;
}

//----------------------------------------------------------------------------------


// native printf(const format[], {Float,_}:...)
static cell AMX_NATIVE_CALL n_printf(AMX *amx, cell *params)
{
	CHECK_PARAMS_BETWEEN(amx, "printf", 1, 51);
	int len;
	logprintf("%s",format_amxstring(amx, params, 1, len));

	return 0;
}

/*

#define MAX_FORMATSTR 256

int amx_printstring(AMX *amx,cell *cstr,AMX_FMTINFO *info);

static int str_putstr(void *dest,TCHAR *str)
{
	if (_tcslen((TCHAR*)dest)+_tcslen(str)<MAX_FORMATSTR)
		_tcscat((TCHAR*)dest,str);
	return 0;
}

static int str_putchar(void *dest,TCHAR ch)
{
	int len=_tcslen((TCHAR*)dest);
	if (len<MAX_FORMATSTR-1)
	{
		((TCHAR*)dest)[len]   = ch;
		((TCHAR*)dest)[len+1] = '\0';
	}
	return 0;
}

// native strformat(const format[], {Fixed,_}:...)
static cell AMX_NATIVE_CALL n_printf(AMX *amx, cell *params)
{
	cell *cstr;
	AMX_FMTINFO info;
	TCHAR output[MAX_FORMATSTR];

	memset(&info,0,sizeof info);
	info.params=params+5;
	info.numparams=(int)(params[0]/sizeof(cell))-4;
	info.skip=0;
	info.length=MAX_FORMATSTR;  // max. length of the string
	info.f_putstr=str_putstr;
	info.f_putchar=str_putchar;
	info.user=output;
	output[0] = __T('\0');

	amx_GetAddr(amx,params[4],&cstr);
	amx_printstring(amx,cstr,&info);

	// store the output string
	amx_GetAddr(amx,params[1],&cstr);
	amx_SetString(cstr,(char*)output,(int)params[3],sizeof(TCHAR)>1,(int)params[2]);
	return 1;
}
*/

//----------------------------------------------------------------------------------

// native format(output[], len, const format[], {Float,_}:...)
static cell AMX_NATIVE_CALL n_format(AMX *amx, cell *params)
{
	CHECK_PARAMS_BETWEEN(amx, "format", 3, 53);
	int len;
	return set_amxstring(amx, params[1], format_amxstring(amx, params, 3, len), params[2]);
}

//----------------------------------------------------------------------------------

// native SetTimer(funcname[], interval, repeating)
static cell AMX_NATIVE_CALL n_SetTimer(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "SetTimer", 3);

	char* szFuncName;
	amx_StrParam(amx, params[1], szFuncName);

	if (szFuncName == 0)
		return 0;

	return pNetGame->GetTimers()->New(szFuncName, params[2], params[3], amx);
}

//----------------------------------------------------------------------------------

// native KillTimer(timerid)
static cell AMX_NATIVE_CALL n_KillTimer(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "KillTimer", 1);

	pNetGame->GetTimers()->Kill(params[1]);

	return 1;
}

// native SetMaxRconAttempt(max_attempt)
static cell n_SetMaxRconLoginAttempt(AMX* amx, cell* params)
{
	CHECK_PARAMS(amx, "SetMaxRconAttempt", 1);
	if (0 < params[1])
	{
		pNetGame->m_uiMaxRconAttempt = params[1];
		return 1;
	}
	return 0;
}

//----------------------------------------------------------------------------------

// native GetTickCount()
static cell AMX_NATIVE_CALL n_GetTickCount(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "GetTickCount", 0);

	return (cell)GetTickCount();
}

//----------------------------------------------------------------------------------
// native GetMaxPlayers()
static cell AMX_NATIVE_CALL n_GetMaxPlayers(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "GetMaxPlayers", 0);
	
	extern CConsole *pConsole;
	return pConsole->GetIntVariable("maxplayers");
}

//----------------------------------------------------------------------------------
// native SetMaxPlayers(maxplayers)
/*static cell AMX_NATIVE_CALL n_SetMaxPlayers(AMX *amx, cell *params)
{
	CHECK_PARAMS(1);
	extern CConsole *pConsole;
	pConsole->SetIntVariable("maxplayers", (int)params[1]);
    return 1;
}*/

//----------------------------------------------------------------------------------
// native GetMaxPlayers()
static cell AMX_NATIVE_CALL n_LimitGlobalChatRadius(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "LimitGlobalChatRadius", 1);
	float fRadius = amx_ctof(params[1]);

	pNetGame->m_bLimitGlobalChatRadius = TRUE;
	pNetGame->m_fGlobalChatRadius = fRadius;
	
	return 1;
}

static cell n_LimitPlayerMarkerRadius(AMX* amx, cell* params)
{
	CHECK_PARAMS(amx, "LimitPlayerMarkerRadius", 1);

	pNetGame->m_bLimitGlobalMarkerRadius = TRUE;
	pNetGame->m_fGlobalMarkerRadius = amx_ctof(params[1]);
	return 1;
}

//----------------------------------------------------------------------------------

// native GetVehicleZAngle(vehicleid, &Float:z_angle)
static cell AMX_NATIVE_CALL n_GetVehicleZAngle(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "GetVehicleZAngle", 2);

	CVehicle* pVehicle = pNetGame->GetVehiclePool()->GetAt((VEHICLEID)params[1]);

	if (pVehicle)
	{
		float fZAngle = atan2(-pVehicle->m_matWorld.up.X, pVehicle->m_matWorld.up.Y) * 180.0f/PI;
		
		// Bound it to [0, 360)
		while(fZAngle < 0.0f) 
			fZAngle += 360.0f;
		while(fZAngle >= 360.0f) 
			fZAngle -= 360.0f;
		
		cell* cptr;
		amx_GetAddr(amx, params[2], &cptr);
		*cptr = amx_ftoc(fZAngle);
		return 1;
	} else {
		return 0;
	}
}

//----------------------------------------------------------------------------------

// native SetVehicleZAngle(vehicleid, Float:z_angle)
static cell AMX_NATIVE_CALL n_SetVehicleZAngle(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "SetVehicleZAngle", 2);

	CVehicle* pVehicle = pNetGame->GetVehiclePool()->GetAt((VEHICLEID)params[1]);

	if (pVehicle)
	{
		if (pVehicle->m_byteDriverID != INVALID_ID)
		{
			RakNet::BitStream bsParams;
			bsParams.Write((VEHICLEID)params[1]);
			bsParams.Write(amx_ctof(params[2]));

			RakServerInterface* pRak = pNetGame->GetRakServer();
			pRak->RPC(RPC_ScrSetVehicleZAngle , &bsParams, HIGH_PRIORITY, 
				RELIABLE, 0, pRak->GetPlayerIDFromIndex(pVehicle->m_byteDriverID), false, false);
		}
		return 1;
	} else {
		return 0;
	}
}

//----------------------------------------------------------------------------------

// native PlayerPlaySound(playerid, soundid, Float:x, Float:y, Float:z)
static cell AMX_NATIVE_CALL n_PlayerPlaySound(AMX *amx, cell *params)
{	
	CHECK_PARAMS(amx, "PlayerPlaySound", 5);

	if (!pNetGame->GetPlayerPool()->GetSlotState((BYTE)params[1])) return 0;

	RakNet::BitStream bsParams;
	
	bsParams.Write(params[2]);
	bsParams.Write(amx_ctof(params[3]));
	bsParams.Write(amx_ctof(params[4]));
	bsParams.Write(amx_ctof(params[5]));

	pNetGame->GetRakServer()->RPC(RPC_ScrPlaySound , &bsParams, HIGH_PRIORITY, 
		RELIABLE, 0, pNetGame->GetRakServer()->GetPlayerIDFromIndex(params[1]), false, false);

	return 1;
}

//----------------------------------------------------------------------------------

// native ShowNameTags(show)
static cell AMX_NATIVE_CALL n_ShowNameTags(AMX *amx, cell *params)
{	
	CHECK_PARAMS(amx, "ShowNameTags", 1);
	pNetGame->m_bShowNameTags = (bool)params[1];
	return 1;
}

//----------------------------------------------------------------------------------

// native ShowPlayerNameTagForPlayer(playerid, showplayerid, show)
static cell AMX_NATIVE_CALL n_ShowPlayerNameTagForPlayer(AMX *amx, cell *params)
{	
	CHECK_PARAMS(amx, "ShowPlayerNameTagForPlayer", 3);

	if (!pNetGame->GetPlayerPool()->GetSlotState((BYTE)params[1]) ||
		!pNetGame->GetPlayerPool()->GetSlotState((BYTE)params[2])) return 0;

	RakNet::BitStream bsParams;
	
	bsParams.Write((BYTE)params[2]);
	bsParams.Write((BYTE)params[3]);

	pNetGame->GetRakServer()->RPC(RPC_ScrShowNameTag , &bsParams, HIGH_PRIORITY, 
		RELIABLE, 0, pNetGame->GetRakServer()->GetPlayerIDFromIndex(params[1]), false, false);
	return 1;
}

//----------------------------------------------------------------------------------

// native ShowPlayerMarkers(show)
static cell AMX_NATIVE_CALL n_ShowPlayerMarkers(AMX *amx, cell *params)
{	
	CHECK_PARAMS(amx, "ShowPlayerMarkers", 1);
	pNetGame->m_bShowPlayerMarkers = (bool)params[1];
	return 1;
}

//----------------------------------------------------------------------------------

// native AllowInteriorWeapons(allow)
static cell AMX_NATIVE_CALL n_AllowInteriorWeapons(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "AllowInteriorWeapons", 1);
	pNetGame->m_bAllowWeapons = (bool)params[1];
	return 1;
}

//----------------------------------------------------------------------------------

// native UsePlayerPedAnims()
static cell AMX_NATIVE_CALL n_UsePlayerPedAnims(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "UsePlayerPedAnims", 0);
	pNetGame->m_bUseCJWalk = TRUE;
	return 1;
}

//----------------------------------------------------------------------------------

// native GetPlayerIP(playerid, const ip[], len)
static cell AMX_NATIVE_CALL n_GetPlayerIp(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "GetPlayerIp", 3);
	
	if (pNetGame->GetPlayerPool()->GetSlotState((BYTE)params[1])) {
		RakServerInterface* pRak = pNetGame->GetRakServer();
		PlayerID Player = pRak->GetPlayerIDFromIndex(params[1]);

		in_addr in;
		in.s_addr = Player.binaryAddress;
		return set_amxstring(amx, params[2], inet_ntoa(in), params[3]);
	} else { return -1; }
}
//----------------------------------------------------------------------------------

// native GetPlayerPing(playerid)
static cell AMX_NATIVE_CALL n_GetPlayerPing(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "GetPlayerPing", 1);
	
	if (pNetGame->GetPlayerPool()->GetSlotState((BYTE)params[1])) {
		RakServerInterface* pRak = pNetGame->GetRakServer();
		PlayerID Player = pRak->GetPlayerIDFromIndex(params[1]);

		return pRak->GetLastPing(Player);
	} else { return -1; }
}
//----------------------------------------------------------------------------------

// native GetPlayerWeapon(playerid)
static cell AMX_NATIVE_CALL n_GetPlayerWeapon(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "GetPlayerWeapon", 1);
	CPlayer* pPlayer = pNetGame->GetPlayerPool()->GetAt((BYTE)params[1]);
	if(!pPlayer) return -1;	
	BYTE byteState = pPlayer->GetState();
	if ((byteState != PLAYER_STATE_DRIVER) || (byteState != PLAYER_STATE_PASSENGER))
	{
		return pPlayer->GetCurrentWeapon();
	} else { return 0; }
	
}

// native SetTimerEx(funcname[], interval, repeating, parameter)
static cell AMX_NATIVE_CALL n_SetTimerEx(AMX *amx, cell *params)
{
	CHECK_PARAMS_BETWEEN(amx, "SetTimerEx", 4, 54);

	char* szFuncName;
	amx_StrParam(amx, params[1], szFuncName);

	if (szFuncName == 0)
		return 0;

	return pNetGame->GetTimers()->NewEx(szFuncName, params[2], params[3], params, amx);
}
	
//----------------------------------------------------

// native VectorSize(Float:x, Float:y, Float:z)
static cell n_VectorSize(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "VectorSize", 3);
	float x = amx_ctof(params[1]);
	float y = amx_ctof(params[2]);
	float z = amx_ctof(params[3]);
	float r = sqrt(x * x + y * y + z * z);
	return amx_ftoc(r);
}

// native SendRconCommand(command[])
static cell AMX_NATIVE_CALL n_SendRconCommand( AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "SendRconCommand", 1);
	extern CConsole *pConsole;
	char* szCommand;
	amx_StrParam(amx, params[1], szCommand);
	pConsole->Execute(szCommand);
	return 1;
}

//----------------------------------------------------
// native GetPlayerArmour(playerid, &Float:armour)
static cell AMX_NATIVE_CALL n_GetPlayerArmour(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "GetPlayerArmour", 2);

	CPlayer* pPlayer = pNetGame->GetPlayerPool()->GetAt((BYTE)params[1]);

	if (pPlayer)
	{
		cell* cptr;
		amx_GetAddr(amx, params[2], &cptr);
		*cptr = amx_ftoc(pPlayer->m_fArmour);

		return 1;
	} else {
		return 0;
	}
}

//----------------------------------------------------
// native SetPlayerArmour(playerid,Float:armour)

static cell AMX_NATIVE_CALL n_SetPlayerArmour(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "SetPlayerArmour", 2);

	if (!pNetGame->GetPlayerPool()->GetSlotState((BYTE)params[1])) return 0;
	float fArmour = amx_ctof(params[2]);

	RakNet::BitStream bsArmour;
	bsArmour.Write(fArmour);

	pNetGame->GetRakServer()->RPC(RPC_ScrSetPlayerArmour , &bsArmour, HIGH_PRIORITY, 
		RELIABLE, 0, pNetGame->GetRakServer()->GetPlayerIDFromIndex(params[1]), false, false);

	return 1;
}

//-----------------------------------------------------
// native SetPlayerMarkerForPlayer(playerid, showplayerid, color)
// Sets a players radar blip color for another player
static cell AMX_NATIVE_CALL n_SetPlayerMarkerForPlayer(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "SetPlayerMarkerForPlayer", 3);

	if (pNetGame->GetPlayerPool()->GetAt((BYTE)params[1]) &&
		pNetGame->GetPlayerPool()->GetAt((BYTE)params[2]))
	{
		RakNet::BitStream bsMarker;
		bsMarker.Write((BYTE)params[2]);
		bsMarker.Write((DWORD)params[3]);

		
		pNetGame->GetRakServer()->RPC(RPC_ScrSetPlayerColor , &bsMarker, HIGH_PRIORITY, 
			RELIABLE, 0, pNetGame->GetRakServer()->GetPlayerIDFromIndex(params[1]), false, false);

		return 1;
	}
	else
	{
		return 0;
	}
}

//-----------------------------------------------------
// native SetPlayerMapIcon(playerid, iconid, Float:x, Float:y, Float:z, icontype, color)
static cell AMX_NATIVE_CALL n_SetPlayerMapIcon(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "SetPlayerMapIcon", 7); // Playerid,
	if ((BYTE)params[2] >= 32) return 0;
	
	//CPlayer* pPlayer = pNetGame->GetPlayerPool()->GetAt((BYTE)params[1]);

	RakNet::BitStream bsIcon;
	//float fPos[3];
	
	CPlayer* pPlayer = pNetGame->GetPlayerPool()->GetAt((BYTE)params[1]);
	if(!pPlayer) return 0;

	//fPos[0] = amx_ctof(params[3]);
	//fPos[1] = amx_ctof(params[4]);
	//fPos[2] = amx_ctof(params[5]);
	
	bsIcon.Write((BYTE)params[2]);
	bsIcon.Write(amx_ctof(params[3])); //fPos[0]);
	bsIcon.Write(amx_ctof(params[4])); //fPos[1]);
	bsIcon.Write(amx_ctof(params[5])); //fPos[2]);
	bsIcon.Write((BYTE)params[6]);
	bsIcon.Write((BYTE)params[7]);

	RakServerInterface* pRak = pNetGame->GetRakServer();
	pRak->RPC(RPC_ScrSetMapIcon , &bsIcon, HIGH_PRIORITY, 
		RELIABLE, 0, pRak->GetPlayerIDFromIndex(params[1]), false, false);

	return 1;
}

//-----------------------------------------------------
// native RemovePlayerMapIcon(playerid, iconid)
static cell AMX_NATIVE_CALL n_RemovePlayerMapIcon(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "RemovePlayerMapIcon", 2); // Playerid, 
	
	//CPlayer* pPlayer = pNetGame->GetPlayerPool()->GetAt((BYTE)params[1]);
	if(!pNetGame->GetPlayerPool()->GetAt((BYTE)params[1])) return 0;
	// Not technically needed but adds checking incase they're not in the server (not actually sure if it'll matter)
	
	RakNet::BitStream bsIcon;
	bsIcon.Write((BYTE)params[2]);

	RakServerInterface* pRak = pNetGame->GetRakServer();
	pRak->RPC(RPC_ScrDisableMapIcon , &bsIcon, HIGH_PRIORITY, 
		RELIABLE, 0, pRak->GetPlayerIDFromIndex(params[1]), false, false);

	return 1;
}

//----------------------------------------------------------------------------------
// native GetPlayerKeys(playerid);

static cell AMX_NATIVE_CALL n_GetPlayerKeys(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "GetPlayerKeys", 4);

	CPlayer * pPlayer = pNetGame->GetPlayerPool()->GetAt((BYTE)params[1]);
	if (pPlayer) {
		WORD wKeys, udAnalog, lrAnalog;
		int iPlayerState = pPlayer->GetState();
		switch (iPlayerState)
		{
		case PLAYER_STATE_ONFOOT:
			wKeys = pPlayer->GetOnFootSyncData()->wKeys;
			udAnalog = (short)pPlayer->GetOnFootSyncData()->udAnalog;
			lrAnalog = (short)pPlayer->GetOnFootSyncData()->lrAnalog;
			break;

		case PLAYER_STATE_DRIVER:
			wKeys = pPlayer->GetInCarSyncData()->wKeys;
			udAnalog = (short)pPlayer->GetInCarSyncData()->udAnalog;
			lrAnalog = (short)pPlayer->GetInCarSyncData()->lrAnalog;
			break;

		case PLAYER_STATE_PASSENGER:
			wKeys = pPlayer->GetPassengerSyncData()->wKeys;
			udAnalog = (short)pPlayer->GetPassengerSyncData()->udAnalog;
			lrAnalog = (short)pPlayer->GetPassengerSyncData()->lrAnalog;
			break;

		case PLAYER_STATE_SPECTATING:
			wKeys = pPlayer->GetSpectatorSyncData()->wKeys;
			udAnalog = (short)pPlayer->GetSpectatorSyncData()->udAnalog;
			lrAnalog = (short)pPlayer->GetSpectatorSyncData()->lrAnalog;
			break;

		default:
			return 0;
		}
		cell* cptr;
		amx_GetAddr(amx, params[2], &cptr);
		*cptr = (cell)wKeys;
		amx_GetAddr(amx, params[3], &cptr);
		*cptr = (cell)udAnalog;
		amx_GetAddr(amx, params[4], &cptr);
		*cptr = (cell)lrAnalog;
		return 1;
	}
	return 0;
}

//----------------------------------------------------------------------------------

// native EnableTirePopping(enable)
static cell AMX_NATIVE_CALL n_EnableTirePopping(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "EnableTirePopping", 1);
	pNetGame->m_bTirePopping = (bool)params[1];
	//char szPopping[128];
	//sprintf(szPopping, "%s", (params[1] == 1) ? "True" : "False");
	//pConsole->SetStringVariable("tirepopping", szPopping);
	// Removed - 
	return 1;
}

//----------------------------------------------------------------------------------

// native SetWeather(weather)
static cell AMX_NATIVE_CALL n_SetWeather(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "SetWeather", 1);

	
	pNetGame->SetWeather((BYTE)params[1]);

	//pNetGame->SendClientMessageToAll(0xFFFFFFFF, "Weather changed");
	// Removed - annoying for a release

	return 1;
}

//----------------------------------------------------------------------------------

// native SetPlayerWeather(playerid, weather)
static cell AMX_NATIVE_CALL n_SetPlayerWeather(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "SetPlayerWeather", 2);

	if (!pNetGame->GetPlayerPool()->GetSlotState((BYTE)params[1])) return 0;
	RakNet::BitStream bsWeather;
	bsWeather.Write((BYTE)params[2]);
	RakServerInterface* pRak = pNetGame->GetRakServer();
	pRak->RPC(RPC_Weather , &bsWeather, HIGH_PRIORITY, RELIABLE, 0, pRak->GetPlayerIDFromIndex(params[1]), false, false);
	//pRak->RPC(RPC_ScrDisableMapIcon", &bsIcon, HIGH_PRIORITY, RELIABLE, 0, pRak->GetPlayerIDFromIndex(params[1]), false, false);

	return 1;
}

//----------------------------------------------------------------------------------

static cell AMX_NATIVE_CALL n_asin(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "asin", 1);
	float fResult = (float)(asin(amx_ctof(params[1])) * 180 / PI);
	return amx_ftoc(fResult);
}

static cell AMX_NATIVE_CALL n_acos(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "acos", 1);
	float fResult = (float)(acos(amx_ctof(params[1])) * 180 / PI);
	return amx_ftoc(fResult);
}

static cell AMX_NATIVE_CALL n_atan(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "atan", 1);
	float fResult = (float)(atan(amx_ctof(params[1])) * 180 / PI);
	return amx_ftoc(fResult);
}

static cell AMX_NATIVE_CALL n_atan2(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "atan2", 2);
	float fResult = (float)(atan2(amx_ctof(params[1]), amx_ctof(params[2])) * 180 / PI);
	return amx_ftoc(fResult);
}

//----------------------------------------------------
// native SetGravity(gravity)

static cell AMX_NATIVE_CALL n_SetGravity(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "SetGravity", 1);

	float fGravity = amx_ctof(params[1]);

	pNetGame->SetGravity(fGravity);

	return 1;
}

//----------------------------------------------------------------------------------
// native SetVehicleHealth(vehicleid, Float:health)

static cell AMX_NATIVE_CALL n_SetVehicleHealth(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "SetVehicleHealth", 2);

	CVehicle* pVehicle = pNetGame->GetVehiclePool()->GetAt((VEHICLEID)params[1]);
	if (pVehicle)
	{
		pVehicle->SetHealth(amx_ctof(params[2]));
		return 1;
	}

	return 0;
}

//----------------------------------------------------------------------------------
// native GetVehicleHealth(vehicleid, &Float:health)

static cell AMX_NATIVE_CALL n_GetVehicleHealth(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "GetVehicleHealth", 2);

	CVehicle* Vehicle = pNetGame->GetVehiclePool()->GetAt((VEHICLEID)params[1]);

	if (Vehicle)
	{

		cell* cptr;
		amx_GetAddr(amx, params[2], &cptr);
		*cptr = amx_ftoc(Vehicle->m_fHealth);

		return 1;
	} else {
		return 0;
	}
}

//----------------------------------------------------------------------------------
// native ApplyAnimation(playerid, animlib[], animname[], Float:fS, opt1, opt2, opt3, opt4, opt5)

static cell AMX_NATIVE_CALL n_ApplyAnimation(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "ApplyAnimation", 9);
	RakNet::BitStream bsSend;

	char *szAnimLib;
	char *szAnimName;
	size_t uiLibLen;
	size_t uiNameLen;
	float fS;
	bool opt1,opt2,opt3,opt4;
	int opt5;
	
	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
	if(!pPlayerPool || !pPlayerPool->GetSlotState(params[1])) return 1;

	amx_StrParam(amx, params[2], szAnimLib);
	amx_StrParam(amx, params[3], szAnimName);

	uiLibLen = strlen(szAnimLib);
	uiNameLen = strlen(szAnimName);

	fS = amx_ctof(params[4]);
	opt1 = (bool)params[5];
	opt2 = (bool)params[6];
	opt3 = (bool)params[7];
	opt4 = (bool)params[8];
	opt5 = (int)params[9];

	bsSend.Write((BYTE)params[1]);
	bsSend.Write(uiLibLen);
	bsSend.Write(szAnimLib, uiLibLen);
	bsSend.Write(uiNameLen);
	bsSend.Write(szAnimName, uiNameLen);
	bsSend.Write(fS);
	bsSend.Write(opt1);
	bsSend.Write(opt2);
	bsSend.Write(opt3);
	bsSend.Write(opt4);
	bsSend.Write(opt5);

	pNetGame->BroadcastDistanceRPC(RPC_ScrApplyAnimation,&bsSend,UNRELIABLE,(BYTE)params[1],200.0f);
	
	return 1;
}
//----------------------------------------------------------------------------------
// native ClearAnimations(playerid)

static cell AMX_NATIVE_CALL n_ClearAnimations(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "ClearAnimations", 1);
	RakNet::BitStream bsSend;
	
	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
	if(!pPlayerPool || !pPlayerPool->GetSlotState(params[1])) return 1;

	bsSend.Write((BYTE)params[1]);
	pNetGame->BroadcastDistanceRPC(RPC_ScrClearAnimations,&bsSend,UNRELIABLE,(BYTE)params[1],200.0f);
	
	return 1;
}


//----------------------------------------------------------------------------------
// native AllowPlayerTeleport(playerid, allow)

static cell AMX_NATIVE_CALL n_AllowPlayerTeleport(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "AllowPlayerTeleport", 2);
	
	CPlayer* pPlayer = pNetGame->GetPlayerPool()->GetAt((BYTE)params[1]);
	if(!pPlayer) return 0;
	if (params[2])
	{
		pPlayer->m_bCanTeleport = true;
	}
	else
	{
		pPlayer->m_bCanTeleport = false;
	}
	return 1;
}

//----------------------------------------------------------------------------------

// native AllowAdminTeleport(allow)

static cell AMX_NATIVE_CALL n_AllowAdminTeleport(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "AllowAdminTeleport", 1);
	
	if (params[1])
	{
		pNetGame->m_bAdminTeleport = true;
	}
	else
	{
		pNetGame->m_bAdminTeleport = false;
	}
	return 1;
}

//----------------------------------------------------

static cell AMX_NATIVE_CALL n_AttachTrailerToVehicle(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "AttachTrailerToVehicle", 2);
	CVehiclePool* pVehiclePool = pNetGame->GetVehiclePool();

	if ( pVehiclePool->GetAt((VEHICLEID)params[1]) && pVehiclePool->GetAt((VEHICLEID)params[2]) )
	{
		RakNet::BitStream bsParams;
		bsParams.Write((VEHICLEID)params[1]);
		bsParams.Write((VEHICLEID)params[2]);
		pNetGame->GetRakServer()->RPC(RPC_ScrAttachTrailerToVehicle , &bsParams, HIGH_PRIORITY, RELIABLE, 0, UNASSIGNED_PLAYER_ID, true, false);
	}
	return 1;
}

//----------------------------------------------------

static cell AMX_NATIVE_CALL n_DetachTrailerFromVehicle(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "DetachTrailerFromVehicle", 1);
	CVehiclePool* pVehiclePool = pNetGame->GetVehiclePool();

	if ( pVehiclePool->GetAt((VEHICLEID)params[1]) && pVehiclePool->GetAt((VEHICLEID)params[2]) )
	{
		RakNet::BitStream bsParams;
		bsParams.Write((VEHICLEID)params[1]);
		pNetGame->GetRakServer()->RPC(RPC_ScrDetachTrailerFromVehicle , &bsParams, HIGH_PRIORITY, RELIABLE, 0, UNASSIGNED_PLAYER_ID, true, false);
	}
	return 1;
}

//----------------------------------------------------

static cell AMX_NATIVE_CALL n_IsTrailerAttachedToVehicle(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "IsTrailerAttachedToVehicle", 1);
	CVehiclePool* pVehiclePool = pNetGame->GetVehiclePool();
	if ( pVehiclePool->GetAt((VEHICLEID)params[1])->m_TrailerID != 0 )
	{
		return 1;
	}
	return 0;
}

//----------------------------------------------------

static cell AMX_NATIVE_CALL n_GetVehicleTrailer(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "GetVehicleTrailer", 1);
	if (!pNetGame->GetVehiclePool()->GetSlotState((VEHICLEID)params[1])) return 0;
	return (cell)pNetGame->GetVehiclePool()->GetAt((VEHICLEID)params[1])->m_TrailerID;
}

cell* get_amxaddr(AMX *amx, cell amx_addr);
// native CallRemoteFunction(functionname[], paramlist[], parameters...)
static cell AMX_NATIVE_CALL n_CallRemoteFunction(AMX *amx, cell *params)
{
	CHECK_PARAMS_BETWEEN(amx, "CallRemoteFunction", 2, 52);
	int iLength;
	char* szFuncName; //[32];
	char* szParamList; //[16];
	AMX *amxFile;
	bool bFound = false;
	
	amx_StrParam(amx, params[1], szFuncName);

	if (szFuncName == 0)
		return 0;

	amx_StrParam(amx, params[2], szParamList);
	if (szParamList == NULL) iLength = 0;
	else iLength = strlen(szParamList);
	
	int idx, i, j;
	cell ret = 0;
	
	for (i = -1; i < MAX_FILTER_SCRIPTS; i++)
	{
		//printf("hi 3: %d\n", i);
		if (i == -1)
		{
			CGameMode* pGameMode = pNetGame->GetGameMode();
			if (pGameMode != NULL && pGameMode->IsInitialised())
			{
				amxFile = pGameMode->GetGameModePointer();
			}
			else
			{
				amxFile = NULL;
			}
		}
		else
		{
			CFilterScripts * pFilterScripts = pNetGame->GetFilterScripts();
			if (pFilterScripts != NULL)
			{
				amxFile = pFilterScripts->GetFilterScript(i);
			}
			else
			{
				amxFile = NULL;
			}
		}
		if (amxFile != NULL)
		{
			if (!amx_FindPublic(amxFile, szFuncName, &idx))
			{
				cell amx_addr[256]; // = NULL, *phys_addr[16];
				j = iLength;
				int iOff = 3, numstr;
				for (numstr = 0; numstr < 16; numstr++)
				{
					amx_addr[numstr] = NULL;
				}
				numstr = 0;
				while (j)
				{
					j--;
					if (*(szParamList + j) == 'a')
					{
						cell *paddr; //, *amx_addr;
						int numcells = *get_amxaddr(amx, params[j + iOff + 1]);
						if (amx_Allot(amxFile, numcells, &amx_addr[numstr], &paddr) == AMX_ERR_NONE)
						{
							memcpy(paddr, get_amxaddr(amx, params[j + iOff]), numcells * sizeof (cell));
							amx_Push(amxFile, amx_addr[numstr]);
							numstr++;
						} /* if */
						//iOff++;
					}
					else if (*(szParamList + j) == 's')
					{
						char* szParamText;
						
						//amx_StrParam(amx, *get_amxaddr(amx, params[j + iOff]), szParamText);
						//printf("source 1");
						amx_StrParam(amx, params[j + iOff], szParamText);
						//printf("source 2");
						if (szParamText != NULL && strlen(szParamText) > 0)
						{
							//printf("source 3");
							//printf("%s", szParamText);
							amx_PushString(amxFile, &amx_addr[numstr], NULL, szParamText, 0, 0);
							numstr++;
						}
						else
						{
							//printf("source 4");
							//amx_Push(amxFile, *get_amxaddr(amx, params[j + iOff]));
							*szParamText = 1;
							*(szParamText + 1) = 0;
							amx_PushString(amxFile, &amx_addr[numstr], NULL, szParamText, 0, 0);
						}
					}
					else
					{
						amx_Push(amxFile, *get_amxaddr(amx, params[j + iOff]));
					}
				}
				amx_Exec(amxFile, &ret, idx);
				while (numstr)
				{
					numstr--;
					amx_Release(amxFile, amx_addr[numstr]);
				}
			}
		}
	}
	return (int)ret;
}
// native CallLocalFunction(functionname[], paramlist[], parameters...)
static cell AMX_NATIVE_CALL n_CallLocalFunction(AMX *amx, cell *params)
{
	CHECK_PARAMS_BETWEEN(amx, "CallLocalFunction", 2, 52);
	int iLength;
	char* szFuncName; //[32];
	char* szParamList; //[16];
	bool bFound = false;
	
	amx_StrParam(amx, params[1], szFuncName);

	if (szFuncName == 0)
		return 0;

	amx_StrParam(amx, params[2], szParamList);
	if (szParamList == NULL) iLength = 0;
	else iLength = strlen(szParamList);
	
	int idx, j;
	cell ret = 0;
	
	if (!amx_FindPublic(amx, szFuncName, &idx))
	{
		cell amx_addr[256];
		j = iLength;
		int numstr, iOff = 3; // Count, func, map
		for (numstr = 0; numstr < 16; numstr++)
		{
			amx_addr[numstr] = NULL;
		}
		numstr = 0;
		while (j)
		{
			j--;
			if (*(szParamList + j) == 'a')
			{
				cell *paddr;
				int numcells = *get_amxaddr(amx, params[j + iOff + 1]);
				if (amx_Allot(amx, numcells, &amx_addr[numstr], &paddr) == AMX_ERR_NONE)
				{
					memcpy(paddr, get_amxaddr(amx, params[j + iOff]), numcells * sizeof (cell));
					amx_Push(amx, amx_addr[numstr]);
					numstr++;
				}
			}
			else if (*(szParamList + j) == 's')
			{
				char* szParamText;
				
				amx_StrParam(amx, params[j + iOff], szParamText);
				if (szParamText != NULL && strlen(szParamText) > 0)
				{
					amx_PushString(amx, &amx_addr[numstr], NULL, szParamText, 0, 0);
					numstr++;
				}
				else
				{
					*szParamText = 1;
					*(szParamText + 1) = 0;
					amx_PushString(amx, &amx_addr[numstr], NULL, szParamText, 0, 0);
				}
			}
			else
			{
				amx_Push(amx, *get_amxaddr(amx, params[j + iOff]));
			}
		}
		amx_Exec(amx, &ret, idx);
		while (numstr)
		{
			numstr--;
			amx_Release(amx, amx_addr[numstr]);
		}
	}
	return (int)ret;
}

// native SetDeathDropAmount(amount)
static cell AMX_NATIVE_CALL n_SetDeathDropAmount(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "SetDeathDropAmount", 1);
	pNetGame->m_iDeathDropMoney = params[1];
	return 1;
}

// native GetGravity()

static cell AMX_NATIVE_CALL n_GetGravity(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "GetGravity", 0);
	return amx_ftoc(pNetGame->m_fGravity);
}

// ============================
// Start of server-wide object code
// ============================

static cell AMX_NATIVE_CALL n_CreateObject(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "CreateObject", 7);
	CObjectPool*	pObjectPool = pNetGame->GetObjectPool();
	VECTOR vecPos, vecRot;

	vecPos.X = amx_ctof(params[2]);
	vecPos.Y = amx_ctof(params[3]);
	vecPos.Z = amx_ctof(params[4]);

	vecRot.X = amx_ctof(params[5]);
	vecRot.Y = amx_ctof(params[6]);
	vecRot.Z = amx_ctof(params[7]);
	
	BYTE byteObjectID = pObjectPool->New((int)params[1], &vecPos, &vecRot);

	if (byteObjectID != 0xFF)
	{
		CObject* pObject = pNetGame->GetObjectPool()->GetAt(byteObjectID);
		CPlayerPool* pPlayerPool = pNetGame->GetPlayerPool();
		for(int x = 0; x < MAX_PLAYERS;x++) 
		{	
			if (pPlayerPool->GetSlotState(x))
			{
				pObject->SpawnForPlayer(x); // Done 100 times, may as well speed up with pointers.
			}
		}
	}
	return byteObjectID;
}

static cell AMX_NATIVE_CALL n_SetObjectPos(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "SetObjectPos", 4);

	VECTOR vecPos;
	vecPos.X = amx_ctof(params[2]);
	vecPos.Y = amx_ctof(params[3]);
	vecPos.Z = amx_ctof(params[4]);

	RakNet::BitStream bsParams;
	bsParams.Write((BYTE)params[1]); // byteObjectID
	bsParams.Write(vecPos.X);	// X
	bsParams.Write(vecPos.Y);	// Y
	bsParams.Write(vecPos.Z);	// Z

	RakServerInterface* pRak = pNetGame->GetRakServer();
	pRak->RPC(RPC_ScrSetObjectPos , &bsParams, HIGH_PRIORITY, RELIABLE, 0, UNASSIGNED_PLAYER_ID, true, false);
	CObjectPool *pObjectPool = pNetGame->GetObjectPool();
	CObject*	pObject = pObjectPool->GetAt((BYTE)params[1]);
	pObject->m_matWorld.pos.X = vecPos.X;
	pObject->m_matWorld.pos.Y = vecPos.Y;
	pObject->m_matWorld.pos.Z = vecPos.Z;
	return 1;
}

static cell AMX_NATIVE_CALL n_GetObjectPos(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "GetObjectPos", 4);
	CObjectPool*	pObjectPool = pNetGame->GetObjectPool();
	CObject*		pObject		= pObjectPool->GetAt((BYTE)params[1]);

	if (pObject)
	{
		cell* cptr;
		amx_GetAddr(amx, params[2], &cptr);
		*cptr = amx_ftoc(pObject->m_matWorld.pos.X);
		amx_GetAddr(amx, params[3], &cptr);
		*cptr = amx_ftoc(pObject->m_matWorld.pos.Y);
		amx_GetAddr(amx, params[4], &cptr);
		*cptr = amx_ftoc(pObject->m_matWorld.pos.Z);

		return 1;
	}

	return 0;
}

static cell AMX_NATIVE_CALL n_SetObjectRot(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "SetObjectRot", 4);

	VECTOR vecRot;
	vecRot.X = amx_ctof(params[2]);
	vecRot.Y = amx_ctof(params[3]);
	vecRot.Z = amx_ctof(params[4]);

	RakNet::BitStream bsParams;
	bsParams.Write((BYTE)params[1]); // byteObjectID
	bsParams.Write(vecRot.X);	// X
	bsParams.Write(vecRot.Y);	// Y
	bsParams.Write(vecRot.Z);	// Z

	RakServerInterface* pRak = pNetGame->GetRakServer();
	pRak->RPC(RPC_ScrSetObjectRotation , &bsParams, HIGH_PRIORITY, RELIABLE, 0, UNASSIGNED_PLAYER_ID, true, false);

	CObjectPool *pObjectPool = pNetGame->GetObjectPool();
	CObject*	pObject = pObjectPool->GetAt((BYTE)params[1]);
	pObject->m_matWorld.up.X = vecRot.X;
	pObject->m_matWorld.up.Y = vecRot.Y;
	pObject->m_matWorld.up.Z = vecRot.Z;

	return 1;
}

static cell AMX_NATIVE_CALL n_GetObjectRot(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "GetObjectRot", 4);
	CObjectPool*	pObjectPool = pNetGame->GetObjectPool();
	CObject*		pObject		= pObjectPool->GetAt((BYTE)params[1]);

	if (pObject)
	{
		cell* cptr;
		amx_GetAddr(amx, params[2], &cptr);
		*cptr = amx_ftoc(pObject->m_matWorld.up.X);
		amx_GetAddr(amx, params[3], &cptr);
		*cptr = amx_ftoc(pObject->m_matWorld.up.Y);
		amx_GetAddr(amx, params[4], &cptr);
		*cptr = amx_ftoc(pObject->m_matWorld.up.Z);
		return 1;
	}
	return 0;
}

// native GetObjectModel(objectid)
static cell n_GetObjectModel(AMX* amx, cell* params)
{
	CHECK_PARAMS(amx, "GetObjectModel", 1);

	CObject* pObject = pNetGame->GetObjectPool()->GetAt(params[1]);
	if (!pObject)
		return -1;

	return pObject->m_iModel;
}

static cell AMX_NATIVE_CALL n_IsValidObject(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "IsValidObject", 1);
	CObjectPool*	pObjectPool = pNetGame->GetObjectPool();

	if (pObjectPool->GetAt((BYTE)params[1]))
	{
		return 1;
	}

	return 0;
}

static cell AMX_NATIVE_CALL n_DestroyObject(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "DestroyObject", 1);
	CObjectPool*	pObjectPool = pNetGame->GetObjectPool();

	if (pObjectPool->GetAt((BYTE)params[1]))
	{
		pObjectPool->Delete((BYTE)params[1]);
		RakNet::BitStream bsParams;

		bsParams.Write((BYTE)params[1]);

		pNetGame->GetRakServer()->RPC(RPC_ScrDestroyObject , &bsParams, HIGH_PRIORITY, RELIABLE, 0, UNASSIGNED_PLAYER_ID, true, false);
	}

	return 1;
}

// native IsObjectMoving(objectid)
static cell n_IsObjectMoving(AMX* amx, cell* params)
{
	CHECK_PARAMS(amx, "IsObjectMoving", 1);

	CObject* pObject = pNetGame->GetObjectPool()->GetAt(params[1]);
	if (!pObject)
		return 0;

	return (pObject->m_byteMoving & 1);
}

static cell AMX_NATIVE_CALL n_MoveObject(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "MoveObject", 5);
	CObjectPool*	pObjectPool = pNetGame->GetObjectPool();
	BYTE byteObject = (BYTE)params[1];
	CObject* pObject = pObjectPool->GetAt(byteObject);
	float ret = 0.0f;

	if (pObject)
	{
		RakNet::BitStream bsParams;

		float x = amx_ctof(params[2]);
		float y = amx_ctof(params[3]);
		float z = amx_ctof(params[4]);
		float s = amx_ctof(params[5]);
		bsParams.Write(byteObject);
		bsParams.Write(pObject->m_matWorld.pos.X);
		bsParams.Write(pObject->m_matWorld.pos.Y);
		bsParams.Write(pObject->m_matWorld.pos.Z);
		// It may have been moved before, make sure all players are up to date
		bsParams.Write(x);
		bsParams.Write(y);
		bsParams.Write(z);
		bsParams.Write(s);

		pNetGame->GetRakServer()->RPC(RPC_ScrMoveObject , &bsParams, HIGH_PRIORITY, RELIABLE, 0, UNASSIGNED_PLAYER_ID, true, false);
		//return *amx_ftoc(pObject->MoveTo(x, y, z, s));
		ret = pObject->MoveTo(x, y, z, s);
	}
	return (int)(ret * 1000.0f);
	//return *amx_ftoc(0.0f);
}

static cell AMX_NATIVE_CALL n_StopObject(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "StopObject", 1);
	CObjectPool*	pObjectPool = pNetGame->GetObjectPool();
	BYTE byteObject = (BYTE)params[1];
	CObject* pObject = pObjectPool->GetAt(byteObject);
	
	if (pObject)
	{
		RakNet::BitStream bsParams;

		pObject->Stop();
		bsParams.Write(byteObject);
		bsParams.Write(pObject->m_matWorld.pos.X);
		bsParams.Write(pObject->m_matWorld.pos.Y);
		bsParams.Write(pObject->m_matWorld.pos.Z);
		// Make sure it stops for the player where the server thinks it is

		pNetGame->GetRakServer()->RPC(RPC_ScrStopObject , &bsParams, HIGH_PRIORITY, RELIABLE, 0, UNASSIGNED_PLAYER_ID, true, false);
		//return *amx_ftoc(pObject->MoveTo(x, y, z, s));
		return 1;
	}
	return 0;
}

// =======================
// Start of player object code
// =======================

static cell AMX_NATIVE_CALL n_CreatePlayerObject(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "CreatePlayerObject", 8);
	if (!pNetGame->GetPlayerPool()->GetSlotState((BYTE)params[1])) return 0;
	CObjectPool*	pObjectPool = pNetGame->GetObjectPool();
	VECTOR vecPos, vecRot;

	vecPos.X = amx_ctof(params[3]);
	vecPos.Y = amx_ctof(params[4]);
	vecPos.Z = amx_ctof(params[5]);

	vecRot.X = amx_ctof(params[6]);
	vecRot.Y = amx_ctof(params[7]);
	vecRot.Z = amx_ctof(params[8]);
	
	BYTE byteObjectID = pObjectPool->New((int)params[1], (int)params[2], &vecPos, &vecRot);

	if (byteObjectID != 0xFF)
	{
		pNetGame->GetObjectPool()->GetAtIndividual((BYTE)params[1], byteObjectID)->SpawnForPlayer((BYTE)params[1]);
	}
	return byteObjectID;
}

static cell AMX_NATIVE_CALL n_SetPlayerObjectPos(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "SetPlayerObjectPos", 5);

	if (!pNetGame->GetPlayerPool()->GetSlotState((BYTE)params[1]) ||
		!pNetGame->GetObjectPool()->GetPlayerSlotState((BYTE)params[1], (BYTE)params[2])) return 0;
	VECTOR vecPos;
	vecPos.X = amx_ctof(params[3]);
	vecPos.Y = amx_ctof(params[4]);
	vecPos.Z = amx_ctof(params[5]);

	RakNet::BitStream bsParams;
	bsParams.Write((BYTE)params[2]); // byteObjectID
	bsParams.Write(vecPos.X);	// X
	bsParams.Write(vecPos.Y);	// Y
	bsParams.Write(vecPos.Z);	// Z

	RakServerInterface* pRak = pNetGame->GetRakServer();
	pRak->RPC(RPC_ScrSetObjectPos , &bsParams, HIGH_PRIORITY, RELIABLE, 0, pRak->GetPlayerIDFromIndex(params[1]), false, false);
	
	CObject*	pObject = pNetGame->GetObjectPool()->GetAtIndividual((BYTE)params[1], (BYTE)params[2]);
	pObject->m_matWorld.pos.X = vecPos.X;
	pObject->m_matWorld.pos.Y = vecPos.Y;
	pObject->m_matWorld.pos.Z = vecPos.Z;
	
	return 1;
}

static cell AMX_NATIVE_CALL n_GetPlayerObjectPos(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "GetPlayerObjectPos", 5);
	if (!pNetGame->GetPlayerPool()->GetSlotState((BYTE)params[1])) return 0;
	CObject* pObject = pNetGame->GetObjectPool()->GetAtIndividual((BYTE)params[1], (BYTE)params[2]);

	if (pObject)
	{
		cell* cptr;
		amx_GetAddr(amx, params[3], &cptr);
		*cptr = amx_ftoc(pObject->m_matWorld.pos.X);
		amx_GetAddr(amx, params[4], &cptr);
		*cptr = amx_ftoc(pObject->m_matWorld.pos.Y);
		amx_GetAddr(amx, params[5], &cptr);
		*cptr = amx_ftoc(pObject->m_matWorld.pos.Z);

		return 1;
	}

	return 0;
}

static cell AMX_NATIVE_CALL n_SetPlayerObjectRot(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "SetPlayerObjectRot", 5);

	if (!pNetGame->GetPlayerPool()->GetSlotState((BYTE)params[1]) ||
		!pNetGame->GetObjectPool()->GetPlayerSlotState((BYTE)params[1], (BYTE)params[2])) return 0;
	VECTOR vecRot;
	vecRot.X = amx_ctof(params[3]);
	vecRot.Y = amx_ctof(params[4]);
	vecRot.Z = amx_ctof(params[5]);

	RakNet::BitStream bsParams;
	bsParams.Write((BYTE)params[2]); // byteObjectID
	bsParams.Write(vecRot.X);	// X
	bsParams.Write(vecRot.Y);	// Y
	bsParams.Write(vecRot.Z);	// Z

	RakServerInterface* pRak = pNetGame->GetRakServer();
	pRak->RPC(RPC_ScrSetObjectRotation , &bsParams, HIGH_PRIORITY, RELIABLE, 0, pRak->GetPlayerIDFromIndex(params[1]), true, false);
	//printf("rotation sent");

	CObject*	pObject = pNetGame->GetObjectPool()->GetAtIndividual((BYTE)params[1], (BYTE)params[2]);
	pObject->m_matWorld.up.X = vecRot.X;
	pObject->m_matWorld.up.Y = vecRot.Y;
	pObject->m_matWorld.up.Z = vecRot.Z;

	return 1;
}

static cell AMX_NATIVE_CALL n_GetPlayerObjectRot(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "GetPlayerObjectRot", 5);
	if (!pNetGame->GetPlayerPool()->GetSlotState((BYTE)params[1])) return 0;
	CObject* pObject = pNetGame->GetObjectPool()->GetAtIndividual((BYTE)params[1], (BYTE)params[2]);

	if (pObject)
	{
		cell* cptr;
		amx_GetAddr(amx, params[3], &cptr);
		*cptr = amx_ftoc(pObject->m_matWorld.up.X);
		amx_GetAddr(amx, params[4], &cptr);
		*cptr = amx_ftoc(pObject->m_matWorld.up.Y);
		amx_GetAddr(amx, params[5], &cptr);
		*cptr = amx_ftoc(pObject->m_matWorld.up.Z);
		return 1;
	}
	return 0;
}

static cell AMX_NATIVE_CALL n_IsValidPlayerObject(AMX *amx, cell *params)
{

	CHECK_PARAMS(amx, "IsValidPlayerObject", 2);
	if (!pNetGame->GetPlayerPool()->GetSlotState((BYTE)params[1])) return 0;

	if (pNetGame->GetObjectPool()->GetAtIndividual((BYTE)params[1], (BYTE)params[2]))
	{
		return 1;
	}

	return 0;
}

static cell AMX_NATIVE_CALL n_DestroyPlayerObject(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "DestroyPlayerObject", 2);
	if (!pNetGame->GetPlayerPool()->GetSlotState((BYTE)params[1])) return 0;
	CObjectPool*	pObjectPool = pNetGame->GetObjectPool();

	if (pObjectPool->GetAtIndividual((BYTE)params[1], (BYTE)params[2]) && pObjectPool->DeleteForPlayer((BYTE)params[1], (BYTE)params[2]))
	{
		RakNet::BitStream bsParams;

		bsParams.Write((BYTE)params[2]);

		RakServerInterface* pRak = pNetGame->GetRakServer();
		pRak->RPC(RPC_ScrDestroyObject , &bsParams, HIGH_PRIORITY, RELIABLE, 0, pRak->GetPlayerIDFromIndex(params[1]), false, false);
	}

	return 1;
}

// native IsPlayerObjectMoving(playerid, playerobjectid)
static cell n_IsPlayerObjectMoving(AMX* amx, cell* params)
{
	CHECK_PARAMS(amx, "IsPlayerObjectMoving", 2);

	if (!pNetGame->GetPlayerPool()->GetSlotState((BYTE)params[1]))
		return 0;

	CObject* pObject = pNetGame->GetObjectPool()->GetAtIndividual(params[1], params[2]);
	if (!pObject)
		return 0;

	return (pObject->m_byteMoving & 1);
}

static cell AMX_NATIVE_CALL n_MovePlayerObject(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "MovePlayerObject", 6);
	if (!pNetGame->GetPlayerPool()->GetSlotState((BYTE)params[1])) return 0;
	CObjectPool*	pObjectPool = pNetGame->GetObjectPool();
	BYTE bytePlayer = (BYTE)params[1];
	BYTE byteObject = (BYTE)params[2];
	CObject* pObject = pObjectPool->GetAtIndividual(bytePlayer, byteObject);
	float ret = 0.0f;
	
	if (pObject)
	{
		RakNet::BitStream bsParams;

		float x = amx_ctof(params[3]);
		float y = amx_ctof(params[4]);
		float z = amx_ctof(params[5]);
		float s = amx_ctof(params[6]);
		bsParams.Write(byteObject);
		bsParams.Write(pObject->m_matWorld.pos.X);
		bsParams.Write(pObject->m_matWorld.pos.Y);
		bsParams.Write(pObject->m_matWorld.pos.Z);
		// It may have been moved before, make sure all players are up to date
		bsParams.Write(x);
		bsParams.Write(y);
		bsParams.Write(z);
		bsParams.Write(s);

		RakServerInterface* pRak = pNetGame->GetRakServer();
		pRak->RPC(RPC_ScrMoveObject , &bsParams, HIGH_PRIORITY, RELIABLE, 0, pRak->GetPlayerIDFromIndex(bytePlayer), false, false);
		//return *amx_ftoc(pObject->MoveTo(x, y, z, s));
		ret = pObject->MoveTo(x, y, z, s);
	}
	return (int)(ret * 1000.0f);
}

static cell AMX_NATIVE_CALL n_StopPlayerObject(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "StopPlayerObject", 2);
	if (!pNetGame->GetPlayerPool()->GetSlotState((BYTE)params[1])) return 0;
	CObjectPool*	pObjectPool = pNetGame->GetObjectPool();
	BYTE bytePlayer = (BYTE)params[1];
	BYTE byteObject = (BYTE)params[2];
	CObject* pObject = pObjectPool->GetAtIndividual(bytePlayer, byteObject);

	if (pObject)
	{
		RakNet::BitStream bsParams;

		pObject->Stop();
		bsParams.Write(byteObject);
		bsParams.Write(pObject->m_matWorld.pos.X);
		bsParams.Write(pObject->m_matWorld.pos.Y);
		bsParams.Write(pObject->m_matWorld.pos.Z);
		// Make sure it stops for the player where the server thinks it is

		RakServerInterface* pRak = pNetGame->GetRakServer();
		pRak->RPC(RPC_ScrStopObject , &bsParams, HIGH_PRIORITY, RELIABLE, 0, pRak->GetPlayerIDFromIndex(bytePlayer), false, false);
		//return *amx_ftoc(pObject->MoveTo(x, y, z, s));
		return 1;
	}
	return 0;
}

// native GetPlayerObjectModel(playerid, objectid)
static cell n_GetPlayerObjectModel(AMX* amx, cell* params)
{
	CHECK_PARAMS(amx, "GetPlayerObjectModel", 2);

	if (!pNetGame->GetPlayerPool()->GetSlotState((BYTE)params[1]))
		return 0;

	CObject* pObject = NULL;
	CObjectPool* pObjectPool = pNetGame->GetObjectPool();
	if (pObjectPool && (pObject = pObjectPool->GetAtIndividual(params[1], params[2])) != NULL)
		return pObject->m_iModel;
	else
		return -1;

	return 0;
}

// Menus

// native Menu:CreateMenu(title[], columns, Float:X, Float:Y, Float:column1width, Float:column2width = 0.0);
static cell AMX_NATIVE_CALL n_CreateMenu(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "CreateMenu", 6);
	CMenuPool* pMenuPool = pNetGame->GetMenuPool();
	if (!pMenuPool)
		return -1;
	char* szMenuTitle;
	float fX = amx_ctof(params[3]), fY = amx_ctof(params[4]),
		fCol1Width = amx_ctof(params[5]), fCol2Width = amx_ctof(params[6]);
	amx_StrParam(amx, params[1], szMenuTitle);
	BYTE menuid = pMenuPool->New((szMenuTitle != 0) ? (szMenuTitle) : (""), fX, fY, params[2], fCol1Width, fCol2Width);
	return menuid != 0xFF ? (menuid) : (-1);
}

// native DestroyMenu(Menu:menuid);
static cell AMX_NATIVE_CALL n_DestroyMenu(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "DestroyMenu", 1);
	CMenuPool* pMenuPool = pNetGame->GetMenuPool();
	if (!pMenuPool)
		return 0;
	return pMenuPool->Delete(params[1]);
}

// native AddMenuItem(Menu:menuid, column, item[]);
static cell AMX_NATIVE_CALL n_AddMenuItem(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "AddMenuItem", 3);
	CMenuPool* pMenuPool = pNetGame->GetMenuPool();
	if (!pMenuPool)
		return 0;
	CMenu* pMenu = pMenuPool->GetAt(params[1]);
	if (!pMenu)
		return 0;
	char* szItemText;
	amx_StrParam(amx, params[3], szItemText);
	BYTE ret = pMenu->AddMenuItem(params[2], szItemText ? (szItemText) : (""));
	return ret != 0xFF ? (ret) : (-1);
}

// native SetMenuColumnHeader(Menu:menuid, column, header[]);
static cell AMX_NATIVE_CALL n_SetMenuColumnHeader(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "SetMenuColumnHeader", 3);
	CMenuPool* pMenuPool = pNetGame->GetMenuPool();
	if (!pMenuPool)
		return 0;
	CMenu* pMenu = pMenuPool->GetAt(params[1]);
	if (!pMenu)
		return 0;
	char* szItemText;
	amx_StrParam(amx, params[3], szItemText);
	pMenu->SetColumnTitle(params[2], szItemText ? (szItemText) : (""));
	return 1;
}

// native ShowMenuForPlayer(Menu:menuid, playerid);
static cell AMX_NATIVE_CALL n_ShowMenuForPlayer(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "ShowMenuForPlayer", 2);
	CMenuPool* pMenuPool = pNetGame->GetMenuPool();
	if (!pMenuPool)
		return 0;
	CMenu* pMenu = pMenuPool->GetAt(params[1]);
	if (!pMenu)
		return 0;
	CPlayerPool* pPlayerPool = pNetGame->GetPlayerPool();
	if (!pPlayerPool && !pPlayerPool->GetSlotState(params[2]))
		return 0;
	pMenu->ShowForPlayer((BYTE)params[2]);
	pMenuPool->SetPlayerMenu((BYTE)params[2], (BYTE)params[1]);
	return 1;
}

// native HideMenuForPlayer(Menu:menuid, playerid);
static cell AMX_NATIVE_CALL n_HideMenuForPlayer(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "HideMenuForPlayer", 2);
	CMenuPool* pMenuPool = pNetGame->GetMenuPool();
	if (!pMenuPool)
		return 0;
	CMenu* pMenu = pMenuPool->GetAt(params[1]);
	if (!pMenu)
		return 0;
	CPlayerPool* pPlayerPool = pNetGame->GetPlayerPool();
	if (!pPlayerPool && !pPlayerPool->GetSlotState(params[2]))
		return 0;
	pMenu->HideForPlayer((BYTE)params[2]);
	pMenuPool->SetPlayerMenu((BYTE)params[2], (BYTE)params[1]);
	return 1;
}

// native IsValidMenu(Menu:menuid);
static cell AMX_NATIVE_CALL n_IsValidMenu(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "IsValidMenu", 1);
	CMenuPool* pMenuPool = pNetGame->GetMenuPool();
	if (!pMenuPool)
		return 0;
	return pMenuPool->GetSlotState(params[1]);
}

// native DisableMenu(Menu:menuid);
static cell AMX_NATIVE_CALL n_DisableMenu(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "DisableMenu", 1);
	CMenuPool* pMenuPool = pNetGame->GetMenuPool();
	if (!pMenuPool)
		return 0;
	CMenu* pMenu = pMenuPool->GetAt(params[1]);
	if (!pMenu)
		return 0;
	pMenu->DisableInteraction();
	return 1;
}

// native DisableMenuRow(Menu:menuid, row);
static cell AMX_NATIVE_CALL n_DisableMenuRow(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "DisableMenuRow", 2);
	CMenuPool* pMenuPool = pNetGame->GetMenuPool();
	if (!pMenuPool)
		return 0;
	CMenu* pMenu = pMenuPool->GetAt(params[1]);
	if (!pMenu)
		return 0;
	pMenu->DisableRow(params[2]);
	return 1;
}

// native Menu:GetPlayerMenu(playerid);
static cell AMX_NATIVE_CALL n_GetPlayerMenu(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "GetPlayerMenu", 1);
	CMenuPool* pMenuPool = pNetGame->GetMenuPool();
	CPlayerPool* pPlayerPool = pNetGame->GetPlayerPool();
	if (!pMenuPool && !pPlayerPool && !pPlayerPool->GetSlotState(params[2]))
		return 255;
	return pMenuPool->GetPlayerMenu((BYTE)params[1]);
}

static cell AMX_NATIVE_CALL n_CreateExplosion(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "CreateExplosion", 5);

	RakNet::BitStream bsParams;

	bsParams.Write(params[1]);
	bsParams.Write(params[2]);
	bsParams.Write(params[3]);
	bsParams.Write(params[4]);
	bsParams.Write(params[5]);

	pNetGame->GetRakServer()->RPC(RPC_ScrCreateExplosion , &bsParams, HIGH_PRIORITY, RELIABLE, 0, UNASSIGNED_PLAYER_ID, true, false);

	return 1;
}

static cell n_CreateExplosionForPlayer(AMX* amx, cell* params)
{
	CHECK_PARAMS(amx, "CreateExplosionForPlayer", 6);

	if (pNetGame->GetPlayerPool()->GetSlotState(params[1]))
	{
		RakNet::BitStream bs;

		bs.Write(amx_ctof(params[2])); // x
		bs.Write(amx_ctof(params[3])); // y
		bs.Write(amx_ctof(params[4])); // z
		bs.Write(params[5]); // type
		bs.Write(amx_ctof(params[6])); // radius

		pNetGame->SendToPlayer(params[1], RPC_ScrCreateExplosion, &bs);
	}
	return 1;
}

static cell AMX_NATIVE_CALL n_SetDisabledWeapons(AMX *amx, cell *params)
{
	long long* lpWeapons = &pNetGame->m_longSynchedWeapons;
	*lpWeapons = DEFAULT_WEAPONS;
	int numweaps = params[0] / sizeof (cell);
	while (numweaps)
	{
		int val = *get_amxaddr(amx, params[numweaps]);
		if (val < 47)
		{
			*lpWeapons &= (long long)~(long long)(((long long)1) << val);
		}
		numweaps--;
	}
	return 1;
}

// native EnableZoneNames(enable);
static cell AMX_NATIVE_CALL n_EnableZoneNames(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "EnableZoneNames", 1);
	pNetGame->m_bZoneNames = (bool)params[1];
	return 1;
}

// native AttachObjectToPlayer( objectid, playerid, Float:OffsetX, Float:OffsetY, Float:OffsetZ, Float:rX, Float:rY, Float:rZ )
static cell AMX_NATIVE_CALL n_AttachObjectToPlayer( AMX *amx, cell *params )
{
	CHECK_PARAMS(amx, "AttachObjectToPlayer", 8);

	if ( pNetGame->GetObjectPool()->GetAt( (BYTE)params[1] ) && 
		 pNetGame->GetPlayerPool()->GetAt( (BYTE)params[2] ) )
	{
		RakNet::BitStream bsParams;

		bsParams.Write((BYTE)params[1]);
		bsParams.Write((BYTE)params[2]);

		VECTOR vecOffsets, vecRotations;
		vecOffsets.X = amx_ctof(params[3]);
		vecOffsets.Y = amx_ctof(params[4]);
		vecOffsets.Z = amx_ctof(params[5]);

		vecRotations.X = amx_ctof(params[6]);
		vecRotations.Y = amx_ctof(params[7]);
		vecRotations.Z = amx_ctof(params[8]);

		bsParams.Write(vecOffsets.X);
		bsParams.Write(vecOffsets.Y);
		bsParams.Write(vecOffsets.Z);

		bsParams.Write(vecRotations.X);
		bsParams.Write(vecRotations.Y);
		bsParams.Write(vecRotations.Z);

		pNetGame->GetRakServer()->RPC( RPC_ScrAttachObjectToPlayer, &bsParams, HIGH_PRIORITY, RELIABLE, 0, UNASSIGNED_PLAYER_ID, true, false);
	}

	return 0;
}

//----------------------------------------------------------------------------------

//native AttachPlayerObjectToPlayer(objectplayer, objectid, attachplayer, Float:OffsetX, Float:OffsetY, Float:OffsetZ, Float:rX, Float:rY, Float:rZ);
static cell AMX_NATIVE_CALL n_AttachPlayerObjectToPlayer(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "AttachPlayerObjectToPlayer", 9);

	if (pNetGame->GetPlayerPool()->GetAt((BYTE)params[1]) &&
		pNetGame->GetObjectPool()->GetAtIndividual((BYTE)params[1], (BYTE)params[2]) && 
		pNetGame->GetPlayerPool()->GetAt((BYTE)params[3]))
	{
		RakNet::BitStream bsParams;

		bsParams.Write((BYTE)params[2]);
		bsParams.Write((BYTE)params[3]);

		VECTOR vecOffsets, vecRotations;
		vecOffsets.X = amx_ctof(params[4]);
		vecOffsets.Y = amx_ctof(params[5]);
		vecOffsets.Z = amx_ctof(params[6]);

		vecRotations.X = amx_ctof(params[7]);
		vecRotations.Y = amx_ctof(params[8]);
		vecRotations.Z = amx_ctof(params[9]);

		bsParams.Write(vecOffsets.X);
		bsParams.Write(vecOffsets.Y);
		bsParams.Write(vecOffsets.Z);

		bsParams.Write(vecRotations.X);
		bsParams.Write(vecRotations.Y);
		bsParams.Write(vecRotations.Z);

		RakServerInterface* pRak = pNetGame->GetRakServer();
		pRak->RPC(RPC_ScrAttachObjectToPlayer, &bsParams, HIGH_PRIORITY, RELIABLE, 0, pRak->GetPlayerIDFromIndex((BYTE)params[1]), false, false);
	}

	return 0;
}

//----------------------------------------------------------------------------------

static cell AMX_NATIVE_CALL n_SetPlayerWantedLevel(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "SetPlayerWantedLevel", 2);
	CPlayer* pPlayer = pNetGame->GetPlayerPool()->GetAt((BYTE)params[1]);
	if (pPlayer)
	{
		pPlayer->SetWantedLevel((BYTE)params[2]);
		RakNet::BitStream bsParams;
		bsParams.Write((BYTE)params[2]);
		RakServerInterface* pRak = pNetGame->GetRakServer();
		pRak->RPC(RPC_ScrSetPlayerWantedLevel, &bsParams, HIGH_PRIORITY, RELIABLE, 0, pRak->GetPlayerIDFromIndex((BYTE)params[1]), false, false);
		return 1;
	}
	return 0;
}

static cell AMX_NATIVE_CALL n_GetPlayerWantedLevel(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "GetPlayerWantedLevel", 1);
	CPlayer* pPlayer = pNetGame->GetPlayerPool()->GetAt((BYTE)params[1]);
	if (pPlayer) return pPlayer->GetWantedLevel();
	return 0;
}

// native GetPlayerVelocity(playerid, &Float:x, &Float:y, &Float:z)
static cell n_GetPlayerVelocity(AMX* amx, cell* params)
{
	CHECK_PARAMS(amx, "GetPlayerVelocity", 4);

	CPlayer* pPlayer = NULL;
	CPlayerPool* pPool = pNetGame->GetPlayerPool();
	if (0 <= params[1] && pPool != NULL && (pPlayer = pPool->GetAt(params[1])) != NULL)
	{
		cell* cptr;
		amx_GetAddr(amx, params[2], &cptr);
		*cptr = amx_ftoc(pPlayer->m_vecMoveSpeed.X);
		amx_GetAddr(amx, params[3], &cptr);
		*cptr = amx_ftoc(pPlayer->m_vecMoveSpeed.Y);
		amx_GetAddr(amx, params[4], &cptr);
		*cptr = amx_ftoc(pPlayer->m_vecMoveSpeed.Z);

		return 1;
	}
	return 0;
}

// native SetPlayerVelocity(playerid, Float:X, Float:Y, Float:Z)
static cell n_SetPlayerVelocity(AMX* amx, cell* params)
{
	CHECK_PARAMS(amx, "SetPlayerVelocity", 4);

	CPlayerPool* pPool = pNetGame->GetPlayerPool();
	if (0 <= params[1] && pPool != NULL && pPool->GetSlotState(params[1]))
	{
		RakNet::BitStream out;
		float fX, fY, fZ;

		fX = amx_ctof(params[2]);
		fY = amx_ctof(params[3]);
		fZ = amx_ctof(params[4]);

		out.Write(params[1]);
		out.Write(fX);
		out.Write(fY);
		out.Write(fZ);

		bool bSend = false;
		if (pNetGame->GetRakServer())
			bSend = pNetGame->GetRakServer()->RPC(RPC_ScrPlayerVelocity, &out,
				HIGH_PRIORITY, RELIABLE, 0, UNASSIGNED_PLAYER_ID, true, false);

		return bSend;
	}
	return 0;
}

// native SetPlayerSkillLevel(playerid, skill, level)
static cell n_SetPlayerSkillLevel(AMX* amx, cell* params)
{
	CHECK_PARAMS(amx, "SetPlayerSkillLevel", 3);
	CPlayerPool* pPool = pNetGame->GetPlayerPool();
	if (0 <= params[1] && pPool != NULL && pPool->GetSlotState(params[1]))
	{
		if (params[2] < 0 || params[2] > 10)
			return 0;

		RakNet::BitStream out;
		
		out.Write<int>(2);
		out.Write((unsigned char)params[2]);
		out.Write((unsigned int)params[3]);

		return (cell)pNetGame->SendToPlayer(params[1], RPC_ScrSetPlayer, &out);
	}
	return 0;
}

// native GetPlayerSurfingVehicleID(playerid)
static cell n_GetPlayerSurfingVehicleID(AMX* amx, cell* params)
{
	CHECK_PARAMS(amx, "GetPlayerSurfingVehicleID", 1);
	CPlayer* pPlayer = NULL;
	CPlayerPool* pPlayerPool = pNetGame->GetPlayerPool();
	if (0 <= params[1] && (pPlayer = pPlayerPool->GetAt(params[1])) != NULL)
	{
		VEHICLEID usId = pPlayer->GetOnFootSyncData()->SurfVehicleId;
		return (usId != 0) ? (usId) : (INVALID_VEHICLE);
	}
	return INVALID_VEHICLE;
}

// native GetPlayerVehicleSeat(playerid)
static cell n_GetPlayerVehicleSeat(AMX* amx, cell* params)
{
	CHECK_PARAMS(amx, "GetPlayerVehicleSeat", 1);
	CPlayer* pPlayer = NULL;
	CPlayerPool* pPlayerPool = pNetGame->GetPlayerPool();
	if (0 <= params[1] && (pPlayer = pPlayerPool->GetAt(params[1])) != NULL)
	{
		unsigned char ucState = pPlayer->GetState();
		if (ucState == PLAYER_STATE_DRIVER)
			return 0;
		else if (ucState == PLAYER_STATE_PASSENGER)
			return pPlayer->GetPassengerSyncData()->byteSeatFlags & 127;

		return -1;
	}
	return -2;
}

// native GetPlayerCameraMode(playerid)
static cell n_GetPlayerCameraMode(AMX* amx, cell* params)
{
	CHECK_PARAMS(amx, "GetPlayerCameraMode", 1);
	CPlayer* pPlayer = NULL;
	CPlayerPool* pPlayerPool = pNetGame->GetPlayerPool();
	if (0 <= params[1] && (pPlayer = pPlayerPool->GetAt(params[1])) != NULL)
	{
		return pPlayer->GetAimSyncData()->byteCamMode;
	}
	return -1;
}

// native SetPlayerArmedWeapon(playerid, weaponid)
static cell n_SetPlayerArmedWeapon(AMX* amx, cell* params)
{
	CHECK_PARAMS(amx, "SetPlayerArmedWeapon", 2);
	CPlayer* pPlayer = NULL;
	CPlayerPool* pPlayerPool = pNetGame->GetPlayerPool();
	if (0 <= params[1] && (pPlayer = pPlayerPool->GetAt(params[1])) != NULL)
	{
		bool bResult = false;
		for (unsigned int i = 0; i < 13; i++)
			if (pPlayer->GetSlotWeapon(i) == params[2])
			{
				bResult = true;
				break;
			}
		if (!bResult)
			return -1;

		RakNet::BitStream out;
		
		out.Write<int>(3);
		out.Write((unsigned char)params[2]);

		bResult = false;
		RakServerInterface* pServer = pNetGame->GetRakServer();
		if (pServer)
			bResult = pNetGame->SendToPlayer(params[1], RPC_ScrSetPlayer, &out);

		return (bResult) ? (1) : (-2);
	}
	return 0;
}

//----------------------------------------------------------------------------------

static cell AMX_NATIVE_CALL n_TextDrawCreate(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "TextDrawCreate", 3);
	CTextDrawPool* pTextDraw = pNetGame->GetTextDrawPool();
	if (pTextDraw)
	{
		char* szText;
		amx_StrParam(amx, params[3], szText);
		szText = (szText != NULL) ? (szText) : ("");
		return pTextDraw->New(amx_ctof(params[1]), amx_ctof(params[2]), szText);
	}
	return 0xFFFF;
}

static cell AMX_NATIVE_CALL n_TextDrawSetString(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "TextDrawSetString", 2);
	CTextDrawPool* pTextDraw = pNetGame->GetTextDrawPool();
	if (pTextDraw && pTextDraw->GetSlotState(params[1]))
	{
		char* szText;
		amx_StrParam(amx, params[2], szText);
		szText = (szText != NULL) ? (szText) : ("");
		pTextDraw->SetTextString(params[1], szText);
		return 1;
	}
	return 0;
}

static cell AMX_NATIVE_CALL n_TextDrawLetterSize(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "TextDrawLetterSize", 3);
	CTextDrawPool* pTextDraw = pNetGame->GetTextDrawPool();
	if (pTextDraw && pTextDraw->GetSlotState(params[1]))
	{
		pTextDraw->SetLetterSize(params[1], amx_ctof(params[2]), amx_ctof(params[3]));
		return 1;
	}
	return 0;
}

static cell AMX_NATIVE_CALL n_TextDrawTextSize(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "TextDrawTextSize", 3);
	CTextDrawPool* pTextDraw = pNetGame->GetTextDrawPool();
	if (pTextDraw && pTextDraw->GetSlotState(params[1]))
	{
		pTextDraw->SetTextSize(params[1], amx_ctof(params[2]), amx_ctof(params[3]));
		return 1;
	}
	return 0;
}

static cell AMX_NATIVE_CALL n_TextDrawAlignment(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "TextDrawAlignment", 2);
	CTextDrawPool* pTextDraw = pNetGame->GetTextDrawPool();
	if (pTextDraw && pTextDraw->GetSlotState(params[1]))
	{
		pTextDraw->SetAlignment(params[1], params[2]);
		return 1;
	}
	return 0;
}

static cell AMX_NATIVE_CALL n_TextDrawColor(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "TextDrawColor", 2);
	CTextDrawPool* pTextDraw = pNetGame->GetTextDrawPool();
	if (pTextDraw && pTextDraw->GetSlotState(params[1]))
	{
		pTextDraw->SetColor(params[1], params[2]);
		return 1;
	}
	return 0;
}

static cell AMX_NATIVE_CALL n_TextDrawUseBox(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "TextDrawUseBox", 2);
	CTextDrawPool* pTextDraw = pNetGame->GetTextDrawPool();
	if (pTextDraw && pTextDraw->GetSlotState(params[1]))
	{
		pTextDraw->SetUseBox(params[1], params[2]);
		return 1;
	}
	return 0;
}

static cell AMX_NATIVE_CALL n_TextDrawBoxColor(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "TextDrawBoxColor", 2);
	CTextDrawPool* pTextDraw = pNetGame->GetTextDrawPool();
	if (pTextDraw && pTextDraw->GetSlotState(params[1]))
	{
		pTextDraw->SetBoxColor(params[1], params[2]);
		return 1;
	}
	return 0;
}

static cell AMX_NATIVE_CALL n_TextDrawSetShadow(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "TextDrawSetShadow", 2);
	CTextDrawPool* pTextDraw = pNetGame->GetTextDrawPool();
	if (pTextDraw && pTextDraw->GetSlotState(params[1]))
	{
		pTextDraw->SetShadow(params[1], params[2]);
		return 1;
	}
	return 0;
}

static cell AMX_NATIVE_CALL n_TextDrawSetOutline(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "TextDrawSetOutline", 2);
	CTextDrawPool* pTextDraw = pNetGame->GetTextDrawPool();
	if (pTextDraw && pTextDraw->GetSlotState(params[1]))
	{
		pTextDraw->SetOutline(params[1], params[2]);
		return 1;
	}
	return 0;
}

static cell AMX_NATIVE_CALL n_TextDrawBackgroundColor(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "TextDrawBackgroundColor", 2);
	CTextDrawPool* pTextDraw = pNetGame->GetTextDrawPool();
	if (pTextDraw && pTextDraw->GetSlotState(params[1]))
	{
		pTextDraw->SetBackgroundColor(params[1], params[2]);
		return 1;
	}
	return 0;
}

static cell AMX_NATIVE_CALL n_TextDrawFont(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "TextDrawFont", 2);
	CTextDrawPool* pTextDraw = pNetGame->GetTextDrawPool();
	if (pTextDraw && pTextDraw->GetSlotState(params[1]))
	{
		pTextDraw->SetFont(params[1], params[2]);
		return 1;
	}
	return 0;
}

static cell AMX_NATIVE_CALL n_TextDrawSetProportional(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "TextDrawSetProportional", 2);
	CTextDrawPool* pTextDraw = pNetGame->GetTextDrawPool();
	if (pTextDraw && pTextDraw->GetSlotState(params[1]))
	{
		pTextDraw->SetProportional(params[1], params[2]);
		return 1;
	}
	return 0;
}

static cell AMX_NATIVE_CALL n_TextDrawShowForPlayer(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "TextDrawShowForPlayer", 2);
	if (!pNetGame->GetPlayerPool()->GetSlotState(params[1])) return 0;
	CTextDrawPool* pTextDraw = pNetGame->GetTextDrawPool();
	if (pTextDraw && pTextDraw->GetSlotState(params[2]))
	{
		pTextDraw->ShowForPlayer(params[1], params[2]);
		return 1;
	}
	return 0;
}

static cell AMX_NATIVE_CALL n_TextDrawHideForPlayer(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "TextDrawHideForPlayer", 2);
	if (!pNetGame->GetPlayerPool()->GetSlotState(params[1])) return 0;
	CTextDrawPool* pTextDraw = pNetGame->GetTextDrawPool();
	if (pTextDraw->GetSlotState(params[2]))
	{
		pTextDraw->HideForPlayer(params[1], params[2]);
		return 1;
	}
	return 0;
}

static cell AMX_NATIVE_CALL n_TextDrawShowForAll(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "TextDrawShowForAll", 1);
	CTextDrawPool* pTextDraw = pNetGame->GetTextDrawPool();
	if (pTextDraw && pTextDraw->GetSlotState(params[1]))
	{
		pTextDraw->ShowForAll(params[1]);
		return 1;
	}
	return 0;
}

static cell AMX_NATIVE_CALL n_TextDrawHideForAll(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "TextDrawHideForAll", 1);
	CTextDrawPool* pTextDraw = pNetGame->GetTextDrawPool();
	if (pTextDraw->GetSlotState(params[1]))
	{
		pTextDraw->HideForAll(params[1]);
		return 1;
	}
	return 0;
}

static cell AMX_NATIVE_CALL n_TextDrawDestroy(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "TextDrawDestroy", 1);
	if (!pNetGame->GetTextDrawPool()->GetSlotState(params[1])) return 0;
	pNetGame->GetTextDrawPool()->Delete(params[1]);
	return 1;
}

//----------------------------------------------------------------------------------

static cell AMX_NATIVE_CALL n_GangZoneCreate(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "GangZoneCreate", 4);
	CGangZonePool *pGangZonePool = pNetGame->GetGangZonePool();
	if (!pGangZonePool) return -1;
	WORD ret = pGangZonePool->New(amx_ctof(params[1]), amx_ctof(params[2]), amx_ctof(params[3]), amx_ctof(params[4]));
	if (ret == 0xFFFF) return -1;
	return ret;
}

static cell AMX_NATIVE_CALL n_GangZoneDestroy(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "GangZoneDestroy", 1);
	CGangZonePool *pGangZonePool = pNetGame->GetGangZonePool();
	if (!pGangZonePool || !pGangZonePool->GetSlotState(params[1])) return 0;
	pGangZonePool->Delete(params[1]);
	return 1;
}

static cell AMX_NATIVE_CALL n_GangZoneShowForPlayer(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "GangZoneShowForPlayer", 3);
	if (!pNetGame->GetPlayerPool()->GetSlotState(params[1])) return 0;
	CGangZonePool *pGangZonePool = pNetGame->GetGangZonePool();
	if (!pGangZonePool || !pGangZonePool->GetSlotState(params[2])) return 0;
	pGangZonePool->ShowForPlayer(params[1], params[2], params[3]);
	return 1;
}

static cell AMX_NATIVE_CALL n_GangZoneShowForAll(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "GangZoneShowForAll", 2);
	CGangZonePool *pGangZonePool = pNetGame->GetGangZonePool();
	if (!pGangZonePool || !pGangZonePool->GetSlotState(params[1])) return 0;
	pGangZonePool->ShowForAll(params[1], params[2]);
	return 1;
}

static cell AMX_NATIVE_CALL n_GangZoneHideForPlayer(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "GangZoneHideForPlayer", 2);
	if (!pNetGame->GetPlayerPool()->GetSlotState(params[1])) return 0;
	CGangZonePool *pGangZonePool = pNetGame->GetGangZonePool();
	if (!pGangZonePool || !pGangZonePool->GetSlotState(params[2])) return 0;
	pGangZonePool->HideForPlayer(params[1], params[2]);
	return 1;
}

static cell AMX_NATIVE_CALL n_GangZoneHideForAll(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "GangZoneHideForAll", 1);
	CGangZonePool *pGangZonePool = pNetGame->GetGangZonePool();
	if (!pGangZonePool || !pGangZonePool->GetSlotState(params[1])) return 0;
	pGangZonePool->HideForAll(params[1]);
	return 1;
}

static cell AMX_NATIVE_CALL n_GangZoneFlashForPlayer(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "GangZoneFlashForPlayer", 3);
	if (!pNetGame->GetPlayerPool()->GetSlotState(params[1])) return 0;
	CGangZonePool *pGangZonePool = pNetGame->GetGangZonePool();
	if (!pGangZonePool || !pGangZonePool->GetSlotState(params[2])) return 0;
	pGangZonePool->FlashForPlayer(params[1], params[2], params[3]);
	return 1;
}

static cell AMX_NATIVE_CALL n_GangZoneFlashForAll(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "GangZoneFlashForAll", 2);
	CGangZonePool *pGangZonePool = pNetGame->GetGangZonePool();
	if (!pGangZonePool || !pGangZonePool->GetSlotState(params[1])) return 0;
	pGangZonePool->FlashForAll(params[1], params[2]);
	return 1;
}

static cell AMX_NATIVE_CALL n_GangZoneStopFlashForPlayer(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "GangZoneStopFlashForPlayer", 2);
	if (!pNetGame->GetPlayerPool()->GetSlotState(params[1])) return 0;
	CGangZonePool *pGangZonePool = pNetGame->GetGangZonePool();
	if (!pGangZonePool || !pGangZonePool->GetSlotState(params[2])) return 0;
	pGangZonePool->StopFlashForPlayer(params[1], params[2]);
	return 1;
}

static cell AMX_NATIVE_CALL n_GangZoneStopFlashForAll(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "GangZoneStopFlashForAll", 1);
	CGangZonePool *pGangZonePool = pNetGame->GetGangZonePool();
	if (!pGangZonePool || !pGangZonePool->GetSlotState(params[1])) return 0;
	pGangZonePool->StopFlashForAll(params[1]);
	return 1;
}

//----------------------------------------------------------------------------------
// native GetServerVarAsString(const varname[], buffer[], len)

static cell AMX_NATIVE_CALL n_GetServerVarAsString(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "GetServerVarAsString", 3);
	char *szParam = 0, *szValue = 0;
	amx_StrParam(amx,params[1],szParam);
	szValue = pConsole->GetStringVariable(szParam);
	return set_amxstring(amx, params[2], (szValue == 0) ? ("") : (szValue), params[3]);
}

// native GetServerVarAsInt(const varname[])
static cell AMX_NATIVE_CALL n_GetServerVarAsInt(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "GetServerVarAsInt", 1);
	char *szParam;
	amx_StrParam(amx,params[1],szParam);
	return pConsole->GetIntVariable(szParam);
}

// native GetServerVarAsBool(const varname[])
static cell AMX_NATIVE_CALL n_GetServerVarAsBool(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "GetServerVarAsBool", 1);
	char *szParam;
	amx_StrParam(amx,params[1],szParam);
	return (int)pConsole->GetBoolVariable(szParam);
}

// native NetStats_GetConnectedTime(playerid)
static cell n_NetStats_GetConnectedTime(AMX* amx, cell* params)
{
	CHECK_PARAMS(amx, "NetStats_GetConnectedTime", 1);
	CPlayerPool* pPool = pNetGame->GetPlayerPool();
	if (0 <= params[1] && pPool->GetSlotState(params[1]))
	{
		RakServerInterface* pSvr = pNetGame->GetRakServer();
		if (pSvr == NULL)
			return 0;

		PlayerID id = pSvr->GetPlayerIDFromIndex(params[1]);
		if (id == UNASSIGNED_PLAYER_ID)
			return 0;

		RakNetStatisticsStruct* rss = NULL;
		rss = pSvr->GetStatistics(id);
		if (rss == 0)
			return 0;

		return rss->connectionStartTime;
	}
	return 0;
}

// native NetStats_MessagesReceived(playerid)
static cell n_NetStats_MessagesReceived(AMX* amx, cell* params)
{
	CHECK_PARAMS(amx, "NetStats_MessagesReceived", 1);
	CPlayerPool* pPool = pNetGame->GetPlayerPool();
	if (0 <= params[1] && pPool->GetSlotState(params[1]))
	{
		RakServerInterface* pSvr = pNetGame->GetRakServer();
		if (pSvr == NULL)
			return 0;

		PlayerID id = pSvr->GetPlayerIDFromIndex(params[1]);
		if (id == UNASSIGNED_PLAYER_ID)
			return 0;

		RakNetStatisticsStruct* rss = NULL;
		rss = pSvr->GetStatistics(id);
		if (rss == 0)
			return 0;

		return
			(rss->duplicateMessagesReceived +
			rss->invalidMessagesReceived +
			rss->messagesReceived);
	}
	return 0;
}

// native NetStats_BytesReceived(playerid)
static cell n_NetStats_BytesReceived(AMX* amx, cell* params)
{
	CHECK_PARAMS(amx, "NetStats_BytesReceived", 1);
	CPlayerPool* pPool = pNetGame->GetPlayerPool();
	if (0 <= params[1] && pPool->GetSlotState(params[1]))
	{
		RakServerInterface* pSvr = pNetGame->GetRakServer();
		if (pSvr == NULL)
			return 0;

		PlayerID id = pSvr->GetPlayerIDFromIndex(params[1]);
		if (id == UNASSIGNED_PLAYER_ID)
			return 0;

		RakNetStatisticsStruct* rss = NULL;
		rss = pSvr->GetStatistics(id);
		if (rss == 0)
			return 0;

		return BITS_TO_BYTES(rss->bitsReceived + rss->bitsWithBadCRCReceived);
	}
	return 0;
}

// native NetStats_MessagesSent(playerid)
static cell n_NetStats_MessagesSent(AMX* amx, cell* params)
{
	CHECK_PARAMS(amx, "NetStats_MessagesSent", 1);
	CPlayerPool* pPool = pNetGame->GetPlayerPool();
	if (0 <= params[1] && pPool->GetSlotState(params[1]))
	{
		RakServerInterface* pSvr = pNetGame->GetRakServer();
		if (pSvr == NULL)
			return 0;

		PlayerID id = pSvr->GetPlayerIDFromIndex(params[1]);
		if (id == UNASSIGNED_PLAYER_ID)
			return 0;

		RakNetStatisticsStruct* rss = NULL;
		rss = pSvr->GetStatistics(id);
		if (rss == 0)
			return 0;

		return
			(rss->messagesSent[SYSTEM_PRIORITY] +
			rss->messagesSent[HIGH_PRIORITY] +
			rss->messagesSent[MEDIUM_PRIORITY] +
			rss->messagesSent[LOW_PRIORITY]);
	}
	return 0;
}

// native NetStats_BytesSent(playerid)
static cell n_NetStats_BytesSent(AMX* amx, cell* params)
{
	CHECK_PARAMS(amx, "NetStats_BytesSent", 1);
	CPlayerPool* pPool = pNetGame->GetPlayerPool();
	if (0 <= params[1] && pPool->GetSlotState(params[1]))
	{
		RakServerInterface* pSvr = pNetGame->GetRakServer();
		if (pSvr == NULL)
			return 0;

		PlayerID id = pSvr->GetPlayerIDFromIndex(params[1]);
		if (id == UNASSIGNED_PLAYER_ID)
			return 0;

		RakNetStatisticsStruct* rss = NULL;
		rss = pSvr->GetStatistics(id);
		if (rss == 0)
			return 0;

		return BITS_TO_BYTES(rss->totalBitsSent);
	}
	return 0;
}

// native NetStats_MessagesRecvPerSecond(playerid)
static cell n_NetStats_MessagesRecvPerSecond(AMX* amx, cell* params)
{
	CHECK_PARAMS(amx, "NetStats_MessagesRecvPerSecond", 1);
	CPlayer* pPlayer = NULL;
	CPlayerPool* pPool = pNetGame->GetPlayerPool();
	if (0 <= params[1] && pPool && (pPlayer = pPool->GetAt(params[1])) != NULL)
	{
		return pPlayer->m_uiMsgRecv;
	}
	return 0;
}

// native NetStats_PacketLossPercent(playerid)
static cell n_NetStats_PacketLossPercent(AMX* amx, cell* params)
{
	CHECK_PARAMS(amx, "NetStats_PacketLossPercent", 1);
	CPlayerPool* pPool = pNetGame->GetPlayerPool();
	if (0 <= params[1] && pPool->GetSlotState(params[1]))
	{
		RakServerInterface* pSvr = pNetGame->GetRakServer();
		if (pSvr == NULL)
			return 0;

		PlayerID id = pSvr->GetPlayerIDFromIndex(params[1]);
		if (id == UNASSIGNED_PLAYER_ID)
			return 0;

		RakNetStatisticsStruct* rss = NULL;
		rss = pSvr->GetStatistics(id);
		if (rss == 0)
			return 0;

		float fResult = 100.0f * (float)rss->messagesTotalBitsResent / (float)rss->totalBitsSent;

		return amx_ftoc(fResult);
	}
	return 0;
}

// native NetStats_ConnectionStatus(playerid)
static cell n_NetStats_ConnectionStatus(AMX* amx, cell* params)
{
	CHECK_PARAMS(amx, "NetStats_ConnectionStatus", 1);
	CPlayerPool* pPool = pNetGame->GetPlayerPool();
	if (0 <= params[1] && pPool->GetSlotState(params[1]))
	{
		RakServerInterface* pSvr = pNetGame->GetRakServer();
		if (pSvr == NULL)
			return -1;

		PlayerID id = pSvr->GetPlayerIDFromIndex(params[1]);
		if (id == UNASSIGNED_PLAYER_ID)
			return -1;

		RemoteSystemStruct* rss = NULL;
		rss = pSvr->GetRemoteSystemFromPlayerID(id);
		return (rss != 0) ? ((cell)rss->connectMode) : (-1);
	}
	return -1;
}

// native NetStats_GetIpPort(playerid, ip_port[], ip_port_len)
static cell n_NetStats_GetIpPort(AMX* amx, cell* params)
{
	CHECK_PARAMS(amx, "NetStats_GetIpPort", 3);
	CPlayerPool* pPool = pNetGame->GetPlayerPool();
	if (0 <= params[1] && pPool->GetSlotState(params[1]))
	{
		RakServerInterface* pSvr = pNetGame->GetRakServer();
		if (pSvr == NULL)
			return -1;

		PlayerID id = pSvr->GetPlayerIDFromIndex(params[1]);
		if (id == UNASSIGNED_PLAYER_ID)
			return -1;

		char szBuffer[22];
		unsigned short usPort = 0;
		pSvr->GetPlayerIPFromID(id, szBuffer, &usPort);
		sprintf_s(szBuffer, "%s:%d", szBuffer, usPort);

		return set_amxstring(amx, params[2], szBuffer, params[3]);
	}
	return -1;
}

//----------------------------------------------------------------------------------

// native EnableStuntBonusForAll(enable)
static cell AMX_NATIVE_CALL n_EnableStuntBonusForAll(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "EnableStuntBonusForAll", 1);
	if (params[1] != 1) params[1] = 0;

	pNetGame->m_bStuntBonus = (bool)params[1];
	RakNet::BitStream bsParams;
	bsParams.Write((bool)params[1]);

	pNetGame->GetRakServer()->RPC(RPC_ScrEnableStuntBonus , &bsParams, HIGH_PRIORITY, RELIABLE, 0, UNASSIGNED_PLAYER_ID, true, false);
	return 1;
}

// native EnableStuntBonusForPlayer(playerid, enable)
static cell AMX_NATIVE_CALL n_EnableStuntBonusForPlayer(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "EnableStuntBonusForPlayer", 2);
	if (!pNetGame->GetPlayerPool()->GetSlotState(params[1])) return 0;
	if (params[2] != 1) params[2] = 0;

	RakNet::BitStream bsParams;
	bsParams.Write((bool)params[2]);

	RakServerInterface* pRak = pNetGame->GetRakServer();
	pRak->RPC(RPC_ScrEnableStuntBonus , &bsParams, HIGH_PRIORITY, RELIABLE, 0, pRak->GetPlayerIDFromIndex((BYTE)params[1]), false, false);
	return 1;
}

//----------------------------------------------------------------------------------

// native DisableInteriorEnterExits()
static cell AMX_NATIVE_CALL n_DisableInteriorEnterExits(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "DisableInteriorEnterExits", 0);
	pNetGame->m_bDisableEnterExits = true;

	return 1;
}

// native SetNameTagDrawDistance(Float:distance)
static cell AMX_NATIVE_CALL n_SetNameTagDrawDistance(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "SetNameTagDrawDistance", 1);
	pNetGame->m_fNameTagDrawDistance = amx_ctof(params[1]);

	return 1;
}

// native DisableNameTagLOS()
static cell n_DisableNameTagLOS(AMX* amx, cell* params)
{
	CHECK_PARAMS(amx, "DisableNameTagLOS", 0);
	if (pNetGame)
		pNetGame->m_bNameTagLOS = false;

	return 1;
}

//----------------------------------------------------------------------------------

// native CreatePlayerPickup(pickupid,playerid,model,type,Float:PosX,Float:PosY,Float:PosZ)
static cell AMX_NATIVE_CALL n_CreatePlayerPickup(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "CreatePlayerPickup", 7);
	int iPickupId = params[1];
	if(!pNetGame->GetPlayerPool()) return 0;
	if(!pNetGame->GetPlayerPool()->GetSlotState(params[2])) return 0;

	PICKUP Pickup;
    Pickup.iModel = params[3];
	Pickup.iType = params[4];
	Pickup.fX = amx_ctof(params[5]);
	Pickup.fY = amx_ctof(params[6]);
	Pickup.fZ = amx_ctof(params[7]);

	RakNet::BitStream bsPickup;
	bsPickup.Write(iPickupId);
	bsPickup.Write((PCHAR)&Pickup,sizeof(PICKUP));
	pNetGame->GetRakServer()->RPC(RPC_Pickup, &bsPickup, HIGH_PRIORITY, RELIABLE, 0,
		pNetGame->GetRakServer()->GetPlayerIDFromIndex(params[2]), false, false);

	return 1;
}

//----------------------------------------------------------------------------------

// native DestroyPlayerPickup(pickupid,playerid)
static cell AMX_NATIVE_CALL n_DestroyPlayerPickup(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "DestroyPlayerPickup", 2);
	int iPickupId = params[1];
	if(!pNetGame->GetPlayerPool()) return 0;
	if(!pNetGame->GetPlayerPool()->GetSlotState(params[2])) return 0;

	RakNet::BitStream bsPickup;
	bsPickup.Write(iPickupId);
	pNetGame->GetRakServer()->RPC(RPC_DestroyPickup, &bsPickup, HIGH_PRIORITY, RELIABLE, 0,
		pNetGame->GetRakServer()->GetPlayerIDFromIndex(params[2]), false, false);

	return 1;
}

//----------------------------------------------------------------------------------

// native IsPlayerInRangeOfPoint(playerid,Float:fRange,Float:PosX,Float:PosY,Float:PosZ)
static cell AMX_NATIVE_CALL n_IsPlayerInRangeOfPoint(AMX *amx, cell *params)
{
	CHECK_PARAMS(amx, "IsPlayerInRangeOfPoint", 5);
	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();

	if(!pPlayerPool) return 0;
	if(!pPlayerPool->GetSlotState(params[1])) return 0;

	CPlayer *pPlayer = pPlayerPool->GetAt(params[1]);

	VECTOR vecTestPoint;
	VECTOR *vecThisPlayer;

	float fRange = amx_ctof(params[2]);
	vecTestPoint.X = amx_ctof(params[3]);
	vecTestPoint.Y = amx_ctof(params[4]);
	vecTestPoint.Z = amx_ctof(params[5]);

    fRange = fRange * fRange; // we'll use the squared distance, not the square root.    
	
	vecThisPlayer = &pPlayer->m_vecPos;
	
	float fSX = (vecThisPlayer->X - vecTestPoint.X) * (vecThisPlayer->X - vecTestPoint.X);
	float fSY = (vecThisPlayer->Y - vecTestPoint.Y) * (vecThisPlayer->Y - vecTestPoint.Y);
	float fSZ = (vecThisPlayer->Z - vecTestPoint.Z) * (vecThisPlayer->Z - vecTestPoint.Z);

	if((float)(fSX + fSY + fSZ) < fRange) return 1;

	return 0;
}

// native Float:GetPlayerDistanceFromPoint(playerid, Float:X, Float:Y, Float:Z)
static cell n_GetPlayerDistanceFromPoint(AMX* amx, cell* params)
{
	CHECK_PARAMS(amx, "GetPlayerDistanceFromPoint", 4);

	float fResult = 0.0f;

	CPlayer* pPlayer = pNetGame->GetPlayerPool()->GetAt(params[1]);
	if (!pPlayer)
		return amx_ftoc(fResult);
	
	float
		fX = amx_ctof(params[2]),
		fY = amx_ctof(params[3]),
		fZ = amx_ctof(params[4]);

	fResult = pPlayer->GetDistanceFromPoint(fX, fY, fZ);

	return amx_ftoc(fResult);
}

// native Float:GetVehicleDistanceFromPoint(vehicleid, Float:X, Float:Y, Float:Z)
static cell n_GetVehicleDistanceFromPoint(AMX* amx, cell* params)
{
	CHECK_PARAMS(amx, "GetVehicleDistanceFromPoint", 4);

	float fResult = 0.0f;

	CVehicle* pVehicle = pNetGame->GetVehiclePool()->GetAt(params[1]);
	if (!pVehicle)
		return amx_ftoc(fResult);

	float
		fX = amx_ctof(params[2]),
		fY = amx_ctof(params[3]),
		fZ = amx_ctof(params[4]);

	fResult = pVehicle->GetDistanceFromPoint(fX, fY, fZ);

	return amx_ctof(fResult);
}

//----------------------------------------------------------------------------------

AMX_NATIVE_INFO custom_Natives[] =
{
	// Util
	{ "print",					n_print },
	{ "printf",					n_printf },
	{ "format",					n_format },
	{ "SetTimer",				n_SetTimer },
	{ "KillTimer",				n_KillTimer },
	DEFINE_NATIVE(SetMaxRconLoginAttempt),
	{ "GetTickCount",			n_GetTickCount },
	{ "GetMaxPlayers",			n_GetMaxPlayers },
	{ "SetTimerEx",				n_SetTimerEx },
	{"VectorSize", n_VectorSize},

	//{ "SetMaxPlayers",			n_SetMaxPlayers },
	{ "LimitGlobalChatRadius",	n_LimitGlobalChatRadius },
	DEFINE_NATIVE(LimitPlayerMarkerRadius),
	{ "SetWeather",				n_SetWeather },
	{ "SetPlayerWeather",		n_SetPlayerWeather },
	{ "CallRemoteFunction",		n_CallRemoteFunction },
	{ "CallLocalFunction",		n_CallLocalFunction },
	{ "asin",					n_asin },
	{ "acos",					n_acos },
	{ "atan2",					n_atan2 },
	{ "atan",					n_atan },

	// Game
	{ "GameModeExit",			n_GameModeExit },
	{ "SetGameModeText",		n_SetGameModeText },
	{ "SetTeamCount",			n_SetTeamCount },
	{ "AddPlayerClass",			n_AddPlayerClass },
	{ "AddPlayerClassEx",		n_AddPlayerClassEx },
	{ "AddStaticVehicle",		n_AddStaticVehicle },
	{ "AddStaticVehicleEx",		n_AddStaticVehicleEx },
	{ "AddStaticPickup",		n_AddStaticPickup },
	{ "CreatePickup",			n_CreatePickup },
	{ "DestroyPickup",			n_DestroyPickup },
	{"DestroyAllPickups", n_DestroyAllPickups},
	{"IsValidPickup", n_IsValidPickup},
	{"IsStaticPickup", n_IsPickupStatic},
	{"GetPickupPos", n_GetPickupPos},
	{"GetPickupModel", n_GetPickupModel},
	{"GetPickupType", n_GetPickupType},
	{"GetPickupCount", n_GetPickupCount},
	DEFINE_NATIVE(GetPlayerWorldBounds),
	DEFINE_NATIVE(SetPlayerWorldBounds),
	{ "ShowNameTags",			n_ShowNameTags },
	{ "ShowPlayerMarkers",		n_ShowPlayerMarkers },
	{ "SetWorldTime",			n_SetWorldTime },
	{ "GetWeaponName",			n_GetWeaponName },
	{ "EnableTirePopping",		n_EnableTirePopping },
	{ "AllowInteriorWeapons",	n_AllowInteriorWeapons },
	{ "SetGravity",				n_SetGravity },
	{ "GetGravity",				n_GetGravity },
	{ "AllowAdminTeleport",		n_AllowAdminTeleport },
	{ "SetDeathDropAmount",		n_SetDeathDropAmount },
	DEFINE_NATIVE(CreateExplosion),
	DEFINE_NATIVE(CreateExplosionForPlayer),
	{ "SetDisabledWeapons",		n_SetDisabledWeapons },
	{ "UsePlayerPedAnims",		n_UsePlayerPedAnims },
	{ "DisableInteriorEnterExits", n_DisableInteriorEnterExits },
	{ "SetNameTagDrawDistance", n_SetNameTagDrawDistance },
	DEFINE_NATIVE(DisableNameTagLOS),

	// Zones
	{ "EnableZoneNames",		n_EnableZoneNames },
	{ "GangZoneCreate",				n_GangZoneCreate },
	{ "GangZoneDestroy",			n_GangZoneDestroy },
	{ "GangZoneShowForPlayer",		n_GangZoneShowForPlayer },
	{ "GangZoneShowForAll",			n_GangZoneShowForAll },
	{ "GangZoneHideForPlayer",		n_GangZoneHideForPlayer },
	{ "GangZoneHideForAll",			n_GangZoneHideForAll },
	{ "GangZoneFlashForPlayer",		n_GangZoneFlashForPlayer },
	{ "GangZoneFlashForAll",		n_GangZoneFlashForAll },
	{ "GangZoneStopFlashForPlayer",	n_GangZoneStopFlashForPlayer },
	{ "GangZoneStopFlashForAll",	n_GangZoneStopFlashForAll },

	// Admin
	{ "IsPlayerAdmin",			n_IsPlayerAdmin },
	DEFINE_NATIVE(SetPlayerAdmin),
	{ "Kick",					n_Kick },
	{ "Ban",					n_Ban },
	{ "BanEx",					n_BanEx },
	{ "RemoveBan", n_RemoveBan},
	{ "IsBanned", n_IsBanned},
	DEFINE_NATIVE(BlockIpAddress),
	DEFINE_NATIVE(UnBlockIpAddress),
	{ "SendRconCommand",		n_SendRconCommand },
	{ "GetServerVarAsString",	n_GetServerVarAsString },
	{ "GetServerVarAsInt",		n_GetServerVarAsInt },
	{ "GetServerVarAsBool",		n_GetServerVarAsBool },

	// NetStats
	DEFINE_NATIVE(NetStats_GetConnectedTime),
	DEFINE_NATIVE(NetStats_MessagesReceived),
	DEFINE_NATIVE(NetStats_BytesReceived),
	DEFINE_NATIVE(NetStats_MessagesSent),
	DEFINE_NATIVE(NetStats_BytesSent),
	DEFINE_NATIVE(NetStats_MessagesRecvPerSecond),
	DEFINE_NATIVE(NetStats_PacketLossPercent),
	DEFINE_NATIVE(NetStats_ConnectionStatus),
	DEFINE_NATIVE(NetStats_GetIpPort),

	// Player
	{ "GetPlayerIDFromName", n_GetPlayerIDFromName },
	{ "GetPlayerCount", n_GetPlayerCount},
	{"GetPlayerPoolSize", n_GetPlayerPoolSize },
	DEFINE_NATIVE(GetPlayerVersion),
	{ "SetSpawnInfo",			n_SetSpawnInfo },
	{ "SpawnPlayer",			n_SpawnPlayer },
	{ "SetPlayerTeam",			n_SetPlayerTeam },
	{ "GetPlayerTeam",			n_GetPlayerTeam },
	{ "SetPlayerName",			n_SetPlayerName },
	{ "SetPlayerSkin",			n_SetPlayerSkin },
	{ "GetPlayerSkin",			n_GetPlayerSkin },
	{ "GetPlayerPos",			n_GetPlayerPos },
	{ "SetPlayerPos",			n_SetPlayerPos },
	{ "SetPlayerPosFindZ",		n_SetPlayerPosFindZ },
	{ "GetPlayerHealth", n_GetPlayerHealth },
	{ "SetPlayerHealth",		n_SetPlayerHealth },
	{ "SetPlayerColor", n_SetPlayerColor },
	{ "GetPlayerColor",			n_GetPlayerColor },
	{ "GetPlayerVehicleID",		n_GetPlayerVehicleID },
	{ "PutPlayerInVehicle",		n_PutPlayerInVehicle },
	{ "RemovePlayerFromVehicle",n_RemovePlayerFromVehicle },
	{ "IsPlayerInVehicle",		n_IsPlayerInVehicle },
	{ "IsPlayerInAnyVehicle",	n_IsPlayerInAnyVehicle },
	{ "GetPlayerName",			n_GetPlayerName },
	{ "SetPlayerCheckpoint",	n_SetPlayerCheckpoint },
	{ "DisablePlayerCheckpoint",n_DisablePlayerCheckpoint },
	{ "IsPlayerInCheckpoint",	n_IsPlayerInCheckpoint },
	{ "SetPlayerRaceCheckpoint",	n_SetPlayerRaceCheckpoint },
	{ "DisablePlayerRaceCheckpoint",n_DisablePlayerRaceCheckpoint },
	{ "IsPlayerInRaceCheckpoint",	n_IsPlayerInRaceCheckpoint },
	{ "SetPlayerInterior",		n_SetPlayerInterior },
	{ "GetPlayerInterior",		n_GetPlayerInterior },
	{ "SetPlayerCameraLookAt",	n_SetPlayerCameraLookAt },
	{ "SetPlayerCameraPos",		n_SetPlayerCameraPos },
	{ "SetCameraBehindPlayer",	n_SetCameraBehindPlayer },
	{ "TogglePlayerControllable",	n_TogglePlayerControllable },
	{ "PlayerPlaySound",		n_PlayerPlaySound },
	{ "SetPlayerScore",			n_SetPlayerScore },
	{ "GetPlayerScore",			n_GetPlayerScore },
	{ "SetPlayerFacingAngle",	n_SetPlayerFacingAngle },
	{ "GetPlayerFacingAngle",	n_GetPlayerFacingAngle },
	{ "GivePlayerMoney",		n_GivePlayerMoney },
	{ "SetPlayerMoney",			n_SetPlayerMoney },
	{ "GetPlayerMoney",			n_GetPlayerMoney },
	{ "ResetPlayerMoney",		n_ResetPlayerMoney },
	{ "IsPlayerConnected",		n_IsPlayerConnected },
	{ "GetPlayerState",			n_GetPlayerState },
	{ "ResetPlayerWeapons",		n_ResetPlayerWeapons },
	{ "GivePlayerWeapon",		n_GivePlayerWeapon },
	{ "GetPlayerIp",			n_GetPlayerIp },
	{ "GetPlayerPing",			n_GetPlayerPing },
	{ "GetPlayerWeapon",		n_GetPlayerWeapon },
	{ "SetPlayerArmour",		n_SetPlayerArmour },
	{ "GetPlayerArmour",		n_GetPlayerArmour },
	{ "SetPlayerMapIcon",		n_SetPlayerMapIcon },
	{ "RemovePlayerMapIcon",	n_RemovePlayerMapIcon },
	{ "GetPlayerKeys",			n_GetPlayerKeys },
	{ "SetPlayerMarkerForPlayer",		n_SetPlayerMarkerForPlayer }, // Changed function name
	{ "GetPlayerAmmo",			n_GetPlayerAmmo },
	{ "SetPlayerAmmo",			n_SetPlayerAmmo },
	{ "GetPlayerWeaponData",	n_GetPlayerWeaponData },
	{ "AllowPlayerTeleport",	n_AllowPlayerTeleport },
	{ "ForceClassSelection",	n_ForceClassSelection },
	{ "SetPlayerWantedLevel",	n_SetPlayerWantedLevel },
	{ "GetPlayerWantedLevel",	n_GetPlayerWantedLevel },
	DEFINE_NATIVE(GetPlayerVelocity),
	DEFINE_NATIVE(SetPlayerVelocity),
	DEFINE_NATIVE(SetPlayerSkillLevel),
	DEFINE_NATIVE(GetPlayerSurfingVehicleID),
	DEFINE_NATIVE(GetPlayerVehicleSeat),
	DEFINE_NATIVE(GetPlayerCameraMode),
	DEFINE_NATIVE(SetPlayerArmedWeapon),
	DEFINE_NATIVE(SetPlayerFightingStyle),
	DEFINE_NATIVE(SetPlayerMaxHealth),
	DEFINE_NATIVE(SetPlayerBlurLevel),
	DEFINE_NATIVE(InterpolateCameraPos),
	DEFINE_NATIVE(InterpolateCameraLookAt),

	{ "SetPlayerVirtualWorld",		n_SetPlayerVirtualWorld },
	{ "GetPlayerVirtualWorld",		n_GetPlayerVirtualWorld },
	{ "ShowPlayerNameTagForPlayer",	n_ShowPlayerNameTagForPlayer },

	{ "EnableStuntBonusForAll",		n_EnableStuntBonusForAll },
	{ "EnableStuntBonusForPlayer",	n_EnableStuntBonusForPlayer },

	{ "TogglePlayerSpectating",	n_TogglePlayerSpectating },
	{ "PlayerSpectateVehicle",	n_PlayerSpectateVehicle },
	{ "PlayerSpectatePlayer",	n_PlayerSpectatePlayer },
	{ "ApplyAnimation",			n_ApplyAnimation },
	{ "ClearAnimations",		n_ClearAnimations },
	{ "SetPlayerSpecialAction", n_SetPlayerSpecialAction },
	{ "GetPlayerSpecialAction", n_GetPlayerSpecialAction },
	DEFINE_NATIVE(GetPlayerDrunkLevel),
	DEFINE_NATIVE(SetPlayerDrunkLevel),

	{ "CreatePlayerPickup",		n_CreatePlayerPickup },
	{ "DestroyPlayerPickup",	n_DestroyPlayerPickup },
	{ "IsPlayerInRangeOfPoint", n_IsPlayerInRangeOfPoint },
	DEFINE_NATIVE(GetPlayerDistanceFromPoint),

		// Vehicle
	{ "GetVehiclePoolSize", n_GetVehiclePoolSize },
	{ "IsValidVehicle",			n_IsValidVehicle },
	{ "CreateVehicle",			n_CreateVehicle },
	{ "DestroyVehicle",			n_DestroyVehicle },
	{ "GetVehiclePos",			n_GetVehiclePos },
	{ "SetVehiclePos",			n_SetVehiclePos },
	{ "GetVehicleZAngle",		n_GetVehicleZAngle },
	{ "SetVehicleZAngle",		n_SetVehicleZAngle },
	DEFINE_NATIVE(GetVehicleDistanceFromPoint),
	{ "SetVehicleParamsForPlayer",	n_SetVehicleParamsForPlayer },
	{ "SetVehicleToRespawn",	n_SetVehicleToRespawn },
	{ "AddVehicleComponent",	n_AddVehicleComponent },
	{ "RemoveVehicleComponent",	n_RemoveVehicleComponent },
	{ "ChangeVehicleColor",		n_ChangeVehicleColor },
	{ "ChangeVehiclePaintjob",	n_ChangeVehiclePaintjob },
	{ "LinkVehicleToInterior",	n_LinkVehicleToInterior },
	{ "SetVehicleHealth",		n_SetVehicleHealth },
	{ "GetVehicleHealth",		n_GetVehicleHealth },
	{ "AttachTrailerToVehicle", n_AttachTrailerToVehicle },
	{ "DetachTrailerFromVehicle", n_DetachTrailerFromVehicle },
	{ "IsTrailerAttachedToVehicle",		n_IsTrailerAttachedToVehicle },
	{ "GetVehicleTrailer",		n_GetVehicleTrailer },
	{ "SetVehicleNumberPlate",	n_SetVehicleNumberPlate },
	{ "GetVehicleModel",		n_GetVehicleModel },
	{ "GetVehicleInterior", n_GetVehicleInterior },
	{ "GetVehicleColor", n_GetVehicleColor },
	{ "GetVehiclePaintjob", n_GetVehiclePaintjob },
	{ "SetVehicleVirtualWorld",		n_SetVehicleVirtualWorld },
	{ "GetVehicleVirtualWorld",		n_GetVehicleVirtualWorld },
	DEFINE_NATIVE(RepairVehicle),
	DEFINE_NATIVE(SetVehicleParamsCarWindows),
	DEFINE_NATIVE(ToggleTaxiLight),
	DEFINE_NATIVE(SetVehicleEngineState),
	DEFINE_NATIVE(GetVehicleVelocity),
	DEFINE_NATIVE(SetVehicleVelocity),

	DEFINE_NATIVE(IsVehicleOnItsSide),
	DEFINE_NATIVE(IsVehicleUpsideDown),
	DEFINE_NATIVE(GetVehicleSirenState),
	DEFINE_NATIVE(IsVehicleWrecked),
	DEFINE_NATIVE(IsVehicleSunked),
	DEFINE_NATIVE(SetVehicleLightState),
	DEFINE_NATIVE(SetVehicleRespawnDelay),
	DEFINE_NATIVE(GetVehicleRespawnDelay),
	DEFINE_NATIVE(GetVehicleSpawnPos),
	DEFINE_NATIVE(SetVehicleSpawnPos),
	DEFINE_NATIVE(GetVehicleComponentInSlot),
	DEFINE_NATIVE(GetVehicleComponentType),

	// Messaging
	{ "SendClientMessage",		n_SendClientMessage },
	{ "SendClientMessageToAll",	n_SendClientMessageToAll },
	{ "SendDeathMessage",		n_SendDeathMessage },
	DEFINE_NATIVE(SendDeathMessageToPlayer),
	{ "GameTextForAll",			n_GameTextForAll },
	{ "GameTextForPlayer",		n_GameTextForPlayer },
	{ "SendPlayerMessageToPlayer",	n_SendPlayerMessageToPlayer },
	{ "SendPlayerMessageToAll",		n_SendPlayerMessageToAll },
	
	{ "TextDrawCreate",				n_TextDrawCreate },
	{ "TextDrawSetString",				n_TextDrawSetString },
	{ "TextDrawLetterSize",			n_TextDrawLetterSize },
	{ "TextDrawTextSize",			n_TextDrawTextSize },
	{ "TextDrawAlignment",			n_TextDrawAlignment },
	{ "TextDrawColor",				n_TextDrawColor },
	{ "TextDrawUseBox",				n_TextDrawUseBox },
	{ "TextDrawBoxColor",			n_TextDrawBoxColor },
	{ "TextDrawSetShadow",			n_TextDrawSetShadow },
	{ "TextDrawSetOutline",			n_TextDrawSetOutline },
	{ "TextDrawBackgroundColor",	n_TextDrawBackgroundColor },
	{ "TextDrawFont",				n_TextDrawFont },
	{ "TextDrawSetProportional",	n_TextDrawSetProportional },
	{ "TextDrawShowForPlayer",		n_TextDrawShowForPlayer },
	{ "TextDrawShowForAll",			n_TextDrawShowForAll },
	{ "TextDrawHideForPlayer",		n_TextDrawHideForPlayer },
	{ "TextDrawHideForAll",			n_TextDrawHideForAll },
	{ "TextDrawDestroy",			n_TextDrawDestroy },
	
	// Objects
	{ "CreateObject",			n_CreateObject },
	{ "SetObjectPos",			n_SetObjectPos },
	{ "SetObjectRot",			n_SetObjectRot },
	{ "GetObjectPos",			n_GetObjectPos },
	{ "GetObjectRot",			n_GetObjectRot },
	DEFINE_NATIVE(GetObjectModel),
	{ "IsValidObject",			n_IsValidObject },
	{ "DestroyObject",			n_DestroyObject },
	DEFINE_NATIVE(IsObjectMoving),
	{ "MoveObject",				n_MoveObject },
	{ "StopObject",				n_StopObject },
	
	{ "CreatePlayerObject",			n_CreatePlayerObject },
	{ "SetPlayerObjectPos",			n_SetPlayerObjectPos },
	{ "SetPlayerObjectRot",			n_SetPlayerObjectRot },
	{ "GetPlayerObjectPos",			n_GetPlayerObjectPos },
	{ "GetPlayerObjectRot",			n_GetPlayerObjectRot },
	{ "IsValidPlayerObject",		n_IsValidPlayerObject },
	{ "DestroyPlayerObject",		n_DestroyPlayerObject },
	DEFINE_NATIVE(IsPlayerObjectMoving),
	{ "MovePlayerObject",			n_MovePlayerObject },
	{ "StopPlayerObject",			n_StopPlayerObject },
	DEFINE_NATIVE(GetPlayerObjectModel),

	{ "AttachObjectToPlayer",		n_AttachObjectToPlayer },
	{ "AttachPlayerObjectToPlayer",	n_AttachPlayerObjectToPlayer },
	
	// Menus
	{ "CreateMenu",				n_CreateMenu },
	{ "DestroyMenu",			n_DestroyMenu },
	{ "AddMenuItem",			n_AddMenuItem },
	{ "SetMenuColumnHeader",	n_SetMenuColumnHeader },
	{ "ShowMenuForPlayer",		n_ShowMenuForPlayer },
	{ "HideMenuForPlayer",		n_HideMenuForPlayer },
	{ "IsValidMenu",			n_IsValidMenu },
	{ "DisableMenu",			n_DisableMenu },
	{ "DisableMenuRow",			n_DisableMenuRow },
	{ "GetPlayerMenu",			n_GetPlayerMenu },
	
	{ "SetPlayerTime",			n_SetPlayerTime },
	{ "TogglePlayerClock",		n_TogglePlayerClock },
	{ "GetPlayerTime",			n_GetPlayerTime },

	{ NULL, NULL }
};

//----------------------------------------------------------------------------------

int amx_CustomInit(AMX *amx)
{
  return amx_Register(amx, custom_Natives, -1);
}

//----------------------------------------------------------------------------------
