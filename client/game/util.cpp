//----------------------------------------------------------
//
// SA:MP Multiplayer Modification For GTA:SA
// Copyright 2004-2005 SA:MP team
//
// Version: $Id: util.cpp,v 1.21 2006/06/18 10:20:21 kyeman Exp $
//
//----------------------------------------------------------

#include <windows.h>
#include <stdio.h>
#include <time.h>
#include "game.h"
#include "../main.h"

DWORD dwPlayerPedPtrs[MAX_PLAYERS];

#define NUM_RADAR_COLORS 200

DWORD dwHudColors[NUM_RADAR_COLORS] = {
0xFF8C13FF, // dark orange
0xC715FFFF, // Medium violet red
0x20B2AAFF, // sea green
0xDC143CFF, // crimson
0x6495EDFF, // cornflower blue
0xf0e68cFF, // khaki
0x778899FF, // light slate grey
0xFF1493FF, // deeppink
0xF4A460FF, // sandy
0xEE82EEFF, // violet
0xFFD720FF, // gold
0x8b4513FF, // chocolate
0x4949A0FF, // midnight blue
0x148b8bFF, // dark cyan
0x14ff7fFF, // spring green
0x556b2fFF,  // olive green
0x0FD9FAFF,
0x10DC29FF,
0x534081FF,
0x0495CDFF,
0xEF6CE8FF,
0xBD34DAFF,
0x247C1BFF,
0x0C8E5DFF,
0x635B03FF,
0xCB7ED3FF,
0x65ADEBFF,
0x5C1ACCFF,
0xF2F853FF,
0x11F891FF,
0x7B39AAFF,
0x53EB10FF,
0x54137DFF,
0x275222FF,
0xF09F5BFF,
0x3D0A4FFF,
0x22F767FF,
0xD63034FF,
0x9A6980FF,
0xDFB935FF,
0x3793FAFF,
0x90239DFF,
0xE9AB2FFF,
0xAF2FF3FF,
0x057F94FF,
0xB98519FF,
0x388EEAFF,
0x028151FF,
0xA55043FF,
0x0DE018FF,
0x93AB1CFF,
0x95BAF0FF,
0x369976FF,
0x18F71FFF,
0x4B8987FF,
0x491B9EFF,
0x829DC7FF,
0xBCE635FF,
0xCEA6DFFF,
0x20D4ADFF,
0x2D74FDFF,
0x3C1C0DFF,
0x12D6D4FF,
0x48C000FF,
0x2A51E2FF,
0xE3AC12FF,
0xFC42A8FF,
0x2FC827FF,
0x1A30BFFF,
0xB740C2FF,
0x42ACF5FF,
0x2FD9DEFF,
0xFAFB71FF,
0x05D1CDFF,
0xC471BDFF,
0x94436EFF,
0xC1F7ECFF,
0xCE79EEFF,
0xBD1EF2FF,
0x93B7E4FF,
0x3214AAFF,
0x184D3BFF,
0xAE4B99FF,
0x7E49D7FF,
0x4C436EFF,
0xFA24CCFF,
0xCE76BEFF,
0xA04E0AFF,
0x9F945CFF,
0xDCDE3DFF,
0x10C9C5FF,
0x70524DFF,
0x0BE472FF,
0x8A2CD7FF,
0x6152C2FF,
0xCF72A9FF,
0xE59338FF,
0xEEDC2DFF,
0xD8C762FF,
0x3FE65CFF, // (100)
0xFF8C13FF, // dark orange
0xC715FFFF, // Medium violet red
0x20B2AAFF, // sea green
0xDC143CFF, // crimson
0x6495EDFF, // cornflower blue
0xf0e68cFF, // khaki
0x778899FF, // light slate grey
0xFF1493FF, // deeppink
0xF4A460FF, // sandy
0xEE82EEFF, // violet
0xFFD720FF, // gold
0x8b4513FF, // chocolate
0x4949A0FF, // midnight blue
0x148b8bFF, // dark cyan
0x14ff7fFF, // spring green
0x556b2fFF,  // olive green
0x0FD9FAFF,
0x10DC29FF,
0x534081FF,
0x0495CDFF,
0xEF6CE8FF,
0xBD34DAFF,
0x247C1BFF,
0x0C8E5DFF,
0x635B03FF,
0xCB7ED3FF,
0x65ADEBFF,
0x5C1ACCFF,
0xF2F853FF,
0x11F891FF,
0x7B39AAFF,
0x53EB10FF,
0x54137DFF,
0x275222FF,
0xF09F5BFF,
0x3D0A4FFF,
0x22F767FF,
0xD63034FF,
0x9A6980FF,
0xDFB935FF,
0x3793FAFF,
0x90239DFF,
0xE9AB2FFF,
0xAF2FF3FF,
0x057F94FF,
0xB98519FF,
0x388EEAFF,
0x028151FF,
0xA55043FF,
0x0DE018FF,
0x93AB1CFF,
0x95BAF0FF,
0x369976FF,
0x18F71FFF,
0x4B8987FF,
0x491B9EFF,
0x829DC7FF,
0xBCE635FF,
0xCEA6DFFF,
0x20D4ADFF,
0x2D74FDFF,
0x3C1C0DFF,
0x12D6D4FF,
0x48C000FF,
0x2A51E2FF,
0xE3AC12FF,
0xFC42A8FF,
0x2FC827FF,
0x1A30BFFF,
0xB740C2FF,
0x42ACF5FF,
0x2FD9DEFF,
0xFAFB71FF,
0x05D1CDFF,
0xC471BDFF,
0x94436EFF,
0xC1F7ECFF,
0xCE79EEFF,
0xBD1EF2FF,
0x93B7E4FF,
0x3214AAFF,
0x184D3BFF,
0xAE4B99FF,
0x7E49D7FF,
0x4C436EFF,
0xFA24CCFF,
0xCE76BEFF,
0xA04E0AFF,
0x9F945CFF,
0xDCDE3DFF,
0x10C9C5FF,
0x70524DFF,
0x0BE472FF,
0x8A2CD7FF,
0x6152C2FF,
0xCF72A9FF,
0xE59338FF,
0xEEDC2DFF,
0xD8C762FF,
0x3FE65CFF
};

DWORD dwUseHudColors[NUM_RADAR_COLORS];

//-----------------------------------------------------------

void  ProcessLineOfSight(VECTOR *vecOrigin, VECTOR *vecLine, VECTOR *colPoint,
		DWORD *pHitEntity, int bCheckBuildings, int bCheckVehicles, int bCheckPeds,
		int bCheckObjects, int bCheckDummies, int bSeeThroughStuff,
		int  bIgnoreSomeObjectsForCamera, int bUnk1)
{	
}

//-----------------------------------------------------------

void __stdcall WorldAddEntity(DWORD *dwEnt)
{
	_asm push dwEnt
	_asm mov ebx, 0x563220
	_asm call ebx
	_asm pop ebx
}

//-----------------------------------------------------------

void __stdcall WorldRemoveEntity(DWORD *dwEnt)
{
	_asm push dwEnt
	_asm mov ebx, 0x563280
	_asm call ebx
	_asm pop ebx
}

//-----------------------------------------------------------

void __stdcall GameDisableCheatCodes()
{

}

//-----------------------------------------------------------

PED_TYPE * __stdcall GamePool_Ped_GetAt(int iID)
{
	PED_TYPE *pActorRet;

	_asm mov ebx, ADDR_PED_TABLE
	_asm mov ecx, [ebx]
	_asm push iID
	_asm mov ebx, ADDR_ACTOR_FROM_ID
	_asm call ebx
	_asm mov pActorRet, eax

	return pActorRet;	
}

//-----------------------------------------------------------

int __stdcall GamePool_Ped_GetIndex(PED_TYPE *pActor)
{
	int iRetVal;

	_asm mov ebx, ADDR_PED_TABLE
	_asm mov ecx, [ebx]
	_asm push pActor
	_asm mov ebx, ADDR_ID_FROM_ACTOR
	_asm call ebx
	_asm mov iRetVal, eax

	return iRetVal;
}

//-----------------------------------------------------------

VEHICLE_TYPE * __stdcall GamePool_Vehicle_GetAt(int iID)
{	
	VEHICLE_TYPE *pVehicleRet;

	_asm mov ebx, ADDR_VEHICLE_TABLE
	_asm mov ecx, [ebx]
	_asm push iID
	_asm mov ebx, ADDR_VEHICLE_FROM_ID
	_asm call ebx
	_asm mov pVehicleRet, eax

	return pVehicleRet;
}

//-----------------------------------------------------------

DWORD __stdcall GamePool_Vehicle_GetIndex(VEHICLE_TYPE *pVehicle)
{
	DWORD dwID=0;

	_asm mov eax, ADDR_VEHICLE_TABLE
	_asm mov ecx, [eax]
	_asm push pVehicle
	_asm mov edx, 0x424160
	_asm call edx
	_asm mov dwID, eax

	return dwID;
}

//-----------------------------------------------------------

ENTITY_TYPE * __stdcall GamePool_Object_GetAt(int iID)
{	
	ENTITY_TYPE *pObjectRet;

	_asm mov ebx, 0xB7449C
	_asm mov ecx, [ebx]
	_asm push iID
	_asm mov ebx, 0x465040
	_asm call ebx
	_asm mov pObjectRet, eax

	return pObjectRet;
}

//-----------------------------------------------------------
// Return the PED_TYPE * of the local player actor.

PED_TYPE * __stdcall GamePool_FindPlayerPed()
{
	return *(PED_TYPE **)(0xB7CD98);
}

//-----------------------------------------------------------
// Translate Weapon model ID into actual weapon ID.

int __stdcall GameGetWeaponModelIDFromWeaponID(int iWeaponID)
{
	switch(iWeaponID)
	{
	case WEAPON_BRASSKNUCKLE:
		return WEAPON_MODEL_BRASSKNUCKLE;

	case WEAPON_GOLFCLUB:
		return WEAPON_MODEL_GOLFCLUB;

	case WEAPON_NITESTICK:
		return WEAPON_MODEL_NITESTICK;

	case WEAPON_KNIFE:
		return WEAPON_MODEL_KNIFE;

	case WEAPON_BAT:
		return WEAPON_MODEL_BAT;

	case WEAPON_SHOVEL:
		return WEAPON_MODEL_SHOVEL;

	case WEAPON_POOLSTICK:
		return WEAPON_MODEL_POOLSTICK;

	case WEAPON_KATANA:
		return WEAPON_MODEL_KATANA;

	case WEAPON_CHAINSAW:
		return WEAPON_MODEL_CHAINSAW;

	case WEAPON_DILDO:
		return WEAPON_MODEL_DILDO;

	case WEAPON_DILDO2:
		return WEAPON_MODEL_DILDO2;

	case WEAPON_VIBRATOR:
		return WEAPON_MODEL_VIBRATOR;

	case WEAPON_VIBRATOR2:
		return WEAPON_MODEL_VIBRATOR2;

	case WEAPON_FLOWER:
		return WEAPON_MODEL_FLOWER;

	case WEAPON_CANE:
		return WEAPON_MODEL_CANE;

	case WEAPON_GRENADE:
		return WEAPON_MODEL_GRENADE;

	case WEAPON_TEARGAS:
		return WEAPON_MODEL_TEARGAS;

	case WEAPON_MOLOTOV:
		return WEAPON_MODEL_MOLOTOV;

	case WEAPON_COLT45:
		return WEAPON_MODEL_COLT45;

	case WEAPON_SILENCED:
		return WEAPON_MODEL_SILENCED;

	case WEAPON_DEAGLE:
		return WEAPON_MODEL_DEAGLE;

	case WEAPON_SHOTGUN:
		return WEAPON_MODEL_SHOTGUN;

	case WEAPON_SAWEDOFF:
		return WEAPON_MODEL_SAWEDOFF;

	case WEAPON_SHOTGSPA:
		return WEAPON_MODEL_SHOTGSPA;

	case WEAPON_UZI:
		return WEAPON_MODEL_UZI;

	case WEAPON_MP5:
		return WEAPON_MODEL_MP5;

	case WEAPON_AK47:
		return WEAPON_MODEL_AK47;

	case WEAPON_M4:
		return WEAPON_MODEL_M4;

	case WEAPON_TEC9:
		return WEAPON_MODEL_TEC9;

	case WEAPON_RIFLE:
		return WEAPON_MODEL_RIFLE;

	case WEAPON_SNIPER:
		return WEAPON_MODEL_SNIPER;

	case WEAPON_ROCKETLAUNCHER:
		return WEAPON_MODEL_ROCKETLAUNCHER;

	case WEAPON_HEATSEEKER:
		return WEAPON_MODEL_HEATSEEKER;

	case WEAPON_FLAMETHROWER:
		return WEAPON_MODEL_FLAMETHROWER;

	case WEAPON_MINIGUN:
		return WEAPON_MODEL_MINIGUN;

	case WEAPON_SATCHEL:
		return WEAPON_MODEL_SATCHEL;

	case WEAPON_BOMB:
		return WEAPON_MODEL_BOMB;

	case WEAPON_SPRAYCAN:
		return WEAPON_MODEL_SPRAYCAN;

	case WEAPON_FIREEXTINGUISHER:
		return WEAPON_MODEL_FIREEXTINGUISHER;

	case WEAPON_CAMERA:
		return WEAPON_MODEL_CAMERA;

	case WEAPON_NIGHT_VIS_GOGGLES:
		return WEAPON_MODEL_NIGHTVISION;

	case WEAPON_THERMAL_GOGGLES:
		return WEAPON_MODEL_INFRARED;

	case WEAPON_PARACHUTE:
		return WEAPON_MODEL_PARACHUTE;

	}

	return -1;
}

//-----------------------------------------------------------

void __stdcall SetRadarColor(BYTE nIndex,DWORD dwColor)
{
	if(nIndex < sizeof(dwUseHudColors)) {
		dwUseHudColors[nIndex] = dwColor;
	}
}

//-----------------------------------------------------------

DWORD __stdcall TranslateColorCodeToRGBA(int iCode)
{
	// Special internal colors
	if(iCode == 200) return 0x888888FF;
	if(iCode == 201) return 0xAA0000FF;
	if(iCode == 202) return 0xE2C063FF;

	if(iCode < sizeof(dwUseHudColors)) {
		return dwUseHudColors[iCode];
	} else {
		return 0x999999FF;
	}
}

//----------------------------------------------------

int Game_PedStatPrim(int model_id)
{
	int *pStat;
	DWORD *d = (DWORD *)(0xA9B0C8 + (model_id*4));
	pStat = (int *)((*d) + 40);
	return *pStat;	
}

//----------------------------------------------------

int Game_PedStat(int model_id)
{
	int *pStat;
	DWORD *d = (DWORD *)(0xA9B0C8 + (model_id*4));
	pStat = (int *)((*d) + 44);
	return *pStat;
}

//-----------------------------------------------------------

void GameResetRadarColors()
{
	memcpy(&dwUseHudColors[0],&dwHudColors[0],sizeof(DWORD)*NUM_RADAR_COLORS);
}
//-----------------------------------------------------------

BOOL __stdcall GameIsEntityOnScreen(DWORD * pdwEnt)
{
	return TRUE;
}

//-----------------------------------------------------------

void __stdcall GamePrepareTrain(VEHICLE_TYPE *pVehicle)
{
	PED_TYPE *pDriver = pVehicle->pDriver;

	// GET RID OF THE PED DRIVER CREATED
	if(pDriver) {
		if (pDriver->dwPedType != 0 && pDriver->dwPedType != 1) {	// Make sure its not a CPlayerPed
			DWORD dwPedPtr = (DWORD)pDriver;
			_asm mov ecx, dwPedPtr
			_asm mov ebx, [ecx] ; vtable
			_asm push 1
			_asm call [ebx] ; destroy
			pVehicle->pDriver = 0;
		}
	}
}

//-----------------------------------------------------------

void __stdcall InitPlayerPedPtrRecords() 
{
	memset(&dwPlayerPedPtrs[0],0,sizeof(DWORD) * MAX_PLAYERS);
}

//-----------------------------------------------------------

void __stdcall SetPlayerPedPtrRecord(BYTE bytePlayer, DWORD dwPedPtr) 
{
	dwPlayerPedPtrs[bytePlayer] = dwPedPtr;
}

//-----------------------------------------------------------

BYTE __stdcall FindPlayerNumFromPedPtr(DWORD dwPedPtr)
{
	BYTE x = 0;
	while(x != MAX_PLAYERS)
	{
		if(dwPlayerPedPtrs[x] == dwPedPtr) return x;
		x++;
	}
	return 0;
}

//-----------------------------------------------------------

float FloatDifference(float f1, float f2)
{	
	return f1 - f2;
}

//-----------------------------------------------------------

float FloatOffset(float f1, float f2)
{	
	if(f1 >= f2) return f1 - f2;
	else return (f2 - f1);
}

//-----------------------------------------------------------

float DistanceBetweenHorizontalPoints(float x1, float y1, float x2, float y2)
{
	float fSX,fSY;

	fSX = (x1 - x2) * (x1 - x2);
	fSY = (y1 - y2) * (y1 - y2);
	
	return (float)sqrt(fSX + fSY);
}

//-----------------------------------------------------------

float DistanceBetweenPoints(float x1, float y1, float z1, float x2, float y2, float z2)
{
	float fSX,fSY,fSZ;

	fSX = (x1 - x2) * (x1 - x2);
	fSY = (y1 - y2) * (y1 - y2);
	fSZ = (z1 - z2) * (z1 - z2);
	
	return (float)sqrt(fSX + fSY + fSZ);
}

//----------------------------------------------------

void VectorNormalise(VECTOR *vec)
{
	_asm push vec
	_asm push vec
	_asm mov edx, 0x7ED9B0 ; RwV3dNormalize
	_asm call edx
	_asm add esp, 8
}

//----------------------------------------------------

float GetNormalisation(VECTOR *vec)
{
	return ((vec->X * vec->X) + (vec->Y * vec->Y) + (vec->Z * vec->Z));
}

//----------------------------------------------------

void CompressNormalVector(VECTOR * vec, C_VECTOR1 * c1)
{
	int X = (int)(vec->X * 8388607.5f);
	int Y = (int)(vec->Y * 8388607.5f);
	int Z = (int)(vec->Z * 8388607.5f);

	memcpy(c1->data+0, &X, 3);
	memcpy(c1->data+3, &Y, 3);
	memcpy(c1->data+6, &Z, 3);
}

//----------------------------------------------------

void DecompressNormalVector(VECTOR * vec, C_VECTOR1 * c1)
{
	int X = 0, Y = 0, Z = 0;

	memcpy(&X, c1->data+0, 3);
	if (c1->data[2+0] < 0) 
		X |= 0xFF000000;
	memcpy(&Y, c1->data+3, 3);
	if (c1->data[2+3] < 0) 
		Y |= 0xFF000000;
	memcpy(&Z, c1->data+6, 3);
	if (c1->data[2+6] < 0) 
		Z |= 0xFF000000;

	vec->X = ((float)X / 8388607.5f);
	vec->Y = ((float)Y / 8388607.5f);
	vec->Z = ((float)Z / 8388607.5f);
}

//----------------------------------------------------

void RwStateSetCall(DWORD opt, float * param)
{
	_asm mov edx, 0xC97B24
	_asm mov eax, [edx]
	_asm push param
	_asm push opt
	_asm call [eax+32]
	_asm pop eax
	_asm pop eax
}

//----------------------------------------------------

float DegToRad(float fDegrees)
{
	if (fDegrees > 360.0f || fDegrees < 0.0f) return 0.0f;
	if (fDegrees > 180.0f) {
		return (float)(-(PI - (((fDegrees - 180.0f) * PI) / 180.0f)));
	} else {
		return (float)((fDegrees * PI) / 180.0f);
	}
}

//----------------------------------------------------

bool IsNumeric(char * szString)
{
	while(*szString) {
		if(*szString < '0' || *szString > '9') {
			return false;
		}
		szString++;
	}
	return true;
}

//-----------------------------------------------------------
// Check if a specific model ID is valid

BOOL __stdcall IsValidModel(int iModelID)
{
	if (iModelID < 0 || iModelID > 20000) return FALSE;
	DWORD* dwModelArray = (DWORD*)0xA9B0C8;

	if (dwModelArray[ iModelID ] == 0)
		return FALSE;

	return TRUE;
}

//----------------------------------------------------

void _declspec(naked) _declspec(noreturn) GameForcedExitHelper()
{
	__asm
	{
		// [esp] = return
		// [esp+4] = pfFoward
		// [esp+8] = spBack;
		mov eax, 0xD0BC00B7;
		ror eax, 16;
		mov ebx, [esp+8];
		add eax, [esp+4];
		mov [esp], 0;
		mov [esp+4], 0;
		mov [esp+8], 0;
		add esp, ebx;			// corrupt the god damn stack!
		push eax;
		push 0;
		mov eax, 0x2A0D99E8;	// obfuscate and determine the FatalAppExitA call
		shl eax, 8;
		xor eax, 0x8F34C54A;
		shr eax, 8;
		push eax;
		ret;
	}

}

void _declspec(noreturn) GameForcedExit(int iReasonCode)
{
#ifdef _DEBUG
	OutputDebugString("GameForcedExit");
	ExitProcess(iReasonCode);
#endif

	// This function will forcefully exit the game with a obfuscated exit code, 
	// and pretty much corrupt the application's stack.

	srand((unsigned int)time(NULL));
	//int nonse = rand() ^ (rand() << 8);
	// xxxxxxxxxx0xxx0xxxxx0x0xx0xxx = 32 max
	// 11111111110111011111010110111 = 
	//                          1000 
	//                       1000000
	//                     100000000
	//               100000000000000
	//            100000000000000000

	/*nonse &= 0xFFFBBEB7;
	nonse |= ((iReasonCode & 0x1) << 3);
	nonse |= ((iReasonCode & 0x2) << 5);
	nonse |= ((iReasonCode & 0x4) << 6);
	nonse |= ((iReasonCode & 0x8) << 11);
	nonse |= ((iReasonCode & 0x16) << 14);*/

	// Go back a certain amount of sp
	int spBack = ((rand() ^ (rand() << 5)) >> 3) % 0x3F;
	if ((spBack % 4) != 0)
		spBack += 4-(spBack%4);

	// Go foward a certain amount from our scanlist data (+4) which is essentially replaced with PlayerInfos
	int pfForward = (rand() ^ (rand() << 7)) % 0xFFF;
	
	// Write the reason code as an octal number
	/*char *pOffs = (char*)(pfForward+0xB7D0BC);
	
	// "Internal check failed "
	memcpy(pOffs, (char*)0x86A5B0, 5);	pOffs+=5;
	memcpy(pOffs, (char*)0x8689B6, 3);	pOffs+=3;
	memcpy(pOffs, (char*)0x8749B6, 7); 	pOffs+=7;
	memcpy(pOffs, (char*)0x858AA5, 7); 	pOffs+=7;
	

	// "Internal error: "
	memcpy(pOffs, (char*)0x86A5BB, 5);	pOffs+=5;
	memcpy(pOffs, (char*)0x8689B6, 3);	pOffs+=3;
	memcpy(pOffs, (char*)0x86B97C, 8); 	pOffs+=8;

	itoa(nonse, (char*)(pOffs), 8);*/
	// Pass over the dirty work to GameForcedExitHelper
	__asm
	{
		push spBack;
		push pfForward;
		call GameForcedExitHelper;
	}

}

//----------------------------------------------------