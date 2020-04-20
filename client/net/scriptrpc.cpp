/*

	SA:MP Multiplayer Modification
	Copyright 2004-2005 SA:MP Team

	file:
		scriptrpc.cpp
	desc:
		Scripting RPCs.

	Version: $Id: scriptrpc.cpp,v 1.36 2006/05/08 14:31:50 kyeman Exp $
*/

#include "../main.h"

using namespace RakNet;
extern CNetGame*	pNetGame;
extern CChatWindow*	pChatWindow;
extern CDeathWindow	*pDeathWindow;
extern CGame * pGame;

//----------------------------------------------------

void ScrSetSpawnInfo(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	PLAYER_SPAWN_INFO SpawnInfo;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);

	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();

	bsData.Read((PCHAR)&SpawnInfo, sizeof(PLAYER_SPAWN_INFO));

	/*
	pChatWindow->AddDebugMessage("Got Spawn Info: %i %i %f %f %f %f %i %i %i %i %i %i",
								SpawnInfo.byteTeam,
								SpawnInfo.iSkin,
								SpawnInfo.vecPos.X,
								SpawnInfo.vecPos.Y,
								SpawnInfo.vecPos.Z,
								SpawnInfo.fRotation,
								SpawnInfo.iSpawnWeapons[0],
								SpawnInfo.iSpawnWeaponsAmmo[0],
								SpawnInfo.iSpawnWeapons[1],
								SpawnInfo.iSpawnWeaponsAmmo[1],
								SpawnInfo.iSpawnWeapons[2],
								SpawnInfo.iSpawnWeaponsAmmo[2]);*/

	pPlayerPool->GetLocalPlayer()->SetSpawnInfo(&SpawnInfo);

	//pPlayerPool->GetAt(bytePlayerID)->SetTeam(byteTeam);
}

//----------------------------------------------------

void ScrSetPlayerTeam(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	BYTE bytePlayerID;
	BYTE byteTeam;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);

	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();

	bsData.Read(bytePlayerID);
	bsData.Read(byteTeam);
	
	if (bytePlayerID == pPlayerPool->GetLocalPlayerID()) {
		pPlayerPool->GetLocalPlayer()->SetTeam(byteTeam);
	} else {
		CRemotePlayer *pPlayer = pPlayerPool->GetAt(bytePlayerID);
		if(pPlayer) pPlayer->SetTeam(byteTeam);
	}
}

//----------------------------------------------------

void ScrSetPlayerName(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	BYTE bytePlayerID;
	BYTE byteNickLen;
	char szNewName[MAX_PLAYER_NAME+1];
	BYTE byteSuccess;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);

	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();

	bsData.Read(bytePlayerID);
	bsData.Read(byteNickLen);

	if(byteNickLen > MAX_PLAYER_NAME) return;

	bsData.Read(szNewName, byteNickLen);
	szNewName[byteNickLen] = '\0';

	// TODO: Remove this
	// Why would you even send packet from server to tell the client it was a success,
	// a.k.a. when the nick is not in use?
	bsData.Read(byteSuccess);
	
	if (pPlayerPool->GetLocalPlayerID() == bytePlayerID)
	{
		PCHAR szOldNick = pPlayerPool->GetLocalPlayerName();
		pPlayerPool->SetLocalPlayerName(szNewName);

		if (pDeathWindow)
			pDeathWindow->ChangeNick(szOldNick, szNewName);
	}
	else if(pPlayerPool->GetSlotState(bytePlayerID))
	{
		PCHAR szOldNick = pPlayerPool->GetPlayerName(bytePlayerID);
		pPlayerPool->SetPlayerName(bytePlayerID, szNewName);

		if (pDeathWindow)
			pDeathWindow->ChangeNick(szOldNick, szNewName);
	}
}

void ScrSetPlayerSkin(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	int iPlayerID;
	unsigned int uiSkin;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);

	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();

	bsData.Read(iPlayerID);
	bsData.Read(uiSkin);

	if (iPlayerID == pPlayerPool->GetLocalPlayerID()) {
		pPlayerPool->GetLocalPlayer()->GetPlayerPed()->SetModelIndex(uiSkin);
	}
	else {
		if(pPlayerPool->GetSlotState(iPlayerID) && pPlayerPool->GetAt(iPlayerID)->GetPlayerPed()) {
			pPlayerPool->GetAt(iPlayerID)->GetPlayerPed()->SetModelIndex(uiSkin);
		}
	}
}

//----------------------------------------------------

void ScrSetPlayerPos(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);

	CLocalPlayer *pLocalPlayer = pNetGame->GetPlayerPool()->GetLocalPlayer();

	VECTOR vecPos;

	bsData.Read(vecPos.X);
	bsData.Read(vecPos.Y);
	bsData.Read(vecPos.Z);

	pLocalPlayer->GetPlayerPed()->TeleportTo(vecPos.X,vecPos.Y,vecPos.Z);
}

//----------------------------------------------------

void ScrSetPlayerPosFindZ(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);

	CLocalPlayer *pLocalPlayer = pNetGame->GetPlayerPool()->GetLocalPlayer();

	VECTOR vecPos;

	bsData.Read(vecPos.X);
	bsData.Read(vecPos.Y);
	bsData.Read(vecPos.Z);

	vecPos.Z = pGame->FindGroundZForCoord(vecPos.X, vecPos.Y, vecPos.Z);
	vecPos.Z += 1.5f;

	pLocalPlayer->GetPlayerPed()->TeleportTo(vecPos.X, vecPos.Y, vecPos.Z);
}

//----------------------------------------------------

void ScrSetPlayerHealth(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	CLocalPlayer *pLocalPlayer = pNetGame->GetPlayerPool()->GetLocalPlayer();
	float fHealth;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
	bsData.Read(fHealth);

	//pChatWindow->AddDebugMessage("Setting your health to: %f", fHealth);
	pLocalPlayer->GetPlayerPed()->SetHealth(fHealth);
}

//----------------------------------------------------

void ScrPutPlayerInVehicle(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
	VEHICLEID vehicleid;
	BYTE seatid;
	bsData.Read(vehicleid);
	bsData.Read(seatid);

	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
	int iVehicleIndex = pNetGame->GetVehiclePool()->FindGtaIDFromID(vehicleid);
	CVehicle *pVehicle = pNetGame->GetVehiclePool()->GetAt(vehicleid);

	if(iVehicleIndex && pVehicle) {
		CPlayerPed *pPed = pPlayerPool->GetLocalPlayer()->GetPlayerPed();
		if(pPed) {
			pPed->Add(); pVehicle->Add();
			pPed->PutDirectlyInVehicle(iVehicleIndex, seatid);
		}
	}
}

//----------------------------------------------------

void ScrRemovePlayerFromVehicle(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);

	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
	pPlayerPool->GetLocalPlayer()->GetPlayerPed()->ExitCurrentVehicle();
}

//----------------------------------------------------

void ScrSetPlayerColor(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
	BYTE bytePlayerID;
	DWORD dwColor;

	bsData.Read(bytePlayerID);
	bsData.Read(dwColor);

	if(bytePlayerID == pPlayerPool->GetLocalPlayerID()) {
		pPlayerPool->GetLocalPlayer()->SetPlayerColor(dwColor);
	} else {
		CRemotePlayer *pPlayer = pPlayerPool->GetAt(bytePlayerID);
		if(pPlayer)	pPlayer->SetPlayerColor(dwColor);
	}
}

//----------------------------------------------------

void ScrDisplayGameText(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
	char szMessage[512];
	int iType;
	int iTime;
	int iLength;

	bsData.Read(iType);
	bsData.Read(iTime);
	bsData.Read(iLength);

	if(iLength > 512) return;

	bsData.Read(szMessage,iLength);
	szMessage[iLength] = '\0';

	pGame->DisplayGameText(szMessage,iTime,iType);
}

//----------------------------------------------------

void ScrSetInterior(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
	BYTE byteInterior;
	bsData.Read(byteInterior);
	
	//pChatWindow->AddDebugMessage("ScrSetInterior(%u)",byteInterior);

	pGame->FindPlayerPed()->SetInterior(byteInterior);	
}

//----------------------------------------------------

void ScrSetCameraPos(RPCParameters *rpcParams)
{
	RakNet::BitStream in(rpcParams->input,
		BYTES_TO_BITS(rpcParams->numberOfBitsOfData), false);
	VECTOR vecPos, vecRot;

	if(in.Read(vecPos) && in.Read(vecRot))
		pGame->GetCamera()->SetPosition(vecPos.X, vecPos.Y,
			vecPos.Z, vecRot.X, vecRot.Y, vecRot.Z);
}

//----------------------------------------------------

void ScrSetCameraLookAt(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
	VECTOR vecPos;
	bsData.Read(vecPos.X);
	bsData.Read(vecPos.Y);
	bsData.Read(vecPos.Z);
	pGame->GetCamera()->LookAtPoint(vecPos.X,vecPos.Y,vecPos.Z,2);	
}

//----------------------------------------------------

void ScrSetVehiclePos(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
	VEHICLEID VehicleId;
	float fX, fY, fZ;
	bsData.Read(VehicleId);
	bsData.Read(fX);
	bsData.Read(fY);
	bsData.Read(fZ);

	if(pNetGame && pNetGame->GetVehiclePool()->GetSlotState(VehicleId)) {
		pNetGame->GetVehiclePool()->GetAt(VehicleId)->TeleportTo(fX, fY, fZ);
	}
}

//----------------------------------------------------

void ScrSetVehicleZAngle(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
	VEHICLEID VehicleId;
	float fZAngle;
	bsData.Read(VehicleId);
	bsData.Read(fZAngle);

	ScriptCommand(&set_car_z_angle, pNetGame->GetVehiclePool()->GetAt(VehicleId)->m_dwGTAId, fZAngle);
}

//----------------------------------------------------

void ScrVehicleParams(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);

	VEHICLEID VehicleID;
	BYTE byteObjectiveVehicle;
	BYTE byteDoorsLocked;

	bsData.Read(VehicleID);
	bsData.Read(byteObjectiveVehicle);
	bsData.Read(byteDoorsLocked);

	CVehiclePool *pVehiclePool = pNetGame->GetVehiclePool();
	pVehiclePool->AssignSpecialParamsToVehicle(VehicleID,byteObjectiveVehicle,byteDoorsLocked);

}

//----------------------------------------------------

void ScrLinkVehicle(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);

	VEHICLEID VehicleID;
	BYTE byteInterior;

	bsData.Read(VehicleID);
	bsData.Read(byteInterior);

	CVehiclePool *pVehiclePool = pNetGame->GetVehiclePool();
	pVehiclePool->LinkToInterior(VehicleID, (int)byteInterior);
}

//----------------------------------------------------

void ScrSetCameraBehindPlayer(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	pGame->GetCamera()->SetBehindPlayer();	
}

//----------------------------------------------------

void ScrTogglePlayerControllable(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
	BYTE byteControllable;
	bsData.Read(byteControllable);
	pNetGame->GetPlayerPool()->GetLocalPlayer()->GetPlayerPed()->TogglePlayerControllable((int)byteControllable);
}

//----------------------------------------------------

void ScrPlaySound(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);

	int iSound;
	float fX, fY, fZ;
	bsData.Read(iSound);
	bsData.Read(fX);
	bsData.Read(fY);
	bsData.Read(fZ);

	pGame->PlaySound(iSound, fX, fY, fZ);
}

//----------------------------------------------------

void ScrSetWorldBounds(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
	bsData.Read(pNetGame->m_WorldBounds[0]);
	bsData.Read(pNetGame->m_WorldBounds[1]);
	bsData.Read(pNetGame->m_WorldBounds[2]);
	bsData.Read(pNetGame->m_WorldBounds[3]);
}

//----------------------------------------------------

void ScrHaveSomeMoney(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	int iAmount;
	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
	bsData.Read(iAmount);
	pGame->AddToLocalMoney(iAmount);
}

//----------------------------------------------------

void ScrSetPlayerFacingAngle(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	float fAngle;
	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
	bsData.Read(fAngle);
	pGame->FindPlayerPed()->ForceTargetRotation(fAngle);
}

//----------------------------------------------------

void ScrResetMoney(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	pGame->ResetLocalMoney();
}

//----------------------------------------------------

void ScrResetPlayerWeapons(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	CPlayerPed *pPlayerPed = pNetGame->GetPlayerPool()->GetLocalPlayer()->GetPlayerPed();
	pPlayerPed->ClearAllWeapons();
	//pChatWindow->AddDebugMessage("Cleared weapons");
}

//----------------------------------------------------

void ScrGivePlayerWeapon(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	int iWeaponID;
	int iAmmo;
	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
	bsData.Read(iWeaponID);
	bsData.Read(iAmmo);

	CPlayerPed *pPlayerPed = pNetGame->GetPlayerPool()->GetLocalPlayer()->GetPlayerPed();
	pPlayerPed->GiveWeapon(iWeaponID, iAmmo);
	//pChatWindow->AddDebugMessage("Gave weapon: %d with ammo %d", iWeaponID, iAmmo);
}

//----------------------------------------------------

void ScrRespawnVehicle(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	VEHICLEID VehicleID;
	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
	bsData.Read(VehicleID);

	CPlayerPed *pPlayerPed = pNetGame->GetPlayerPool()->GetLocalPlayer()->GetPlayerPed();	
	CVehiclePool *pVehiclePool = pNetGame->GetVehiclePool();
	CVehicle *pVehicle = pVehiclePool->GetAt(VehicleID);

	if(pVehicle) {
		if( (pPlayerPed->m_pPed) &&
			(pPlayerPed->m_pPed->pVehicle == (DWORD)pVehicle->m_pVehicle) ) {
			MATRIX4X4 mat;
			pPlayerPed->GetMatrix(&mat);
			pPlayerPed->RemoveFromVehicleAndPutAt(mat.pos.X,mat.pos.Y,mat.pos.Z + 1.0f);	
		}

		pVehiclePool->SetForRespawn(VehicleID);
	}
		
}

//----------------------------------------------------

void ScrDeathMessage(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	BYTE byteKiller, byteKillee, byteWeapon;
	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
	PCHAR szKillerName = NULL;
	PCHAR szKilleeName = NULL;
	DWORD dwKillerColor = 0;
	DWORD dwKilleeColor = 0;

	bsData.Read(byteKiller);
	bsData.Read(byteKillee);
	bsData.Read(byteWeapon);

	//pChatWindow->AddDebugMessage("RawDeath: %u %u %u",byteKiller,byteKillee,byteWeapon);

	if(byteKillee == INVALID_PLAYER_ID) return;

	// Determine the killer's name and color
	if(byteKiller == INVALID_PLAYER_ID) {
		szKillerName = NULL; dwKillerColor = 0;
	} else {
		if(pPlayerPool->GetLocalPlayerID() == byteKiller) {
			szKillerName = pPlayerPool->GetLocalPlayerName();
			dwKillerColor = pPlayerPool->GetLocalPlayer()->GetPlayerColorAsARGB();
		} else {
			if(pPlayerPool->GetSlotState(byteKiller)) {
				szKillerName = pPlayerPool->GetPlayerName(byteKiller);
				dwKillerColor = pPlayerPool->GetAt(byteKiller)->GetPlayerColorAsARGB();
			} else {
				//pChatWindow->AddDebugMessage("Slot State Killer FALSE");
				szKillerName = NULL; dwKillerColor = 0;
			}
		}
	}

	// Determine the killee's name and color
	if(pPlayerPool->GetLocalPlayerID() == byteKillee) {
		szKilleeName = pPlayerPool->GetLocalPlayerName();
		dwKilleeColor = pPlayerPool->GetLocalPlayer()->GetPlayerColorAsARGB();
	} else {
		if(pPlayerPool->GetSlotState(byteKillee)) {
			szKilleeName = pPlayerPool->GetPlayerName(byteKillee);
			dwKilleeColor = pPlayerPool->GetAt(byteKillee)->GetPlayerColorAsARGB();
		} else {
			//pChatWindow->AddDebugMessage("Slot State Killee FALSE");
			szKilleeName = NULL; dwKilleeColor = 0;
		}
	}

	if(pDeathWindow) pDeathWindow->AddMessage(szKillerName,szKilleeName,dwKillerColor,dwKilleeColor,byteWeapon);
}

//----------------------------------------------------

void ScrSetMapIcon(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);

	BYTE byteIndex;
	BYTE byteIcon;
	DWORD byteColor;
	float fPos[3];

	bsData.Read(byteIndex);
	bsData.Read(fPos[0]);
	bsData.Read(fPos[1]);
	bsData.Read(fPos[2]);
	bsData.Read(byteIcon);
	bsData.Read(byteColor);

	pNetGame->SetMapIcon(byteIndex, fPos[0], fPos[1], fPos[2], byteIcon, byteColor);
}

void ScrDisableMapIcon(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);

	BYTE byteIndex;

	bsData.Read(byteIndex);

	pNetGame->DisableMapIcon(byteIndex);
}

void ScrSetPlayerArmour(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	CLocalPlayer *pLocalPlayer = pNetGame->GetPlayerPool()->GetLocalPlayer();
	float fArmour;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
	bsData.Read(fArmour);

	pLocalPlayer->GetPlayerPed()->SetArmour(fArmour);
}

void ScrSetWeaponAmmo(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	CLocalPlayer *pLocalPlayer = pNetGame->GetPlayerPool()->GetLocalPlayer();
	BYTE byteWeapon;
	WORD wordAmmo;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
	bsData.Read(byteWeapon);
	bsData.Read(wordAmmo);

	pLocalPlayer->GetPlayerPed()->SetAmmo(byteWeapon, wordAmmo);
}

//----------------------------------------------------

void ScrSetGravity(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	float fGravity;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
	bsData.Read(fGravity);

	pGame->SetGravity(fGravity);
}

//----------------------------------------------------

void ScrSetVehicleHealth(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	float fVehicleHealth;
	VEHICLEID VehicleID;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
	bsData.Read(VehicleID);
	bsData.Read(fVehicleHealth);

	if (pNetGame->GetVehiclePool()->GetSlotState(VehicleID))
	{
		pNetGame->GetVehiclePool()->GetAt(VehicleID)->SetHealth(fVehicleHealth);
	}
}

//----------------------------------------------------

void ScrAttachTrailerToVehicle(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	VEHICLEID TrailerID, VehicleID;
	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
	bsData.Read(TrailerID);
	bsData.Read(VehicleID);
	CVehicle* pTrailer = pNetGame->GetVehiclePool()->GetAt(TrailerID);
	CVehicle* pVehicle = pNetGame->GetVehiclePool()->GetAt(VehicleID);
	
	pVehicle->SetTrailer(pTrailer);
	pVehicle->AttachTrailer();
}

//----------------------------------------------------

void ScrDetachTrailerFromVehicle(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	VEHICLEID VehicleID;
	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
	bsData.Read(VehicleID);
	CVehicle* pVehicle = pNetGame->GetVehiclePool()->GetAt(VehicleID);

	pVehicle->DetachTrailer();
	pVehicle->SetTrailer(NULL);
}

//----------------------------------------------------

void ScrCreateObject(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	byte byteObjectID;
	int iModel;
	VECTOR vecPos, vecRot;
	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
	bsData.Read(byteObjectID);
	bsData.Read(iModel);

	bsData.Read(vecPos.X);
	bsData.Read(vecPos.Y);
	bsData.Read(vecPos.Z);

	bsData.Read(vecRot.X);
	bsData.Read(vecRot.Y);
	bsData.Read(vecRot.Z);

	CObjectPool* pObjectPool =	pNetGame->GetObjectPool();
	pObjectPool->New(byteObjectID, iModel, vecPos, vecRot);
}

//----------------------------------------------------

void ScrSetObjectPos(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	byte byteObjectID;
	float fX, fY, fZ, fRotation;
	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
	bsData.Read(byteObjectID);
	bsData.Read(fX);
	bsData.Read(fY);
	bsData.Read(fZ);
	bsData.Read(fRotation);

	CObjectPool*	pObjectPool =	pNetGame->GetObjectPool();
	CObject*		pObject		=	pObjectPool->GetAt(byteObjectID);
	if (pObject)
	{
		pObject->TeleportTo(fX, fY, fZ);
	}
}

//----------------------------------------------------

void ScrSetObjectRotation(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	byte byteObjectID;
	float fX, fY, fZ;
	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
	bsData.Read(byteObjectID);
	bsData.Read(fX);
	bsData.Read(fY);
	bsData.Read(fZ);

	CObjectPool*	pObjectPool =	pNetGame->GetObjectPool();
	CObject*		pObject		=	pObjectPool->GetAt(byteObjectID);
	if (pObject)
	{
		pObject->InstantRotate(fX, fY, fZ);
	}
}

//----------------------------------------------------

void ScrDestroyObject(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	byte byteObjectID;
	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
	bsData.Read(byteObjectID);

	CObjectPool* pObjectPool =	pNetGame->GetObjectPool();
	if (pObjectPool->GetAt(byteObjectID))
	{
		pObjectPool->Delete(byteObjectID);
	}
}

//----------------------------------------------------

void ScrSetPlayerVirtualWorld(RPCParameters *rpcParams)
{
	// Sets which players are visible to the local player and which aren't
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	BYTE byteCount = rpcParams->numberOfBitsOfData / 8;
	RakNet::BitStream bsData(Data, byteCount + 1, false);
	byteCount /= 2;

	BYTE bytePlayer;
	BYTE byteVW;
	
	CPlayerPool* pPlayerPool = pNetGame->GetPlayerPool();

	BYTE byteLocal = pPlayerPool->GetLocalPlayerID();	
	
	for (BYTE i = 0; i < byteCount; i++)
	{
		bsData.Read(bytePlayer);
		bsData.Read(byteVW);
		if (bytePlayer == byteLocal)
		{
			CLocalPlayer* pPlayer = pPlayerPool->GetLocalPlayer();
			if (pPlayer) pPlayer->SetVirtualWorld(byteVW);
		}
		else
		{
			CRemotePlayer* pPlayer = pPlayerPool->GetAt(bytePlayer);
			if (pPlayer) pPlayer->SetVirtualWorld(byteVW);
		}
	}
}

//----------------------------------------------------

void ScrSetVehicleVirtualWorld(RPCParameters *rpcParams)
{
	// Sets which players are visible to the local player and which aren't
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iCount = rpcParams->numberOfBitsOfData / 8;
	RakNet::BitStream bsData(Data, iCount + 1, false);
	iCount /= (sizeof(VEHICLEID) + sizeof(BYTE));

	VEHICLEID Vehicle;
	BYTE byteVW;
	
	CVehiclePool* pVehiclePool = pNetGame->GetVehiclePool();
	
	for (int i = 0; i < iCount; i++)
	{
		bsData.Read((VEHICLEID)Vehicle);
		bsData.Read(byteVW);
		pVehiclePool->SetVehicleVirtualWorld(Vehicle, byteVW);
	}
}

void ScrCreateExplosion(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
	float X, Y, Z, Radius;
	int   iType;

	bsData.Read(X);
	bsData.Read(Y);
	bsData.Read(Z);
	bsData.Read(iType);
	bsData.Read(Radius);

	ScriptCommand(&create_explosion_with_radius, X, Y, Z, iType, Radius);
}

void ScrShowNameTag(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
	BYTE bytePlayerID;
	BYTE byteShow;

	bsData.Read(bytePlayerID);
	bsData.Read(byteShow);

	if (pNetGame->GetPlayerPool()->GetSlotState(bytePlayerID))
	{
		pNetGame->GetPlayerPool()->GetAt(bytePlayerID)->ShowNameTag(byteShow);
	}
}

void ScrMoveObject(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
	BYTE byteObjectID;
	float curx, cury, curz, newx, newy, newz, speed;

	bsData.Read(byteObjectID);
	bsData.Read(curx);
	bsData.Read(cury);
	bsData.Read(curz);
	bsData.Read(newx);
	bsData.Read(newy);
	bsData.Read(newz);
	bsData.Read(speed);

	CObject* pObject = pNetGame->GetObjectPool()->GetAt(byteObjectID);
	if (pObject)
	{
		pObject->TeleportTo(curx, cury, curz);
		pObject->MoveTo(newx, newy, newz, speed);
	}
}

void ScrStopObject(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
	BYTE byteObjectID;
	float newx, newy, newz;

	bsData.Read(byteObjectID);
	bsData.Read(newx);
	bsData.Read(newy);
	bsData.Read(newz);

	CObject* pObject = pNetGame->GetObjectPool()->GetAt(byteObjectID);
	if (pObject)
	{
		pObject->MoveTo(newx, newy, newz, pObject->m_fMoveSpeed);
	}
}

void ScrNumberPlate(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
	
	VEHICLEID Vehicle;
	CHAR cNumberPlate[9];
	
	bsData.Read(Vehicle);
	bsData.Read(cNumberPlate, 9);
	strcpy_s(pNetGame->GetVehiclePool()->m_charNumberPlate[Vehicle], cNumberPlate);
}

//----------------------------------------------------

void ScrTogglePlayerSpectating(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
	BOOL bToggle;
	bsData.Read(bToggle);
	pPlayerPool->GetLocalPlayer()->ToggleSpectating(bToggle);
}

void ScrSetPlayerSpectating(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	BYTE bytePlayerID;
	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
	bsData.Read(bytePlayerID);
	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
	if (pPlayerPool->GetSlotState(bytePlayerID)) {
		pPlayerPool->GetAt(bytePlayerID)->SetState(PLAYER_STATE_SPECTATING);
	}
}

#define SPECTATE_TYPE_NORMAL	1
#define SPECTATE_TYPE_FIXED		2
#define SPECTATE_TYPE_SIDE		3

void ScrPlayerSpectatePlayer(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
	
	BYTE bytePlayerID;
    BYTE byteMode;
	
	bsData.Read(bytePlayerID);
	bsData.Read(byteMode);

	switch (byteMode) {
		case SPECTATE_TYPE_FIXED:
			byteMode = 15;
			break;
		case SPECTATE_TYPE_SIDE:
			byteMode = 14;
			break;
		default:
			byteMode = 4;
	}
	CLocalPlayer *pLocalPlayer = pNetGame->GetPlayerPool()->GetLocalPlayer();
	pLocalPlayer->m_byteSpectateMode = byteMode;
	pLocalPlayer->SpectatePlayer(bytePlayerID);
}

void ScrPlayerSpectateVehicle(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);

	//pChatWindow->AddDebugMessage("ScrPlayerSpectateVehicle");
	
	VEHICLEID VehicleID;
    BYTE byteMode;
	
	bsData.Read(VehicleID);
	bsData.Read(byteMode);

	switch (byteMode) {
		case SPECTATE_TYPE_FIXED:
			byteMode = 15;
			break;
		case SPECTATE_TYPE_SIDE:
			byteMode = 14;
			break;
		default:
			byteMode = 3;
	}
	CLocalPlayer *pLocalPlayer = pNetGame->GetPlayerPool()->GetLocalPlayer();
	pLocalPlayer->m_byteSpectateMode = byteMode;
	pLocalPlayer->SpectateVehicle(VehicleID);
}

void ScrRemoveComponent(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
	
	VEHICLEID VehicleID;
    DWORD dwComponent;
	
	bsData.Read(VehicleID);
	bsData.Read(dwComponent);

	int iVehicleID;
	//int iComponent;

	if(!pNetGame) return;

	iVehicleID = pNetGame->GetVehiclePool()->FindGtaIDFromID(VehicleID);
	if(iVehicleID) ScriptCommand(&remove_component, iVehicleID, (int)dwComponent);
}

void ScrForceSpawnSelection(RPCParameters *rpcParams)
{
	pNetGame->GetPlayerPool()->GetLocalPlayer()->ReturnToClassSelection();
}

void ScrAttachObjectToPlayer(RPCParameters *rpcParams)
{
	PCHAR Data		= reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength	= rpcParams->numberOfBitsOfData;
	PlayerID sender	= rpcParams->sender;

	RakNet::BitStream bsData(Data, (iBitLength/8)+1, false);

	BYTE byteObjectID, bytePlayerID;
	float OffsetX, OffsetY, OffsetZ, rX, rY, rZ;

	bsData.Read( byteObjectID );
	bsData.Read( bytePlayerID );

	bsData.Read( OffsetX );
	bsData.Read( OffsetY );
	bsData.Read( OffsetZ );

	bsData.Read( rX );
	bsData.Read( rY );
	bsData.Read( rZ );

	try {

	CObject* pObject =	pNetGame->GetObjectPool()->GetAt(	byteObjectID );

	if ( !pObject )
		return;

	if ( bytePlayerID == pNetGame->GetPlayerPool()->GetLocalPlayerID() )
	{
		CLocalPlayer* pPlayer = pNetGame->GetPlayerPool()->GetLocalPlayer();
		ScriptCommand( &attach_object_to_actor, pObject->m_dwGTAId, pPlayer->GetPlayerPed()->m_dwGTAId,
			OffsetX,
			OffsetY,
			OffsetZ, 
			rX,
			rY,
			rZ);
	} else {
		CRemotePlayer* pPlayer =	pNetGame->GetPlayerPool()->GetAt(	bytePlayerID );

		if ( !pPlayer )
			return;

		ScriptCommand( &attach_object_to_actor, pObject->m_dwGTAId, pPlayer->GetPlayerPed()->m_dwGTAId,
																	OffsetX,
																	OffsetY,
																	OffsetZ, 
																	rX,
																	rY,
																	rZ);
	}

	} catch(...) {}
}

void ScrInitMenu(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
	
	if(!pNetGame) return;
	CMenuPool* pMenuPool = pNetGame->GetMenuPool();

	BYTE byteMenuID;
	BOOL bColumns; // 0 = 1, 1 = 2
	CHAR cText[MAX_MENU_LINE];
	float fX;
	float fY;
	float fCol1;
	float fCol2 = 0.0;
	MENU_INT MenuInteraction;
	
	bsData.Read(byteMenuID);
	bsData.Read(bColumns);
	bsData.Read(cText, MAX_MENU_LINE);
	bsData.Read(fX);
	bsData.Read(fY);
	bsData.Read(fCol1);
	if (bColumns) bsData.Read(fCol2);
	bsData.Read(MenuInteraction.bMenu);
	for (BYTE i = 0; i < MAX_MENU_ITEMS; i++)
	{
		bsData.Read(MenuInteraction.bRow[i]);
	}

	CMenu* pMenu;
	
	if (pMenuPool->GetSlotState(byteMenuID))
	{
		pMenuPool->Delete(byteMenuID);
	}
	
	pMenu = pMenuPool->New(byteMenuID, cText, fX, fY, ((BYTE)bColumns) + 1, fCol1, fCol2, &MenuInteraction);
	
	if (!pMenu) return;
	
	BYTE byteColCount;
	bsData.Read(cText, MAX_MENU_LINE);
	pMenu->SetColumnTitle(0, cText);
	
	bsData.Read(byteColCount);
	for (BYTE i = 0; i < byteColCount; i++)
	{
		bsData.Read(cText, MAX_MENU_LINE);
		pMenu->AddMenuItem(0, i, cText);
	}
	
	if (bColumns)
	{
		bsData.Read(cText, MAX_MENU_LINE);
		pMenu->SetColumnTitle(1, cText);
		
		bsData.Read(byteColCount);
		for (BYTE i = 0; i < byteColCount; i++)
		{
			bsData.Read(cText, MAX_MENU_LINE);
			pMenu->AddMenuItem(1, i, cText);
		}
	}
}

void ScrShowMenu(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
	
	if(!pNetGame) return;
	CMenuPool* pMenuPool = pNetGame->GetMenuPool();

	BYTE byteMenuID;
	bsData.Read(byteMenuID);
	pNetGame->GetMenuPool()->ShowMenu(byteMenuID);
}

void ScrHideMenu(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
	
	if(!pNetGame) return;
	CMenuPool* pMenuPool = pNetGame->GetMenuPool();

	BYTE byteMenuID;
	bsData.Read(byteMenuID);
	pNetGame->GetMenuPool()->HideMenu(byteMenuID);
}

void ScrSetPlayerWantedLevel(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
	
	if(!pGame) return;

	BYTE byteLevel;
	bsData.Read(byteLevel);
	pGame->SetWantedLevel(byteLevel);
}

void ScrShowTextDraw(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
	
	CTextDrawPool* pTextDrawPool = pNetGame->GetTextDrawPool();
	if (pTextDrawPool)
	{
		WORD wTextID;
		TEXT_DRAW_TRANSMIT TextDrawTransmit;
		CHAR cText[MAX_TEXT_DRAW_LINE];

		bsData.Read(wTextID);
		bsData.Read((PCHAR)&TextDrawTransmit, sizeof (TEXT_DRAW_TRANSMIT));
		bsData.Read(cText, MAX_TEXT_DRAW_LINE);
		pTextDrawPool->New(wTextID, &TextDrawTransmit, cText);
	}
}

void ScrHideTextDraw(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
	CTextDrawPool* pTextDrawPool = pNetGame->GetTextDrawPool();
	if (pTextDrawPool)
	{
		WORD wTextID;
		bsData.Read(wTextID);
		pTextDrawPool->Delete(wTextID);
	}
}

void ScrEditTextDraw(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
	CTextDrawPool* pTextDrawPool = pNetGame->GetTextDrawPool();
	if (pTextDrawPool)
	{
		WORD wTextID;
		CHAR cText[MAX_TEXT_DRAW_LINE];
		bsData.Read(wTextID);
		bsData.Read(cText, MAX_TEXT_DRAW_LINE);
		CTextDraw* pText = pTextDrawPool->GetAt(wTextID);
		if (pText) pText->SetText(cText);
	}
}

void ScrAddGangZone(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
	CGangZonePool* pGangZonePool = pNetGame->GetGangZonePool();
	if (pGangZonePool)
	{
		WORD wZoneID;
		float minx, miny, maxx, maxy;
		DWORD dwColor;
		bsData.Read(wZoneID);
		bsData.Read(minx);
		bsData.Read(miny);
		bsData.Read(maxx);
		bsData.Read(maxy);
		bsData.Read(dwColor);
//		pChatWindow->AddDebugMessage("called %d %f %f %f %f %x", wZoneID, minx, miny, maxx, maxy, dwColor);
		pGangZonePool->New(wZoneID, minx, miny, maxx, maxy, dwColor);
	}
}

void ScrRemoveGangZone(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
	CGangZonePool* pGangZonePool = pNetGame->GetGangZonePool();
	if (pGangZonePool)
	{
		WORD wZoneID;
		bsData.Read(wZoneID);
		pGangZonePool->Delete(wZoneID);
	}
}

void ScrFlashGangZone(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
	CGangZonePool* pGangZonePool = pNetGame->GetGangZonePool();
	if (pGangZonePool)
	{
		WORD wZoneID;
		DWORD dwColor;
		bsData.Read(wZoneID);
		bsData.Read(dwColor);
		pGangZonePool->Flash(wZoneID, dwColor);
	}
}

void ScrStopFlashGangZone(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);
	CGangZonePool* pGangZonePool = pNetGame->GetGangZonePool();
	if (pGangZonePool)
	{
		WORD wZoneID;
		bsData.Read(wZoneID);
		pGangZonePool->StopFlash(wZoneID);
	}
}

//----------------------------------------------------

void ScrApplyAnimation(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;
	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);

	BYTE bytePlayerID;
	BYTE byteAnimLibLen;
	BYTE byteAnimNameLen;
	char szAnimLib[256];
	char szAnimName[256];
	float fS;
	bool opt1,opt2,opt3,opt4;
	int  opt5;
	CPlayerPool *pPlayerPool=NULL;
	CPlayerPed *pPlayerPed=NULL;

	memset(szAnimLib,0,256);
	memset(szAnimName,0,256);

	bsData.Read(bytePlayerID);
	bsData.Read(byteAnimLibLen);
	bsData.Read(szAnimLib,byteAnimLibLen);
	bsData.Read(byteAnimNameLen);
	bsData.Read(szAnimName,byteAnimNameLen);
	bsData.Read(fS);
	bsData.Read(opt1);
	bsData.Read(opt2);
	bsData.Read(opt3);
	bsData.Read(opt4);
	bsData.Read(opt5);

	szAnimLib[byteAnimLibLen] = '\0';
	szAnimName[byteAnimNameLen] = '\0';

	pPlayerPool = pNetGame->GetPlayerPool();

	if(pPlayerPool) {
		// Get the CPlayerPed for this player
		if(bytePlayerID == pPlayerPool->GetLocalPlayerID()) {
			pPlayerPed = pPlayerPool->GetLocalPlayer()->GetPlayerPed();
		}
		else {
			if(pPlayerPool->GetSlotState(bytePlayerID)) {
				pPlayerPed = pPlayerPool->GetAt(bytePlayerID)->GetPlayerPed();
			}
		}
		if(pPlayerPed) {
			try {
				pPlayerPed->ApplyAnimation(szAnimName,szAnimLib,fS,
					(int)opt1,(int)opt2,(int)opt3,(int)opt4,(int)opt5);
			} catch(...) {}
		}
	}
}

//----------------------------------------------------

void ScrClearAnimations(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;
	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);

	BYTE bytePlayerID;
	bsData.Read(bytePlayerID);
	MATRIX4X4 mat;

	CPlayerPool *pPlayerPool=NULL;
	CPlayerPed *pPlayerPed=NULL;

	pPlayerPool = pNetGame->GetPlayerPool();

	if(pPlayerPool) {
		// Get the CPlayerPed for this player
		if(bytePlayerID == pPlayerPool->GetLocalPlayerID()) {
			pPlayerPed = pPlayerPool->GetLocalPlayer()->GetPlayerPed();
		}
		else {
			if(pPlayerPool->GetSlotState(bytePlayerID)) {
				pPlayerPed = pPlayerPool->GetAt(bytePlayerID)->GetPlayerPed();
			}
		}
		if(pPlayerPed) {
			try {
				
				pPlayerPed->GetMatrix(&mat);
				pPlayerPed->TeleportTo(mat.pos.X,mat.pos.Y,mat.pos.Z);

			} catch(...) {}
		}
	}
}

//----------------------------------------------------

void ScrSetSpecialAction(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;
	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);

	BYTE byteSpecialAction;
	bsData.Read(byteSpecialAction);
	
	CPlayerPool *pPool=pNetGame->GetPlayerPool();
	if(pPool) pPool->GetLocalPlayer()->ApplySpecialAction(byteSpecialAction);
}

//----------------------------------------------------

void ScrEnableStuntBonus(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;
	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);

	bool bStuntBonusEnabled;
	bsData.Read(bStuntBonusEnabled);
	pGame->EnableStuntBonus(bStuntBonusEnabled);
}

static void ScrSetVehicle(RPCParameters* rpcParams)
{
	RakNet::BitStream in(rpcParams->input,
		BYTES_TO_BITS(rpcParams->numberOfBitsOfData), false);

	int iOP = 0;
	VEHICLEID iVehicleID = 0;

	in.Read(iOP);
	in.Read(iVehicleID);

	CVehicle* pVehicle = pNetGame->GetVehiclePool()->GetAt(iVehicleID);
	if (pVehicle == NULL)
		return;
	
	switch (iOP)
	{
	case 1:
		pVehicle->Fix();
		break;
	case 2:
		int d, p, bl, br;

		in.Read(d);
		in.Read(p);
		in.Read(bl);
		in.Read(br);

		pVehicle->ToggleWindow(10, !!d);
		pVehicle->ToggleWindow(8, !!p);
		pVehicle->ToggleWindow(11, !!bl);
		pVehicle->ToggleWindow(9, !!br);
		break;
	case 3:
	{
		int iOn = 0;
		in.Read(iOn);
		pVehicle->ToggleTaxiLight(!!iOn);
		break;
	}
	case 4:
	{
		int iEngineState = 0;
		in.Read(iEngineState);
		pVehicle->ToggleEngine(!!iEngineState);
		break;
	}
	case 5:
	{
		unsigned char ucState = 0;
		in.Read(ucState);
		pVehicle->SetLightState(ucState);
		break;
	}
	}
}

static void ScrSetPlayer(RPCParameters* rpcParams)
{
	RakNet::BitStream in(rpcParams->input,
		BYTES_TO_BITS(rpcParams->numberOfBitsOfData), false);

	int iOP = 0;
	in.Read(iOP);
	switch (iOP)
	{
	case 1:
	{
		float fDrunkLevel = 0.0f;
		in.Read(fDrunkLevel);
		pGame->SetDrunkLevel(fDrunkLevel);
		break;
	}
	case 2:
	{
		unsigned char ucSkill = 0;
		unsigned int uiLevel = 0;
		in.Read(ucSkill);
		in.Read(uiLevel);
		pGame->SetWeaponSkill(ucSkill, uiLevel);
		break;
	}
	case 3:
	{
		unsigned char ucWeaponId = 0;
		in.Read(ucWeaponId);
		pNetGame->GetPlayerPool()->GetLocalPlayer()->GetPlayerPed()->SetArmedWeapon(ucWeaponId);
		break;
	}
	case 4:
	{
		unsigned char style, move;
		in.Read(style);
		in.Read(move);
		pNetGame->GetPlayerPool()->GetLocalPlayer()->GetPlayerPed()->SetFightingStyle(style, move);
		break;
	}
	case 5://Max Health
	{
		float fVal = 0.0f;
		in.Read(fVal);
		pGame->SetMaxHealth(fVal);
		break;
	}
	case 6://Blur Level 
	{
		unsigned char level;
		in.Read(level);
		pGame->SetBlurLevel(level);
		break;
	}
}

static void ScrVehicleVelocity(RPCParameters* rpcParams)
{
	RakNet::BitStream in(rpcParams->input,
		BYTES_TO_BITS(rpcParams->numberOfBitsOfData), false);

	int iVehicleId = -1;
	float fX = 0.0f, fY = 0.0f, fZ = 0.0f;

	if (!in.Read(iVehicleId) && iVehicleId < 0)
		return;

	CVehicle* pVehicle = pNetGame->GetVehiclePool()->GetAt(iVehicleId);
	if (pVehicle == NULL && in.GetNumberOfUnreadBits() < 96)
		return;
	
	in.Read(fX);
	in.Read(fY);
	in.Read(fZ);

	pVehicle->SetMoveSpeedVector({ fX, fY, fZ });
	pVehicle->ApplyMoveSpeed();
}

static void ScrPlayerVelocity(RPCParameters* rpcParams)
{
	RakNet::BitStream in(rpcParams->input,
		BITS_TO_BYTES(rpcParams->numberOfBitsOfData), false);

	if (in.GetNumberOfUnreadBits() != 128)
		return;

	int iPlayerId = -1;
	float fX = 0.0f, fY = 0.0f, fZ = 0.0f;

	if (!in.Read(iPlayerId) && iPlayerId < 0)
		return;

	CPlayerPed* pPlayerPed = NULL;
	CPlayerPool* pPool = pNetGame->GetPlayerPool();
	if (pPool) {
		if (iPlayerId == pPool->GetLocalPlayerID())
			pPlayerPed = pPool->GetLocalPlayer()->GetPlayerPed();
		else
			if (pPool->GetSlotState(iPlayerId))
				pPlayerPed = pPool->GetAt(iPlayerId)->GetPlayerPed();

		if (pPlayerPed)
		{
			in.Read(fX);
			in.Read(fY);
			in.Read(fZ);

			pPlayerPed->SetMoveSpeedVector({ fX, fY, fZ });
			pPlayerPed->ApplyMoveSpeed();
		}
	}
}

static void ScrInterpolateCamera(RPCParameters* rpcParams)
{
	RakNet::BitStream in(rpcParams->input, BITS_TO_BYTES(rpcParams->numberOfBitsOfData), false);

	BYTE type, mode;
	VECTOR from, to;
	FLOAT time;
	
	in.Read(type);
	in.Read(from.X);
	in.Read(from.Y);
	in.Read(from.Z);
	in.Read(to.X);
	in.Read(to.Y);
	in.Read(to.Z);
	in.Read(time);
	in.Read(mode);

	if (0.0f < time && (mode == 1 || mode == 2))
	{
		if (type)
			pGame->GetCamera()->InterpolateCameraPos(&from, &to, time, mode);
		else
			pGame->GetCamera()->InterpolateCameraLookAt(&from, &to, time, mode);
	}
}

//----------------------------------------------------

void RegisterScriptRPCs(RakClientInterface* pRakClient)
{
	REGISTER_STATIC_RPC(pRakClient, ScrSetSpawnInfo);
	REGISTER_STATIC_RPC(pRakClient, ScrSetPlayerTeam);
	REGISTER_STATIC_RPC(pRakClient, ScrSetPlayerSkin);
	REGISTER_STATIC_RPC(pRakClient, ScrSetPlayerName);
	REGISTER_STATIC_RPC(pRakClient, ScrSetPlayerPos);
	REGISTER_STATIC_RPC(pRakClient, ScrSetPlayerPosFindZ);
	REGISTER_STATIC_RPC(pRakClient, ScrSetPlayerHealth);
	REGISTER_STATIC_RPC(pRakClient, ScrPutPlayerInVehicle);
	REGISTER_STATIC_RPC(pRakClient, ScrRemovePlayerFromVehicle);
	REGISTER_STATIC_RPC(pRakClient, ScrSetPlayerColor);
	REGISTER_STATIC_RPC(pRakClient, ScrDisplayGameText);
	REGISTER_STATIC_RPC(pRakClient, ScrSetInterior);
	REGISTER_STATIC_RPC(pRakClient, ScrSetCameraPos);
	REGISTER_STATIC_RPC(pRakClient, ScrSetCameraLookAt);
	REGISTER_STATIC_RPC(pRakClient, ScrSetVehiclePos);
	REGISTER_STATIC_RPC(pRakClient, ScrSetVehicleZAngle);
	REGISTER_STATIC_RPC(pRakClient, ScrVehicleParams);
	REGISTER_STATIC_RPC(pRakClient, ScrSetCameraBehindPlayer);
	REGISTER_STATIC_RPC(pRakClient, ScrTogglePlayerControllable);
	REGISTER_STATIC_RPC(pRakClient, ScrPlaySound);
	REGISTER_STATIC_RPC(pRakClient, ScrSetWorldBounds);
	REGISTER_STATIC_RPC(pRakClient, ScrHaveSomeMoney);
	REGISTER_STATIC_RPC(pRakClient, ScrSetPlayerFacingAngle);
	REGISTER_STATIC_RPC(pRakClient, ScrResetMoney);
	REGISTER_STATIC_RPC(pRakClient, ScrResetPlayerWeapons);
	REGISTER_STATIC_RPC(pRakClient, ScrGivePlayerWeapon);
	REGISTER_STATIC_RPC(pRakClient, ScrRespawnVehicle);
	REGISTER_STATIC_RPC(pRakClient, ScrLinkVehicle);
	REGISTER_STATIC_RPC(pRakClient, ScrSetPlayerArmour);
	REGISTER_STATIC_RPC(pRakClient, ScrDeathMessage);
	//REGISTER_STATIC_RPC(pRakClient, ScrSetPlayerMarker);
	REGISTER_STATIC_RPC(pRakClient, ScrSetMapIcon);
	REGISTER_STATIC_RPC(pRakClient, ScrDisableMapIcon);
	REGISTER_STATIC_RPC(pRakClient, ScrSetWeaponAmmo);
	REGISTER_STATIC_RPC(pRakClient, ScrSetGravity);
	REGISTER_STATIC_RPC(pRakClient, ScrSetVehicleHealth);
	REGISTER_STATIC_RPC(pRakClient, ScrAttachTrailerToVehicle);
	REGISTER_STATIC_RPC(pRakClient, ScrDetachTrailerFromVehicle);
	REGISTER_STATIC_RPC(pRakClient, ScrCreateObject);
	REGISTER_STATIC_RPC(pRakClient, ScrSetObjectPos);
	REGISTER_STATIC_RPC(pRakClient, ScrSetObjectRotation);
	REGISTER_STATIC_RPC(pRakClient, ScrDestroyObject);
	REGISTER_STATIC_RPC(pRakClient, ScrSetPlayerVirtualWorld);
	REGISTER_STATIC_RPC(pRakClient, ScrSetVehicleVirtualWorld);
	REGISTER_STATIC_RPC(pRakClient, ScrCreateExplosion);
	REGISTER_STATIC_RPC(pRakClient, ScrShowNameTag);
	REGISTER_STATIC_RPC(pRakClient, ScrMoveObject);
	REGISTER_STATIC_RPC(pRakClient, ScrStopObject);
	REGISTER_STATIC_RPC(pRakClient, ScrNumberPlate);
	REGISTER_STATIC_RPC(pRakClient, ScrTogglePlayerSpectating);
	REGISTER_STATIC_RPC(pRakClient, ScrSetPlayerSpectating);
	REGISTER_STATIC_RPC(pRakClient, ScrPlayerSpectatePlayer);
	REGISTER_STATIC_RPC(pRakClient, ScrPlayerSpectateVehicle);
	REGISTER_STATIC_RPC(pRakClient, ScrRemoveComponent);
	REGISTER_STATIC_RPC(pRakClient, ScrForceSpawnSelection);
	REGISTER_STATIC_RPC(pRakClient, ScrAttachObjectToPlayer);
	REGISTER_STATIC_RPC(pRakClient, ScrInitMenu);
	REGISTER_STATIC_RPC(pRakClient, ScrShowMenu);
	REGISTER_STATIC_RPC(pRakClient, ScrHideMenu);
	REGISTER_STATIC_RPC(pRakClient, ScrSetPlayerWantedLevel);
	REGISTER_STATIC_RPC(pRakClient, ScrShowTextDraw);
	REGISTER_STATIC_RPC(pRakClient, ScrHideTextDraw);
	REGISTER_STATIC_RPC(pRakClient, ScrEditTextDraw);
	REGISTER_STATIC_RPC(pRakClient, ScrAddGangZone);
	REGISTER_STATIC_RPC(pRakClient, ScrRemoveGangZone);
	REGISTER_STATIC_RPC(pRakClient, ScrFlashGangZone);
	REGISTER_STATIC_RPC(pRakClient, ScrStopFlashGangZone);
	REGISTER_STATIC_RPC(pRakClient, ScrApplyAnimation);
	REGISTER_STATIC_RPC(pRakClient, ScrClearAnimations);
	REGISTER_STATIC_RPC(pRakClient, ScrSetSpecialAction);
	REGISTER_STATIC_RPC(pRakClient, ScrEnableStuntBonus);
	REGISTER_STATIC_RPC(pRakClient, ScrSetVehicle);
	REGISTER_STATIC_RPC(pRakClient, ScrSetPlayer);
	REGISTER_STATIC_RPC(pRakClient, ScrVehicleVelocity);
	REGISTER_STATIC_RPC(pRakClient, ScrPlayerVelocity);
	REGISTER_STATIC_RPC(pRakClient, ScrInterpolateCamera);
}

//----------------------------------------------------

void UnRegisterScriptRPCs(RakClientInterface* pRakClient)
{
	UNREGISTER_STATIC_RPC(pRakClient, ScrSetSpawnInfo); // 32
	UNREGISTER_STATIC_RPC(pRakClient, ScrSetPlayerTeam);
	UNREGISTER_STATIC_RPC(pRakClient, ScrSetPlayerName);
	UNREGISTER_STATIC_RPC(pRakClient, ScrSetPlayerSkin);
	UNREGISTER_STATIC_RPC(pRakClient, ScrSetPlayerPos);
	UNREGISTER_STATIC_RPC(pRakClient, ScrSetPlayerPosFindZ);
	UNREGISTER_STATIC_RPC(pRakClient, ScrSetPlayerHealth);
	UNREGISTER_STATIC_RPC(pRakClient, ScrPutPlayerInVehicle);
	UNREGISTER_STATIC_RPC(pRakClient, ScrRemovePlayerFromVehicle);
	UNREGISTER_STATIC_RPC(pRakClient, ScrSetPlayerColor);
	UNREGISTER_STATIC_RPC(pRakClient, ScrDisplayGameText);
	UNREGISTER_STATIC_RPC(pRakClient, ScrSetInterior);
	UNREGISTER_STATIC_RPC(pRakClient, ScrSetCameraPos);
	UNREGISTER_STATIC_RPC(pRakClient, ScrSetCameraLookAt);
	UNREGISTER_STATIC_RPC(pRakClient, ScrSetVehiclePos);
	UNREGISTER_STATIC_RPC(pRakClient, ScrSetVehicleZAngle);
	UNREGISTER_STATIC_RPC(pRakClient, ScrVehicleParams);
	UNREGISTER_STATIC_RPC(pRakClient, ScrSetCameraBehindPlayer);
	UNREGISTER_STATIC_RPC(pRakClient, ScrTogglePlayerControllable); // 50
	UNREGISTER_STATIC_RPC(pRakClient, ScrPlaySound);
	UNREGISTER_STATIC_RPC(pRakClient, ScrSetWorldBounds);
	UNREGISTER_STATIC_RPC(pRakClient, ScrHaveSomeMoney);
	UNREGISTER_STATIC_RPC(pRakClient, ScrSetPlayerFacingAngle);
	UNREGISTER_STATIC_RPC(pRakClient, ScrResetMoney);
	UNREGISTER_STATIC_RPC(pRakClient, ScrResetPlayerWeapons);
	UNREGISTER_STATIC_RPC(pRakClient, ScrGivePlayerWeapon);
	UNREGISTER_STATIC_RPC(pRakClient, ScrRespawnVehicle); // 58
	UNREGISTER_STATIC_RPC(pRakClient, ScrLinkVehicle);
	UNREGISTER_STATIC_RPC(pRakClient, ScrDeathMessage);
	//UNREGISTER_STATIC_RPC(pRakClient, ScrSetPlayerMarker);
	UNREGISTER_STATIC_RPC(pRakClient, ScrSetMapIcon);
	UNREGISTER_STATIC_RPC(pRakClient, ScrDisableMapIcon);
	UNREGISTER_STATIC_RPC(pRakClient, ScrSetWeaponAmmo);
	UNREGISTER_STATIC_RPC(pRakClient, ScrSetGravity);
	UNREGISTER_STATIC_RPC(pRakClient, ScrSetVehicleHealth);
	UNREGISTER_STATIC_RPC(pRakClient, ScrAttachTrailerToVehicle);
	UNREGISTER_STATIC_RPC(pRakClient, ScrDetachTrailerFromVehicle);
	UNREGISTER_STATIC_RPC(pRakClient, ScrCreateObject);
	UNREGISTER_STATIC_RPC(pRakClient, ScrSetObjectPos);
	UNREGISTER_STATIC_RPC(pRakClient, ScrSetObjectRotation);
	UNREGISTER_STATIC_RPC(pRakClient, ScrDestroyObject); // Possible problem source
	UNREGISTER_STATIC_RPC(pRakClient, ScrSetPlayerVirtualWorld);
	UNREGISTER_STATIC_RPC(pRakClient, ScrSetVehicleVirtualWorld);
	UNREGISTER_STATIC_RPC(pRakClient, ScrCreateExplosion);
	UNREGISTER_STATIC_RPC(pRakClient, ScrShowNameTag);
	UNREGISTER_STATIC_RPC(pRakClient, ScrMoveObject);
	UNREGISTER_STATIC_RPC(pRakClient, ScrStopObject);
	UNREGISTER_STATIC_RPC(pRakClient, ScrNumberPlate);
	UNREGISTER_STATIC_RPC(pRakClient, ScrTogglePlayerSpectating);
	UNREGISTER_STATIC_RPC(pRakClient, ScrSetPlayerSpectating);
	UNREGISTER_STATIC_RPC(pRakClient, ScrPlayerSpectatePlayer);
	UNREGISTER_STATIC_RPC(pRakClient, ScrPlayerSpectateVehicle);
	UNREGISTER_STATIC_RPC(pRakClient, ScrRemoveComponent);
	UNREGISTER_STATIC_RPC(pRakClient, ScrForceSpawnSelection);
	UNREGISTER_STATIC_RPC(pRakClient, ScrAttachObjectToPlayer);
	UNREGISTER_STATIC_RPC(pRakClient, ScrInitMenu);
	UNREGISTER_STATIC_RPC(pRakClient, ScrShowMenu);
	UNREGISTER_STATIC_RPC(pRakClient, ScrHideMenu);
	UNREGISTER_STATIC_RPC(pRakClient, ScrSetPlayerWantedLevel);
	UNREGISTER_STATIC_RPC(pRakClient, ScrShowTextDraw);
	UNREGISTER_STATIC_RPC(pRakClient, ScrHideTextDraw);
	UNREGISTER_STATIC_RPC(pRakClient, ScrEditTextDraw);
	UNREGISTER_STATIC_RPC(pRakClient, ScrAddGangZone);
	UNREGISTER_STATIC_RPC(pRakClient, ScrRemoveGangZone);
	UNREGISTER_STATIC_RPC(pRakClient, ScrFlashGangZone);
	UNREGISTER_STATIC_RPC(pRakClient, ScrStopFlashGangZone);
	UNREGISTER_STATIC_RPC(pRakClient, ScrApplyAnimation);
	UNREGISTER_STATIC_RPC(pRakClient, ScrClearAnimations);
	UNREGISTER_STATIC_RPC(pRakClient, ScrSetSpecialAction);
	UNREGISTER_STATIC_RPC(pRakClient, ScrEnableStuntBonus);
	UNREGISTER_STATIC_RPC(pRakClient, ScrSetVehicle);
	UNREGISTER_STATIC_RPC(pRakClient, ScrSetPlayer);
	UNREGISTER_STATIC_RPC(pRakClient, ScrVehicleVelocity);
	UNREGISTER_STATIC_RPC(pRakClient, ScrInterpolateCamera);
}

//----------------------------------------------------
