//----------------------------------------------------------
//
// SA:MP Multiplayer Modification For GTA:SA
// Copyright 2004-2005 SA:MP team
//
// Version: $Id: main.h,v 1.30 2006/05/08 14:31:50 kyeman Exp $
//
//----------------------------------------------------------

#pragma once

#include <stdio.h>
#include <windows.h>
#include <process.h>

#define IDC_CMDEDIT		1
#define IDC_CHATBACK	2
#define IDC_CHATSCROLL  3

#define MAX_SETTINGS_STRING		100

#define GTASA_VERSION_UNKNOWN	0
#define GTASA_VERSION_USA10		1
#define GTASA_VERSION_EU10		2
#define LOCKING_DISTANCE		200.0f
#define CSCANNER_DISTANCE		200.0f
#define PSCANNER_DISTANCE		600.0f

// Shared stuffs
#include "shared.h"

// BASS
#include "bass/bass.h"

typedef struct _GAME_SETTINGS {
	BOOL bDebug;
	BOOL bPlayOnline;
	BOOL bWindowedMode;
	CHAR szConnectPass[MAX_SETTINGS_STRING];
	CHAR szConnectHost[MAX_SETTINGS_STRING];
	CHAR szConnectPort[6];
	CHAR szNickName[MAX_PLAYER_NAME];
	CHAR szDebugScript[MAX_PATH];
} GAME_SETTINGS;

#include <d3d9.h>
#include <d3dx9core.h>
#include <dxut/dxstdafx.h>
#include "game/game.h"

#include <raknet/RakClientInterface.h>
#include <raknet/BitStream.h>
#include <raknet/PacketEnumerations.h>
#include <raknet/SAMPRPC.h>

#include "net/localplayer.h"
#include "net/remoteplayer.h"
#include "net/netrpc.h"
#include "net/playerpool.h"
#include "net/vehiclepool.h"
#include "net/pickuppool.h"
#include "net/objectpool.h"
#include "net/gangzonepool.h"
#include "net/menupool.h"
#include "net/textdrawpool.h"
#include "net/netgame.h"
#include "net/scriptrpc.h"

#include "runutil.h"
#include "filehooks.h"
#include "fontrender.h"
#include "chatwindow.h"
#include "cmdwindow.h"
#include "deathwindow.h"
#include "spawnscreen.h"
#include "playertags.h"
//#include "newplayertags.h"
#include "scoreboard.h"
#include "label.h"
#include "netstats.h"
//#include "svrnetstats.h"
//#include "helpdialog.h"
#include "archive/ArchiveFS.h"
#include "game/scripting.h"
#include "d3dhook.h"
#include "cursor.h"
#include "audiostream.h"

void QuitGame();

//void UnFuck(DWORD addr, int size);

extern CGame* pGame;
extern CChatWindow* pChatWindow;
extern CDeathWindow* pDeathWindow;
extern CNetGame* pNetGame;
extern CCmdWindow* pCmdWindow;
extern CSpawnScreen* pSpawnScreen;
extern CFontRender* pDefaultFont;
extern CPlayerTags* pPlayerTags;
extern CScoreBoard* pScoreBoard;
extern CLabel* pLabel;
extern CNetStats* pNetStats;
//extern CSvrNetStats* pSvrNetStats;
//extern CHelpDialog* pHelpDialog;
extern CFileSystem* pFileSystem;
extern CCursor* pCursor;
extern CAudioStream* pAudioStream;

extern CDXUTDialogResourceManager* pDialogResourceManager;
extern CDXUTDialog* pGameUI;

extern GAME_SETTINGS tSettings;
extern IDirect3DDevice9* pD3DDevice;
extern bool bShowDebugLabels;
extern bool bWantHudScaling;
extern bool bHeadMove;
extern int iGtaVersion;

#include "outputdebugstring.h"

//----------------------------------------------------
// EOF

