/*

	SA:MP Multiplayer Modification
	Copyright 2004-2005 SA:MP Team

	file:
		gamemodes.h
	desc:
		GameMode script functions and management header file. 

	Version: $Id: gamemodes.h,v 1.12 2006/04/15 18:58:19 spookie Exp $
*/

#ifndef SAMPSRV_GAMEMODES_H
#define SAMPSRV_GAMEMODES_H

extern char szGameModeFileName[256];

//----------------------------------------------------------------------------------

class CGameMode
{
private:
	AMX m_amx;
	bool m_bInitialised;
	bool m_bSleeping;
	float m_fSleepTime;

	//CScriptTimers* m_pScriptTimers;
public:
	CGameMode();
	~CGameMode();

	char* GetFileName() { return &szGameModeFileName[0]; };
	//CScriptTimers* GetTimers() { return m_pScriptTimers; };
	AMX* GetGameModePointer() { return &m_amx; };

	bool Load(char* pFileName);
	void Unload();
	void Frame(float fElapsedTime);
	bool IsInitialised() { return m_bInitialised; };

//----------------------------------------------------------------------------------

	int CallPublic(char* szFuncName);

	int OnPlayerConnect(cell playerid);
	int OnPlayerDisconnect(cell playerid, cell reason);
	int OnPlayerSpawn(cell playerid);
	int OnPlayerDeath(cell playerid, cell killerid, cell reason);
	int OnVehicleSpawn(cell vehicleid);
	int OnVehicleDeath(cell vehicleid, cell killerid);
	int OnPlayerText(cell playerid, unsigned char * szText);
	int OnPlayerCommandText(cell playerid, unsigned char * szCommandText);
	int OnPlayerInfoChange(cell playerid);
	int OnPlayerRequestClass(cell playerid, cell classid);
	int OnPlayerRequestSpawn(cell playerid);
	int OnPlayerEnterVehicle(cell playerid, cell vehicleid, cell ispassenger);
	int OnPlayerExitVehicle(cell playerid, cell vehicleid);
	int OnPlayerStateChange(cell playerid, cell newstate, cell oldstate);
	int OnPlayerEnterCheckpoint(cell playerid);
	int OnPlayerLeaveCheckpoint(cell playerid);
	int OnPlayerEnterRaceCheckpoint(cell playerid);
	int OnPlayerLeaveRaceCheckpoint(cell playerid);
	int OnRconCommand(char* szCommand);
	int OnObjectMoved(cell objectid);
	int OnPlayerObjectMoved(cell playerid, cell objectid);
	int OnPlayerPickedUpPickup(cell playerid, cell pickupid);
	int OnPlayerExitedMenu(cell playerid);
	int OnPlayerSelectedMenuRow(cell playerid, cell row);
	int OnVehicleRespray(cell playerid, cell vehicleid, cell color1, cell color2);
	int OnVehicleMod(cell playerid, cell vehicleid, cell componentid);
	int OnEnterExitModShop(cell playerid, cell enterexit, cell interiorid);
	int OnVehiclePaintjob(cell playerid, cell vehicleid, cell paintjobid);
	int OnPlayerInteriorChange(cell playerid, cell newid, cell oldid);
	int OnPlayerKeyStateChange(cell playerid, cell newkeys, cell oldkeys);
	int OnPlayerUpdate(cell playerid);
	int OnIncomingConnection(cell playerid, char* ip, cell port);
	int OnPlayerClickMap(cell playerid, float fX, float fY, float fZ);
	int OnTrailerUpdate(cell playerid, cell vehicleid);
	int OnRconLoginAttempt(char* szIP, char* szPassword, cell success);
	void OnPlayerBeginTyping(cell playerid);
	void OnPlayerEndTyping(cell playerid);
	int OnPlayerStunt(cell playerid, cell vehicleid);
	void OnClientCheckResponse(cell playerid, cell type, cell address, cell checksum);
	void OnVehicleSirenStateChange(cell playerid, cell vehicleid, cell newstate);
	void OnVehicleDamageStatusUpdate(cell vehicleid, cell playerid);
	void OnActorStreamIn(cell actorid, cell forplayerid);
	void OnActorStreamOut(cell actorid, cell forplayerid);
	void OnPlayerGiveDamageActor(cell playerid, cell actorid, float fDamage, cell weaponid, cell bodypart);
	void OnPlayerClickTextDraw(cell playerid, cell clickedid);
	void OnPlayerClickPlayerTextDraw(cell playerid, cell playertextid);
};

//----------------------------------------------------------------------------------

#endif

