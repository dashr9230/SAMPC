//----------------------------------------------------------
//
// SA:MP Multiplayer Modification For GTA:SA
// Copyright 2004-2005 SA:MP team
//
// Version: $Id: playerped.h,v 1.28 2006/05/07 15:38:35 kyeman Exp $
//
//----------------------------------------------------------

#pragma once

#include "game.h"
#include "aimstuff.h"
#include "entity.h"

#define FALL_SKYDIVE		1
#define FALL_SKYDIVE_ACCEL	2
#define PARA_DECEL			3
#define PARA_FLOAT			4

//-----------------------------------------------------------

class CPlayerPed : public CEntity
{
public:

	void ResetForRespawn();

	void ResetPointers();
	void SetInitialState();

	void  SetKeys(UINT uiKeys, WORD lrAnalog, WORD udAnalog);
	UINT  GetKeys(WORD* lrAnalog, WORD* udAnalog);

	CAMERA_AIM * GetCurrentAim();
	void SetCurrentAim(CAMERA_AIM *pAim);

	BYTE GetCameraMode() {
		return GameGetLocalPlayerCameraMode();
	};

	void SetCameraMode(BYTE byteCamMode) {
		GameSetPlayerCameraMode(byteCamMode,m_bytePlayerNumber);
	};

	float GetCameraExtendedZoom() {
		return GameGetLocalPlayerCameraExtZoom();
	};

	void SetCameraExtendedZoom(float fZoom) {
		GameSetPlayerCameraExtZoom(m_bytePlayerNumber,fZoom);
	};

	void  Destroy();
	//void  ShowMarker(int iMarkerColor);
	void  ShowMarker();
	//void  ShowMarkerEx();
	void  HideMarker();
	BYTE  GetCurrentWeapon();
	int   GetCurrentVehicleID();
	bool  IsOnScreen();
	float GetHealth();
	void  SetHealth(float fHealth);
	float GetArmour();
	void  SetArmour(float fArmour);
	DWORD GetStateFlags();
	void  SetStateFlags(DWORD dwStateFlags);
	bool  IsDead();
	bool  IsInVehicle();
	BYTE  GetActionTrigger();
	void  SetActionTrigger(BYTE byteTrigger);
	WORD  GetAmmo();
	void  SetAmmo(BYTE byteWeapon, WORD wordAmmo);

	float GetTargetRotation();
	void  SetTargetRotation(float fRotation);
	void  ForceTargetRotation(float fRotation);

	void GiveWeapon(int iWeaponID, int iAmmo);
	void ClearAllWeapons();
	void SetArmedWeapon(int iWeaponType);
	WEAPON_SLOT_TYPE * GetCurrentWeaponSlot();
	WEAPON_SLOT_TYPE * FindWeaponSlot(DWORD dwWeapon);
	bool HasAmmoForCurrentWeapon();
	void SetWeaponModelIndex(int iWeapon);

	void SetImmunities(int bBullet, int bFire, int bExplosion, int bCollision, int bMelee);
	
	void PutDirectlyInVehicle(int iVehicleID, int iSeat);
	void EnterVehicle(int iVehicleID, bool bPassenger);
	void ExitCurrentVehicle();
	void RemoveFromVehicleAndPutAt(float fX, float fY, float fZ);

	bool IsAPassenger();

	VEHICLE_TYPE * GetGtaVehicle();
	PED_TYPE * GetGtaActor() { return m_pPed; };

	VEHICLE_TYPE * GetGtaContactVehicle();
	ENTITY_TYPE  * GetGtaContactEntity();

	float GetDistanceFromVehicle(CVehicle* pVehicle);

	int GetVehicleSeatID();
	void TogglePlayerControllable(int iControllable);
	BYTE FindDeathReasonAndResponsiblePlayer(BYTE * nPlayer);
	void RestartIfWastedAt(VECTOR *vecRestart, float fRotation);

	void StartJetpack();
	void StopJetpack();
	bool IsInJetpackMode();

	bool StartPassengerDriveByMode();
	bool PerformingDriveByFreeAimTask();
	void DestroyDriveByFreeAimTask();

	void SetModelIndex(UINT uiModel);
	void SetDead();
	void ExtinguishFire();
	void SetAnimationSet(PCHAR szAnim);
	void SetMoney(int iAmount);
	void ApplyAnimation(char *szAnimName, char *szAnimFile, float fT,
						int opt1, int opt2, int opt3, int opt4, int iUnk);

	void SetInterior(BYTE byteID);
	bool IsOnGround();
	void ResetDamageEntity();
	bool IsPerformingAnimation(char *szAnimName);

	// Constructor/Destructor.	
	CPlayerPed();
	CPlayerPed(int iPlayerNumber, int iSkin, float fPosX, float fPosY, float fPosZ, float fRotation = 0.0f, BYTE byteCreateMarker = 1);
	virtual ~CPlayerPed();

	void		ProcessVehicleHorn();

	bool		IsPerformingCustomAnim();
	void		ProcessParachutes();
	void		ProcessParachuteSkydiving();
	void		ProcessParachuting();
	void		CheckVehicleParachute();
	int			GetPedStat();

	int			m_iDanceState;
	int			m_iDanceStyle;
	int			m_iLastDanceMove;
	void		StartDancing(int iStyle);
	void		StopDancing();
	bool		IsDancing();
	void		ProcessDancing();
	char		*GetDanceAnimForMove(int iMove);
	void		HandsUp();
	bool		HasHandsUp();
	bool		IsJumpTask();
	bool		IsTakeDamageFallTask();
	bool		IsSwimTask();
	void		HoldItem(int iObject);

	void		StartPissing();
    void		StopPissing();
	int			IsPissing();

	void 		StartBeer();
	void        StopBeer();
	int			IsBeer();

	void 		StopSpecialAction();

	void		ProcessMarkers(bool bMarkerStreamingEnabled, float fMarkerStreamRadius, bool bVisible);

	void		ApplyCommandTask(char *szTaskName, int p1, int p2, int p3, 
								VECTOR *p4, int p5, float p6, int p7, int p8, int p9);

	void		DestroyFollowPedTask();
	void		ToggleCellphone(int iOn);
    int			IsCellphoneEnabled();
	int			m_iCellPhoneEnabled;

	float		GetAimZ();
	void		SetAimZ(float fAimZ);

	void SetFightingStyle(unsigned char style, unsigned char move);
	void GetFightingStyle(unsigned char* style, unsigned char* move);

	bool IsDucking();
	unsigned char IsEnteringVehicle();
	bool IsExitingVehicle();

	void SetEntryExit(char* szSectionName);
	void LoadShoppingData(char* szSectionName);

	VECTOR* GetBonePosition(int iBoneID, VECTOR* pPos);

	PED_TYPE    *m_pPed;
	BYTE		m_bytePlayerNumber;
	DWORD		m_dwMarkerID;

	int			m_iParachuteState;
	DWORD		m_dwParachuteObject;
	int			m_iParachuteAnim;
	
	int			m_iDancingState;
	int			m_iDancingAnim;

	int			m_iPissingState;
	DWORD		m_dwPissParticlesHandle;

	int 		m_iBeerState;

	DWORD 		m_dwArrow;
	BYTE		m_byteCreateMarker;
};

//-----------------------------------------------------------
