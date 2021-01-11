//----------------------------------------------------------
//
// SA:MP Multiplayer Modification For GTA:SA
// Copyright 2004-2005 SA:MP team
//
// Version: $Id: common.h,v 1.22 2006/05/07 21:16:50 kyeman Exp $
//
//----------------------------------------------------------

#pragma once

#include <windows.h>
#include <assert.h>

#include "shared.h"
//#define  _ASSERT	assert

//-----------------------------------------------------------

#define PADDING(x,y) BYTE x[y]

//-----------------------------------------------------------

#define IN_VEHICLE(x) ((x->dwStateFlags & 256) >> 8)

//-----------------------------------------------------------

typedef struct _WEAPON_SLOT_TYPE
{
	DWORD dwType;
	DWORD dwState;
	DWORD dwAmmoInClip;
	DWORD dwAmmo;
	PADDING(_pwep1,12);
} WEAPON_SLOT_TYPE;  // MUST BE EXACTLY ALIGNED TO 28 bytes

//-----------------------------------------------------------

typedef struct _PED_TASKS_TYPE
{
	DWORD * pdwPed; // 0-4
	// Basic Tasks
	DWORD * pdwDamage; // 4-8
	DWORD * pdwFallEnterExit; // 8-12
	DWORD * pdwSwimWasted; // 12-16
	DWORD * pdwJumpJetPack; // 16-20
	DWORD * pdwAction; // 20-24
	// Extended Tasks
	DWORD * pdwFighting; // 24-28
	DWORD * pdwCrouching; // 28-32
	DWORD * pdwExtUnk1; // 32-36
	DWORD * pdwExtUnk2; // 36-40
	DWORD * pdwExtUnk3; // 40-44
	DWORD * pdwExtUnk4; // 44-48
} PED_TASKS_TYPE;

//-----------------------------------------------------------

typedef struct _ENTITY_TYPE
{
	// ENTITY STUFF
	DWORD vtable; // 0-4
	PADDING(_pad0,12); // 4-16
	FLOAT fRotZBeforeMat; // 16-20 (likely contains the rotation of the entity when mat==0);
	MATRIX4X4 *mat; // 20-24
	DWORD *pdwRenderWare; // 24-28
	DWORD dwProcessingFlags; // 28-32
	PADDING(_pad1,2); // 32-34
	WORD nModelIndex; // 34-36
	PADDING(_pad2,18); // 36-54
	BYTE nControlFlags; // 54-55
	PADDING(_pad3,11); // 55-66
	BYTE byteImmunities; // 66-67
	BYTE byteUnkEntFlags; // 67-68
	VECTOR vecMoveSpeed; // 68-80
	VECTOR vecTurnSpeed; // 80-92
	PADDING(_pad5,72); // 92-164
	BYTE byteAudio[5]; // 164-169
	PADDING(_pad5a,11); // 169-180
	DWORD dwUnkModelRel; // 180-184
} ENTITY_TYPE;

//-----------------------------------------------------------

typedef struct _PED_TYPE
{
	ENTITY_TYPE entity; // 0-184

	// CPED STUFF
	
	PADDING(_pad100,948); // 184-1132
	DWORD dwStateFlags; // 1132-1136
	DWORD dwInvulFlags; // 1136-1140		0x1000 = can_decap
	PADDING(_pad104,8); // 1140-1148
	PED_TASKS_TYPE *Tasks; // 1148-1152
	DWORD dwPlayerInfoOffset; // 1152-1156
	PADDING(_pad200,144); // 1156-1300
	float fAimZ; // 1300-1304
	PADDING(_pad201,16); // 1304-1320
	BYTE byteAimAnimState; // 1320-1321
	PADDING(_pad202,7); // 1321-1328
	DWORD dwAction; // 1328-1332
	PADDING(_pad203,12); // 1332-1344
	float fHealth;		 // 1344-1348
	float fMaxHealth;	// 1348-1352
	float fArmour;		// 1352-1356
	PADDING(_pad250,4); // 1356-1360
	float fMoveRot1;	// 1360
	float fMoveRot2;	// 1364
	float fRotation1;	// 1368-1372
	float fRotation2;	// 1372-1376
	float fUnkRotRel;
	float fRotCamAdjust;
	DWORD pContactVehicle; // 1384 - 1388
	PADDING(_pad292, 24);
	DWORD pContactEntity; // 1412 - 1416
	PADDING(_pad224, 4);
	DWORD pVehicle;	// 1420-1424
	PADDING(_pad261,8); // 1424-1432
	DWORD dwPedType; // 1432-1436
	DWORD dwUnk1;	 // 1436-1440
	WEAPON_SLOT_TYPE WeaponSlots[13]; // 1440-1804
	PADDING(_pad270,12); // 1804-1816
	BYTE byteCurWeaponSlot; // 1816-1817
	PADDING(_pad280,23); // 1817-1840
	DWORD pFireObject;	 // 1840-1844
	PADDING(_pad281,44); // 1844-1888
	DWORD  dwWeaponUsed; // 1888-1892
	PDWORD pdwDamageEntity; // 1892-1896
	PADDING(_pad290, 36); // 1896-1932
	DWORD* pEntryExit; // 1932-1936
	PADDING(_pad291, 12); // 1936-1948
	DWORD pTarget; // 1948
	
} PED_TYPE;

//-----------------------------------------------------------

#pragma pack(1)
typedef struct _VEHICLE_TYPE
{
	ENTITY_TYPE entity; // 0-184

	// CVEHICLE STUFF
	PADDING(_pad200,318); // 184-502
    BYTE byteHorn;		  // 502-503
	PADDING(_pad2001,185); // 503-688
	int iHornLevel;			// 688-692
	int iSirenLevel;	   // 692-696
	PADDING(_pad2002,196); // 696-892
	BYTE bNitroOn;        // 892-893
	PADDING(_pad201,171); // 893-1064
	BYTE byteFlags;		  // 1064-1065
	PADDING(_pad210,4);  // 1065-1069
	BYTE _pad211  : 7;   // 1069-1070 (bits 0..6)
	BYTE bSirenOn : 1;   // 1069-1070 (bit 7)
	PADDING(_pad212,6);  // 1070-1076
	BYTE byteColor1;      // 1076-1077
	BYTE byteColor2;      // 1077-1078
	BYTE byteColor3;	// 1078-1079
	BYTE byteColor4;	// 1079-1080
	PADDING(_pad1080_to_1082, 2); // 1080-1082
	unsigned short sComponent[15]; // 1082-1112
	PADDING(_pad1112_to_1120, 8); // 1112-1120
	PED_TYPE * pDriver;   // 1120-1124
	PED_TYPE * pPassengers[7]; // 1124-1152

	PADDING(_pad235,8); // 1152-1160
	BYTE byteNumOfSeats; // 1160-1161
	PADDING(_pad236, 7); // 1161-1168

	DWORD pFireObject;	 // 1168-1172

	float fSteerAngle1; // 1172-1176
	float fSteerAngle2; // 1176-1180
	float fAcceleratorPedal; // 1180-1184
	float fBrakePedal; // 1184-1188
	
	PADDING(_pad275,28); // 1188-1216

	float fHealth;			// 1216-1220
	PADDING(_pad240,4);		// 1220-1224
	DWORD dwTrailer;		// 1224-1228
	PADDING(_pad241,44);	// 1228-1272
	DWORD dwDoorsLocked;	// 1272-1276
	PADDING(_pad2423,24);	// 1276-1300
	BYTE byteHorn2;			// 1300-1301
	PADDING(_pad2424,143);	// 1301-1444
	union {
		struct {
			PADDING(_pad2421,1);     // 1444-1445
			BYTE bCarWheelPopped[4]; // 1445-1449
		};
		struct {
			float fTrainSpeed;   // 1444-1448
			PADDING(_pad2422,1); // 1448-1449
		};
	};
	DWORD dwDoorsDamageStatus; // 1449-1453
	PADDING(_pad1453, 3); // 1453-1456
	DWORD dwLightsDamageStatus; // 1456-1460
	DWORD dwPanelsDamageStatus; // 1460-1464
	PADDING(_pad1464, 164);  // 1464-1628
	BYTE bBikeWheelPopped[2]; // 1628-1630
	PADDING(_pad244, 526);  // 1630-2156
	DWORD dwHydraThrusters; // 2156-2160
	PADDING(_pad245, 220);  // 2160-2380
	float fTankRotX;		// 2380-2384
	float fTankRotY;		// 2384-2388
	PADDING(_pad246, 120);  // 2388-2508
	float fPlaneLandingGear;// 2508-2512
	PADDING(_pad247, 1517); // 2512-4029
} VEHICLE_TYPE;
#pragma pack()

//-----------------------------------------------------------

typedef struct {
	PADDING(pad_0, 3310); // 0-3310
	BYTE bLockPosition; // 3310-3311
	BYTE bLockTargetPoint; // 3311-3312
	PADDING(pad_3312, 136); // 3312-3448
} CAMERA_TYPE;


//-----------------------------------------------------------

#define FADE_OUT						0
#define FADE_IN							1

//-----------------------------------------------------------

#define	VEHICLE_SUBTYPE_CAR				1
#define	VEHICLE_SUBTYPE_BIKE			2
#define	VEHICLE_SUBTYPE_HELI			3
#define	VEHICLE_SUBTYPE_BOAT			4
#define	VEHICLE_SUBTYPE_PLANE			5
#define	VEHICLE_SUBTYPE_PUSHBIKE		6
#define	VEHICLE_SUBTYPE_TRAIN			7

//-----------------------------------------------------------

#define ACTION_WASTED					55
#define ACTION_DEATH					54
#define ACTION_INCAR					50
#define ACTION_NORMAL					1
#define ACTION_SCOPE					12
#define ACTION_NONE						0 

//-----------------------------------------------------------

//---- weapon model defines ----
#define WEAPON_MODEL_BRASSKNUCKLE		331 // was 332
#define WEAPON_MODEL_GOLFCLUB			333
#define WEAPON_MODEL_NITESTICK			334
#define WEAPON_MODEL_KNIFE				335
#define WEAPON_MODEL_BAT				336
#define WEAPON_MODEL_SHOVEL				337
#define WEAPON_MODEL_POOLSTICK			338
#define WEAPON_MODEL_KATANA				339
#define WEAPON_MODEL_CHAINSAW			341
#define WEAPON_MODEL_DILDO				321
#define WEAPON_MODEL_DILDO2				322
#define WEAPON_MODEL_VIBRATOR			323
#define WEAPON_MODEL_VIBRATOR2			324
#define WEAPON_MODEL_FLOWER				325
#define WEAPON_MODEL_CANE				326
#define WEAPON_MODEL_GRENADE			342 // was 327
#define WEAPON_MODEL_TEARGAS			343 // was 328
#define WEAPON_MODEL_MOLTOV				344 // was 329
#define WEAPON_MODEL_COLT45				346
#define WEAPON_MODEL_SILENCED			347
#define WEAPON_MODEL_DEAGLE				348
#define WEAPON_MODEL_SHOTGUN			349
#define WEAPON_MODEL_SAWEDOFF			350
#define WEAPON_MODEL_SHOTGSPA			351
#define WEAPON_MODEL_UZI				352
#define WEAPON_MODEL_MP5				353
#define WEAPON_MODEL_AK47				355
#define WEAPON_MODEL_M4					356
#define WEAPON_MODEL_TEC9				372
#define WEAPON_MODEL_RIFLE				357
#define WEAPON_MODEL_SNIPER				358
#define WEAPON_MODEL_ROCKETLAUNCHER		359
#define WEAPON_MODEL_HEATSEEKER			360
#define WEAPON_MODEL_FLAMETHROWER		361
#define WEAPON_MODEL_MINIGUN			362
#define WEAPON_MODEL_SATCHEL			363
#define WEAPON_MODEL_BOMB				364
#define WEAPON_MODEL_SPRAYCAN			365
#define WEAPON_MODEL_FIREEXTINGUISHER	366
#define WEAPON_MODEL_CAMERA				367
#define WEAPON_MODEL_NIGHTVISION		368	// newly added
#define WEAPON_MODEL_INFRARED			369	// newly added
#define WEAPON_MODEL_JETPACK			370	// newly added
#define WEAPON_MODEL_PARACHUTE			371

#define OBJECT_PARACHUTE				3131
