//----------------------------------------------------------
//
// SA:MP Multiplayer Modification For GTA:SA
// Copyright 2004-2005 SA:MP team
//
// Version: $Id: util.h,v 1.15 2006/05/08 20:33:58 kyeman Exp $
//
//----------------------------------------------------------

#pragma once

#include "../main.h"

#define ENTITY_TYPE_UNKNOWN		0
#define ENTITY_TYPE_PED			1
#define ENTITY_TYPE_VEHICLE		2

void ProcessLineOfSight(VECTOR *vecOrigin, VECTOR *vecLine, VECTOR *colPoint,
		DWORD *pHitEntity, int bCheckBuildings, int bCheckVehicles, int bCheckPeds,
		int bCheckObjects, int bCheckDummies, int bSeeThroughStuff,
		int  bIgnoreSomeObjectsForCamera, int bShootThrough);

float GetNormalisation(VECTOR *vec);
void CompressNormalVector(VECTOR* vec, C_VECTOR1 * c1);
void DecompressNormalVector(VECTOR * vec, C_VECTOR1 * c1);

float FloatDifference(float f1, float f2);
float FloatOffset(float f1, float f2);

void __stdcall SetRadarColor(BYTE nIndex,DWORD dwColor);

void __stdcall WorldRemoveEntity(DWORD *dwEnt);
void __stdcall WorldAddEntity(DWORD *dwEnt);

int __stdcall GameGetWeaponModelIDFromWeaponID(int iWeaponID);
void __stdcall GameDisableCheatCodes();
PED_TYPE * __stdcall GamePool_Ped_GetAt(int iID);
int __stdcall GamePool_Ped_GetIndex(PED_TYPE *pActor);
VEHICLE_TYPE * __stdcall GamePool_Vehicle_GetAt(int iID);
ENTITY_TYPE * __stdcall GamePool_Object_GetAt(int iID);
DWORD __stdcall GamePool_Vehicle_GetIndex(VEHICLE_TYPE *pVehicle);
PED_TYPE * __stdcall GamePool_FindPlayerPed();
ENTITY_TYPE* __stdcall GamePool_Building_Get();
ENTITY_TYPE* __stdcall GamePool_Dummys_Get();
ENTITY_TYPE* __stdcall GamePool_Objects_Get();
int Game_PedStatPrim(int model_id);
int Game_PedStat(int model_id);

DWORD __stdcall TranslateColorCodeToRGBA(int iCode);
BOOL __stdcall GameIsEntityOnScreen(DWORD * pdwEnt);
void __stdcall InitPlayerPedPtrRecords();
void __stdcall SetPlayerPedPtrRecord(BYTE bytePlayer, DWORD dwPedPtr);
BYTE __stdcall FindPlayerNumFromPedPtr(DWORD dwPedPtr);
void __stdcall GamePrepareTrain(VEHICLE_TYPE *pVehicle);
float DistanceBetweenHorizontalPoints(float x1, float y1, float x2, float y2);
float DistanceBetweenPoints(float x1, float y1, float z1, float x2, float y2, float z2);

void GameResetRadarColors();

float DegToRad(float fDegrees);

bool IsNumeric(char *szString);

void GameForcedExit(int iReasonCode);

BOOL __stdcall IsValidModel(int iModelID);

#define FORCE_EXIT(a) GameForcedExit(a); while(true);

int GetTaskTypeFromTask(DWORD* task);
DWORD* GetNextTaskFromTask(DWORD * task);

unsigned int CRC32UppercaseString(char* szString);

void RemoveBuilding(int iModelId, float fX, float fY, float fZ, float fRadius);

void ConvertMatrixToQuat(MATRIX4X4* mat, D3DXQUATERNION * quat);
void QuatNormalize(D3DXQUATERNION* pQuat);

//-----------------------------------------------------------

