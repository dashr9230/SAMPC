//----------------------------------------------------------
//
// SA:MP Multiplayer Modification For GTA:SA
// Copyright 2004-2005 SA:MP team
//
// Version: $Id: netgame.cpp,v 1.60 2006/05/21 11:28:29 kyeman Exp $
//
//----------------------------------------------------------

#include <raknet/RakClient.h>
#include "../main.h"
#include "../game/util.h"
#include "../mod.h"

//INCAR_SYNC_DATA DebugSync;
//BOOL bDebugUpdate=FALSE;

static int iExceptMessageDisplayed=0;

int iVehiclesBench=0;
int iPlayersBench=0;
int iPicksupsBench=0;
int iMenuBench=0;
int iObjectBench=0;
//int iTextDrawBench=0;

static int iVehiclePoolProcessFlag=0;
static int iPickupPoolProcessFlag=0;

//----------------------------------------------------

BYTE GetPacketID(Packet *p)
{
	if (p==0) return 255;

	if ((unsigned char)p->data[0] == ID_TIMESTAMP) {
		assert(p->length > sizeof(unsigned char) + sizeof(unsigned long));
		return (unsigned char) p->data[sizeof(unsigned char) + sizeof(unsigned long)];
	}
	else {
		return (unsigned char) p->data[0];
	}
}

//----------------------------------------------------

CNetGame::CNetGame(PCHAR szHostOrIp, int iPort, 
				   PCHAR szPlayerName, PCHAR szPass)
{
	strcpy_s(m_szHostName, "San Andreas Multiplayer");
	strncpy_s(m_szHostOrIp, szHostOrIp, sizeof(m_szHostOrIp));
	m_iPort = iPort;

	// Setup player pool
	m_pPlayerPool = new CPlayerPool();
	m_pPlayerPool->GetLocalPlayer()->SetName(szPlayerName);

	m_pVehiclePool = new CVehiclePool();
	m_pPickupPool  = new CPickupPool();
	m_pObjectPool	= new CObjectPool();
	m_pMenuPool = new CMenuPool();
	m_pTextDrawPool = new CTextDrawPool();
	m_pGangZonePool = new CGangZonePool();

	m_pRakClient = new RakClient;

	RegisterRPCs(m_pRakClient);
	RegisterScriptRPCs(m_pRakClient);	// Register server-side scripting RPCs.

	m_pRakClient->SetPassword(szPass);

	m_dwLastConnectAttempt = GetTickCount();
	m_iGameState = GAMESTATE_WAIT_CONNECT;
	
	m_iSpawnsAvailable = 0;
	m_byteWorldTime = 12;
	m_byteWorldMinute = 0;
	m_byteWeather	= 10;
	m_fGravity = 0.008000000f;
	m_iDeathDropMoney = 0;
	m_bLanMode = false;
	m_byteHoldTime = 1;
	m_bUseCJWalk = false;
	m_bDisableEnterExits = false;
	m_bDisableVehMapIcons = false;
	m_fNameTagDrawDistance = 70.0f;
	m_bNameTagLOS = true;
	m_bAllowWeapons = true;
	m_bLimitGlobalMarkerRadius = false;
	m_bShowPlayerMarkers = true;
	m_bShowPlayerTags = true;
	m_bTirePopping = true;
	m_fGlobalMarkerRadius = 10000.0f;

	m_WorldBounds[0] = m_WorldBounds[2] = 20000.0f;
	m_WorldBounds[1] = m_WorldBounds[3] = -20000.0f;

	int i;
	for (i = 0; i < MAX_MAP_ICON; i++) m_dwMapIcon[i] = NULL;

	m_byteFriendlyFire = 1;
	pGame->EnableClock(0); // Hide the clock by default
	pGame->EnableZoneNames(0);
	m_bZoneNames = false;
	m_bInstagib = false;
	m_iCheckLoadedStuff = 0;

	if(pChatWindow) pChatWindow->AddDebugMessage("SA:MP " SAMP_VERSION " Initialized");

}

//----------------------------------------------------

CNetGame::~CNetGame()
{
	m_pRakClient->Disconnect(0);
	//UnRegisterRPCs(m_pRakClient);
	//UnRegisterScriptRPCs(m_pRakClient);	// Unregister server-side scripting RPCs.
	SAFE_DELETE(m_pRakClient);
	SAFE_DELETE(m_pPlayerPool);
	SAFE_DELETE(m_pVehiclePool);
	SAFE_DELETE(m_pPickupPool);
	SAFE_DELETE(m_pObjectPool);
	SAFE_DELETE(m_pMenuPool);
	SAFE_DELETE(m_pTextDrawPool);
	SAFE_DELETE(m_pGangZonePool);
}

//----------------------------------------------------

void CNetGame::ShutdownForGameModeRestart()
{
	m_byteWorldTime = 12;
	m_byteWorldMinute = 0;
	m_byteWeather	= 10;
	m_byteHoldTime = 1;
	m_bUseCJWalk = false;
	m_fGravity = (float)0.008000000;
	m_iDeathDropMoney = 0;
	pGame->SetGravity(m_fGravity);
	pGame->SetWantedLevel(0);
	pGame->EnableClock(0);
	m_bDisableEnterExits = false;
	m_bDisableVehMapIcons = false;
	m_fNameTagDrawDistance = 70.0f;

	for (BYTE bytePlayerID = 0; bytePlayerID < MAX_PLAYERS; bytePlayerID++) {
		CRemotePlayer* pPlayer = m_pPlayerPool->GetAt(bytePlayerID);
		if (pPlayer) {
			pPlayer->SetTeam(NO_TEAM);
			pPlayer->ResetAllSyncAttributes();
			pPlayer->SetVirtualWorld(0);
		}
	}
	m_pPlayerPool->GetLocalPlayer()->ResetAllSyncAttributes();
	m_pPlayerPool->GetLocalPlayer()->ToggleSpectating(false);
	m_pPlayerPool->GetLocalPlayer()->SetVirtualWorld(0);
	m_iGameState = GAMESTATE_RESTARTING;

	pChatWindow->AddInfoMessage("Game mode restarting..");

	// Disable the ingame players and reset the vehicle pool.
	m_pPlayerPool->DeactivateAll();
	
	// Process the pool one last time
	m_pPlayerPool->Process();

	ResetVehiclePool();
	ResetPickupPool();
	ResetObjectPool();
	ResetMenuPool();
	ResetTextDrawPool();
	ResetGangZonePool();

	if(pDeathWindow)
		pDeathWindow->ClearWindow();

	ResetMapIcons();
	pGame->ToggleCheckpoints(false);
	pGame->ToggleRaceCheckpoints(false);
	pGame->FindPlayerPed()->SetInterior(0);
	pGame->ResetLocalMoney();
	pGame->FindPlayerPed()->SetDead();
	pGame->FindPlayerPed()->SetArmour(0.0f);
	pGame->EnableZoneNames(0);
	m_bZoneNames = false;
	
	GameResetRadarColors();
}

//----------------------------------------------------

/*void CNetGame::InitGameLogic()
{
	//GameResetRadarColors();

	m_WorldBounds[0] = 20000.0f;
	m_WorldBounds[1] = -20000.0f;
	m_WorldBounds[2] = 20000.0f;
	m_WorldBounds[3] = -20000.0f;
}*/

//----------------------------------------------------

void CNetGame::Process()
{	
	UpdateNetwork();

	if (m_byteHoldTime)	{
		pGame->SetWorldTime(m_byteWorldTime, m_byteWorldMinute);
	}

	// Keep the weather fixed at m_byteWeather so it doesnt desync
	pGame->SetWorldWeather(m_byteWeather);

	// KEEP THE FOLLOWING ANIMS LOADED DURING THE NETGAME
	if(CGame::IsAnimationLoaded("PARACHUTE") == 0) CGame::RequestAnimation("PARACHUTE");
	if(CGame::IsAnimationLoaded("DANCING") == 0) CGame::RequestAnimation("DANCING");
	if(CGame::IsAnimationLoaded("GFUNK") == 0) CGame::RequestAnimation("GFUNK");
	if(CGame::IsAnimationLoaded("RUNNINGMAN") == 0)	CGame::RequestAnimation("RUNNINGMAN");
	if(CGame::IsAnimationLoaded("WOP") == 0) CGame::RequestAnimation("WOP");
	if(CGame::IsAnimationLoaded("STRIP") == 0) CGame::RequestAnimation("STRIP");
	if(CGame::IsAnimationLoaded("PAULNMAC") == 0) CGame::RequestAnimation("PAULNMAC");
				
	if(!CGame::IsModelLoaded(OBJECT_PARACHUTE)) {
		CGame::RequestModel(OBJECT_PARACHUTE);
	}

	// keep the throwable weapon models loaded
	if (!CGame::IsModelLoaded(WEAPON_MODEL_TEARGAS))
		CGame::RequestModel(WEAPON_MODEL_TEARGAS);
	if (!CGame::IsModelLoaded(WEAPON_MODEL_GRENADE))
		CGame::RequestModel(WEAPON_MODEL_GRENADE);
	if (!CGame::IsModelLoaded(WEAPON_MODEL_MOLTOV))
		CGame::RequestModel(WEAPON_MODEL_MOLTOV);

	// cellphone
	if (!CGame::IsModelLoaded(330)) CGame::RequestModel(330);

	if(GetGameState() == GAMESTATE_CONNECTED) {

		DWORD dwStartTick = GetTickCount();

		if(m_pPlayerPool) m_pPlayerPool->Process();
		iPlayersBench += GetTickCount() - dwStartTick;

		if(m_pVehiclePool && iVehiclePoolProcessFlag > 5) {
			dwStartTick = GetTickCount();

			try { m_pVehiclePool->Process(); }
			catch(...) { 
				if(!iExceptMessageDisplayed) {				
					pChatWindow->AddDebugMessage("Warning: Error processing vehicle pool"); 
					iExceptMessageDisplayed++;
				}
			}
			iVehiclesBench += GetTickCount() - dwStartTick;
			iVehiclePoolProcessFlag = 0;
		} else {
			iVehiclePoolProcessFlag++;
		}
			
		if(m_pPickupPool && iPickupPoolProcessFlag > 10) {

			dwStartTick = GetTickCount();

			try { m_pPickupPool->Process(); }
			catch(...) {
				if(!iExceptMessageDisplayed) {				
					pChatWindow->AddDebugMessage("Warning: Error processing pickup pool"); 
					iExceptMessageDisplayed++;
				}
			}
			iPicksupsBench += GetTickCount() - dwStartTick;
			iPickupPoolProcessFlag = 0;
		}
		else
		{
			iPickupPoolProcessFlag++;
		}

		if(m_pObjectPool) {
			dwStartTick = GetTickCount();
			try { m_pObjectPool->Process(); }
			catch(...) { 
				if(!iExceptMessageDisplayed) {				
					pChatWindow->AddDebugMessage("Warning: Error processing object pool"); 
					iExceptMessageDisplayed++;
				}
			}
			iObjectBench += GetTickCount() - dwStartTick;
		}

		if(m_pMenuPool) {
			dwStartTick = GetTickCount();
			try { m_pMenuPool->Process(); }
			catch(...) { 
				if(!iExceptMessageDisplayed) {				
					pChatWindow->AddDebugMessage("Warning: Error processing menu pool"); 
					iExceptMessageDisplayed++;
				}
			}
			iMenuBench += GetTickCount() - dwStartTick;
		}	
	}
	else {
		if(pGame->FindPlayerPed()->IsInVehicle()) {
			pGame->FindPlayerPed()->RemoveFromVehicleAndPutAt(1500.0f,-887.0979f,32.56055f);
		} else {
			pGame->FindPlayerPed()->TeleportTo(1500.0f,-887.0979f,32.56055f);
		}
		pGame->GetCamera()->SetPosition(1497.803f,-887.0979f,62.56055f,0.0f,0.0f,0.0f);
		pGame->GetCamera()->LookAtPoint(1406.65f,-795.7716f,82.2771f,2);
		pGame->FindPlayerPed()->TogglePlayerControllable(0);
		pGame->DisplayHud(false);
	}

	if( GetGameState() == GAMESTATE_WAIT_CONNECT && 
		(GetTickCount() - m_dwLastConnectAttempt) > 3000) 
	{
		if(pChatWindow) pChatWindow->AddDebugMessage("Connecting to %s:%d...",m_szHostOrIp,m_iPort);
		m_pRakClient->Connect(m_szHostOrIp,m_iPort,0,0,10);
		m_dwLastConnectAttempt = GetTickCount();
		SetGameState(GAMESTATE_CONNECTING);
	}
}

//----------------------------------------------------
// UPDATE NETWORK
//----------------------------------------------------

void CNetGame::UpdateNetwork()
{
	Packet* pkt=NULL;
	unsigned char packetIdentifier;

	while((pkt = m_pRakClient->Receive()))
	{
		packetIdentifier = GetPacketID(pkt);

		switch(packetIdentifier)
		{
		case ID_RSA_PUBLIC_KEY_MISMATCH:
			Packet_RSAPublicKeyMismatch(pkt);
			break;
		case ID_CONNECTION_BANNED:
			Packet_ConnectionBanned(pkt);
			break;
		case ID_NO_FREE_INCOMING_CONNECTIONS:
			Packet_NoFreeIncomingConnections(pkt);
			break;
		case ID_DISCONNECTION_NOTIFICATION:
			Packet_DisconnectionNotification(pkt);
			break;
		case ID_CONNECTION_LOST:
			Packet_ConnectionLost(pkt);
			break;
		case ID_INVALID_PASSWORD:
			Packet_InvalidPassword(pkt);
			break;
		case ID_MODIFIED_PACKET:
			Packet_ModifiedPacket(pkt);
			break;
		case ID_CONNECTION_ATTEMPT_FAILED: 
			Packet_ConnectAttemptFailed(pkt); 
			break;
		case ID_CONNECTION_REQUEST_ACCEPTED:
			Packet_ConnectionSucceeded(pkt);
			break;
		case ID_PLAYER_SYNC:
			Packet_PlayerSync(pkt);
			break;
		case ID_VEHICLE_SYNC:
			Packet_VehicleSync(pkt);
			break;
		case ID_PASSENGER_SYNC:
			Packet_PassengerSync(pkt);
			break;
		case ID_AIM_SYNC:
			Packet_AimSync(pkt);
			break;
		case ID_TRAILER_SYNC:
			Packet_TrailerSync(pkt);
			break;
		}

		m_pRakClient->DeallocatePacket(pkt);		
	}

}

//----------------------------------------------------
// PACKET HANDLERS INTERNAL
//----------------------------------------------------

void CNetGame::Packet_PlayerSync(Packet *p)
{
	CRemotePlayer * pPlayer;
	RakNet::BitStream bsPlayerSync(p);
	ONFOOT_SYNC_DATA ofSync;
	BYTE bytePlayerID=0;
	
	bool bHasLR,bHasUD;
	bool bMoveSpeedX,bMoveSpeedY,bMoveSpeedZ;
	bool bHasVehicleSurfingInfo;

	if(GetGameState() != GAMESTATE_CONNECTED) return;

	memset(&ofSync,0,sizeof(ONFOOT_SYNC_DATA));

	bsPlayerSync.IgnoreBits(8);
	bsPlayerSync.Read(bytePlayerID);

	//bsPlayerSync.Read((PCHAR)&ofSync,sizeof(ONFOOT_SYNC_DATA));

	// LEFT/RIGHT KEYS
	bsPlayerSync.Read(bHasLR);
	if(bHasLR) bsPlayerSync.Read(ofSync.lrAnalog);
	
	// UP/DOWN KEYS
	bsPlayerSync.Read(bHasUD);
	if(bHasUD) bsPlayerSync.Read(ofSync.udAnalog);

	// GENERAL KEYS
	bsPlayerSync.Read(ofSync.uiKeys);

	// VECTOR POS
	bsPlayerSync.Read((char*)&ofSync.vecPos,sizeof(VECTOR));

	// ROTATION
	bsPlayerSync.Read(ofSync.fRotation);
	
	// HEALTH/ARMOUR (COMPRESSED INTO 1 BYTE)
	BYTE byteHealthArmour;
	BYTE byteArmTemp=0,byteHlTemp=0;

	bsPlayerSync.Read(byteHealthArmour);
	byteArmTemp = (byteHealthArmour & 0x0F);
	byteHlTemp = (byteHealthArmour >> 4);

	if(byteArmTemp == 0xF) ofSync.byteArmour = 100;
	else if(byteArmTemp == 0) ofSync.byteArmour = 0;
	else ofSync.byteArmour = byteArmTemp * 7;

	if(byteHlTemp == 0xF) ofSync.byteHealth = 100;
	else if(byteHlTemp == 0) ofSync.byteHealth = 0;
	else ofSync.byteHealth = byteHlTemp * 7;

	// CURRENT WEAPON
	bsPlayerSync.Read(ofSync.byteCurrentWeapon);

	// Special Action
	bsPlayerSync.Read(ofSync.byteSpecialAction);
	
	// READ MOVESPEED VECTORS
	bsPlayerSync.Read(bMoveSpeedX);
	if(bMoveSpeedX) bsPlayerSync.Read(ofSync.vecMoveSpeed.X);
	else ofSync.vecMoveSpeed.X = 0.0f;

	bsPlayerSync.Read(bMoveSpeedY);
	if(bMoveSpeedY) bsPlayerSync.Read(ofSync.vecMoveSpeed.Y);
	else ofSync.vecMoveSpeed.Y = 0.0f;

	bsPlayerSync.Read(bMoveSpeedZ);
	if(bMoveSpeedZ) bsPlayerSync.Read(ofSync.vecMoveSpeed.Z);
	else ofSync.vecMoveSpeed.Z = 0.0f;

	bsPlayerSync.Read(bHasVehicleSurfingInfo);
	if(bHasVehicleSurfingInfo) {
		bsPlayerSync.Read(ofSync.SurfVehicleId);
		bsPlayerSync.Read(ofSync.vecSurfOffsets.X);
		bsPlayerSync.Read(ofSync.vecSurfOffsets.Y);
		bsPlayerSync.Read(ofSync.vecSurfOffsets.Z);
	} else {
		ofSync.SurfVehicleId = INVALID_VEHICLE_ID;
	}

	pPlayer = GetPlayerPool()->GetAt(bytePlayerID);

	if(pPlayer) {
		pPlayer->StoreOnFootFullSyncData(&ofSync);
	}
}

//----------------------------------------------------

void CNetGame::Packet_AimSync(Packet *p)
{
	CRemotePlayer * pPlayer;
	RakNet::BitStream bsAimSync(p);
	AIM_SYNC_DATA aimSync;
	BYTE bytePlayerID=0;

	if(GetGameState() != GAMESTATE_CONNECTED) return;

	bsAimSync.IgnoreBits(8);
	bsAimSync.Read(bytePlayerID);
	bsAimSync.Read((PCHAR)&aimSync,sizeof(AIM_SYNC_DATA));

	pPlayer = GetPlayerPool()->GetAt(bytePlayerID);

	if(pPlayer) {
		pPlayer->UpdateAimFromSyncData(&aimSync);
	}
}

//----------------------------------------------------

void CNetGame::Packet_VehicleSync(Packet *p)
{
	CRemotePlayer * pPlayer;
	RakNet::BitStream bsSync(p);
	BYTE		bytePlayerID=0;
	INCAR_SYNC_DATA icSync;

	bool bSiren,bLandingGear;
	bool bHydra,bTrain,bTrailer;
	bool bTire;

	if(GetGameState() != GAMESTATE_CONNECTED) return;

	memset(&icSync,0,sizeof(INCAR_SYNC_DATA));

	bsSync.IgnoreBits(8);
	bsSync.Read(bytePlayerID);
	bsSync.Read(icSync.VehicleID);

	//bsSync.Read((PCHAR)&icSync,sizeof(INCAR_SYNC_DATA));

	// KEYS
	bsSync.Read(icSync.lrAnalog);
	bsSync.Read(icSync.udAnalog);
	bsSync.Read(icSync.uiKeys);

	// ROLL / DIRECTION / POSITION / MOVE SPEED
	bsSync.Read((char*)&icSync.cvecRoll,sizeof(C_VECTOR1));
	bsSync.Read((char*)&icSync.cvecDirection,sizeof(C_VECTOR1));
	bsSync.Read((char*)&icSync.vecPos,sizeof(VECTOR));
	bsSync.Read((char*)&icSync.vecMoveSpeed,sizeof(VECTOR));

	// VEHICLE HEALTH
	WORD wTempVehicleHealth;
	bsSync.Read(wTempVehicleHealth);
	icSync.fCarHealth = (float)wTempVehicleHealth;

	// HEALTH/ARMOUR (COMPRESSED INTO 1 BYTE)
	BYTE byteHealthArmour;
	BYTE byteArmTemp=0,byteHlTemp=0;

	bsSync.Read(byteHealthArmour);
	byteArmTemp = (byteHealthArmour & 0x0F);
	byteHlTemp = (byteHealthArmour >> 4);

	if(byteArmTemp == 0xF) icSync.bytePlayerArmour = 100;
	else if(byteArmTemp == 0) icSync.bytePlayerArmour = 0;
	else icSync.bytePlayerArmour = byteArmTemp * 7;

	if(byteHlTemp == 0xF) icSync.bytePlayerHealth = 100;
	else if(byteHlTemp == 0) icSync.bytePlayerHealth = 0;
	else icSync.bytePlayerHealth = byteHlTemp * 7;

	// CURRENT WEAPON
	bsSync.Read(icSync.byteCurrentWeapon);
	
	// SIREN
	bsSync.Read(bSiren);
	if(bSiren) icSync.byteSirenOn = 1;

	// LANDING GEAR
	bsSync.Read(bLandingGear);
	if(bLandingGear) icSync.byteLandingGearState = 1;

	if (m_bTirePopping) {
		bsSync.Read(bTire);
		if (bTire) icSync.byteTires[0] = 1;
		bsSync.Read(bTire);
		if (bTire) icSync.byteTires[1] = 1;
		bsSync.Read(bTire);
		if (bTire) icSync.byteTires[2] = 1;
		bsSync.Read(bTire);
		if (bTire) icSync.byteTires[3] = 1;
	}

	// HYDRA SPECIAL
	bsSync.Read(bHydra);
	if(bHydra) bsSync.Read(icSync.dwHydraThrustAngle);

	// TRAIN SPECIAL
	bsSync.Read(bTrain);
	if(bTrain) bsSync.Read(icSync.fTrainSpeed);

	// TRAILER ID
	bsSync.Read(bTrailer);
	if(bTrailer) bsSync.Read(icSync.TrailerID);
	
	pPlayer = GetPlayerPool()->GetAt(bytePlayerID);

	if(pPlayer)	{
		pPlayer->StoreInCarFullSyncData(&icSync);
	}
}

//----------------------------------------------------

void CNetGame::Packet_PassengerSync(Packet *p)
{
	CRemotePlayer * pPlayer;
	RakNet::BitStream bsPassengerSync(p);
	BYTE		bytePlayerID=0;
	PASSENGER_SYNC_DATA psSync;

	if(GetGameState() != GAMESTATE_CONNECTED) return;

	bsPassengerSync.IgnoreBits(8);
	bsPassengerSync.Read(bytePlayerID);
	bsPassengerSync.Read((PCHAR)&psSync,sizeof(PASSENGER_SYNC_DATA));

	pPlayer = GetPlayerPool()->GetAt(bytePlayerID);

	//OutputDebugString("Getting Passenger Packets");

	if(pPlayer)	{
		pPlayer->StorePassengerFullSyncData(&psSync);
	}
}

//----------------------------------------------------

void CNetGame::Packet_TrailerSync(Packet *p)
{
	CRemotePlayer * pPlayer;
	RakNet::BitStream bsSpectatorSync(p);

	if(GetGameState() != GAMESTATE_CONNECTED) return;

	BYTE bytePlayerID=0;
	TRAILER_SYNC_DATA trSync;
	
	bsSpectatorSync.IgnoreBits(8);
	bsSpectatorSync.Read(bytePlayerID);
	bsSpectatorSync.Read((PCHAR)&trSync, sizeof(TRAILER_SYNC_DATA));

	pPlayer = GetPlayerPool()->GetAt(bytePlayerID);

	if(pPlayer)	{
	    pPlayer->StoreTrailerFullSyncData(&trSync);
	}
}

//----------------------------------------------------

void CNetGame::Packet_RSAPublicKeyMismatch(Packet* packet)
{
	pChatWindow->AddDebugMessage("Failed to initialize encryption.");
}

//----------------------------------------------------

void CNetGame::Packet_ConnectionBanned(Packet* packet)
{
	pChatWindow->AddDebugMessage("You're banned from this server.");
}

//----------------------------------------------------

void CNetGame::Packet_ConnectionRequestAccepted(Packet* packet)
{
	pChatWindow->AddDebugMessage("Server has accepted the connection.");
}

//----------------------------------------------------

void CNetGame::Packet_NoFreeIncomingConnections(Packet* packet)
{
	pChatWindow->AddDebugMessage("The server is full. Retrying...");
	SetGameState(GAMESTATE_WAIT_CONNECT);	
}

//----------------------------------------------------

void CNetGame::Packet_DisconnectionNotification(Packet* packet)
{
	pChatWindow->AddDebugMessage("Server closed the connection.");
	m_pRakClient->Disconnect(0);
}

//----------------------------------------------------

void CNetGame::Packet_ConnectionLost(Packet* packet)
{
	pChatWindow->AddDebugMessage("Lost connection to the server. Reconnecting..");
	ShutdownForGameModeRestart();
    SetGameState(GAMESTATE_WAIT_CONNECT);	
}

//----------------------------------------------------

void CNetGame::Packet_InvalidPassword(Packet* packet)
{
	pChatWindow->AddDebugMessage("Wrong server password.");
	m_pRakClient->Disconnect(0);
}

//----------------------------------------------------

void CNetGame::Packet_ModifiedPacket(Packet* packet)
{
#ifdef _DEBUG
	char szBuffer[256];
	sprintf_s(szBuffer, "Packet was modified, sent by id: %d, ip: %s", 
					(unsigned int)packet->playerIndex, packet->playerId.ToString());
	pChatWindow->AddDebugMessage(szBuffer);
	//m_pRakClient->Disconnect(0);
#endif
}

//----------------------------------------------------
// RST

void CNetGame::Packet_ConnectAttemptFailed(Packet* packet)
{
	pChatWindow->AddDebugMessage("The server didn't respond. Retrying..");
	SetGameState(GAMESTATE_WAIT_CONNECT);
}

//----------------------------------------------------
// Connection Success

void CNetGame::Packet_ConnectionSucceeded(Packet *p)
{
	RakNet::BitStream bsReturnParams(p);

	unsigned int binaryAddr=0;
	unsigned short port=0;
	unsigned short playerId=0;
	unsigned int uiChallenge=0;
	CLocalPlayer* pPlayer = m_pPlayerPool->GetLocalPlayer();

	bsReturnParams.IgnoreBits(8);
	bsReturnParams.Read(binaryAddr);
	bsReturnParams.Read(port);
	bsReturnParams.Read(playerId);
	bsReturnParams.Read(uiChallenge);

	uiChallenge ^= NETGAME_VERSION;

	if(pChatWindow) {
		pChatWindow->AddDebugMessage("Connection success. Loading network game...");
	}

	m_iGameState = GAMESTATE_AWAIT_JOIN;

	int iVersion = NETGAME_VERSION;
	//BYTE byteMod = MOD_VERSION;
	BYTE byteNameLen = (BYTE)strlen(pPlayer->GetName());

	RakNet::BitStream bsSend;
	bsSend.Write(iVersion);
	//bsSend.Write(byteMod);
	bsSend.Write(byteNameLen);
	bsSend.Write(pPlayer->GetName(),byteNameLen);
	bsSend.Write(uiChallenge);
	
	size_t uiVersionLen = strlen(SAMP_VERSION);
	bsSend.Write(uiVersionLen);
	bsSend.Write(SAMP_VERSION, uiVersionLen);

	m_pRakClient->RPC(RPC_ClientJoin,&bsSend,HIGH_PRIORITY,RELIABLE,0,false);
}

//----------------------------------------------------

void CNetGame::UpdatePlayerPings()
{
	static DWORD dwLastUpdateTick = 0;

	if ((GetTickCount() - dwLastUpdateTick) > RPC_PING_UPDATE_TIME) {
		dwLastUpdateTick = GetTickCount();
		m_pRakClient->RPC(RPC_UpdatePings, NULL, HIGH_PRIORITY, RELIABLE, 0, false);
	}
}

//----------------------------------------------------

void CNetGame::ResetVehiclePool()
{
	if(m_pVehiclePool) {
		delete m_pVehiclePool;
	}
	m_pVehiclePool = new CVehiclePool();
}

//----------------------------------------------------

void CNetGame::ResetPlayerPool()
{
	if(m_pPlayerPool) {
		delete m_pPlayerPool;
	}
	m_pPlayerPool = new CPlayerPool();
}

//----------------------------------------------------

void CNetGame::ResetPickupPool()
{
	if(m_pPickupPool) {
		delete m_pPickupPool;
	}
	m_pPickupPool = new CPickupPool();
}

//----------------------------------------------------

void CNetGame::ResetMenuPool()
{
	if(m_pMenuPool) {
		delete m_pMenuPool;
	}
	m_pMenuPool = new CMenuPool();
}

//----------------------------------------------------

void CNetGame::ResetTextDrawPool()
{
	if(m_pTextDrawPool) {
		delete m_pTextDrawPool;
	}
	m_pTextDrawPool = new CTextDrawPool();
}

//----------------------------------------------------

void CNetGame::ResetObjectPool()
{
	if(m_pObjectPool) {
		delete m_pObjectPool;
	}
	m_pObjectPool = new CObjectPool();
}

//----------------------------------------------------

void CNetGame::ResetGangZonePool()
{
	if(m_pGangZonePool) {
		delete m_pGangZonePool;
	}
	m_pGangZonePool = new CGangZonePool();
}


//-----------------------------------------------------------
// Puts a personal marker using any of the radar icons on the map

void CNetGame::SetMapIcon(BYTE byteIndex, float fX, float fY, float fZ, BYTE byteIcon, DWORD dwColor)
{
	if (byteIndex >= MAX_MAP_ICON) return;
	if (m_dwMapIcon[byteIndex] != NULL) DisableMapIcon(byteIndex);
	//ScriptCommand(&create_radar_marker_without_sphere, fX, fY, fZ, byteIcon, &m_dwMapIcon);
	m_dwMapIcon[byteIndex] = pGame->CreateRadarMarkerIcon(byteIcon, fX, fY, fZ, dwColor);
}

//-----------------------------------------------------------
// Removes the Map Icon

void CNetGame::DisableMapIcon(BYTE byteIndex)
{
	if (byteIndex >= MAX_MAP_ICON) return;
	ScriptCommand(&disable_marker, m_dwMapIcon[byteIndex]);
	m_dwMapIcon[byteIndex] = NULL;
}

//----------------------------------------------------

void CNetGame::ResetMapIcons()
{
	BYTE i;
	for (i = 0; i < MAX_MAP_ICON; i++)
	{
		if (m_dwMapIcon[i] != NULL) DisableMapIcon(i);
	}
}

bool CNetGame::Send(short sUniqueID, RakNet::BitStream* pBitStream)
{
	if (m_pRakClient)
		return m_pRakClient->RPC(sUniqueID, pBitStream, HIGH_PRIORITY, RELIABLE, 0, false);

	return false;
}

//----------------------------------------------------
// EOF
