//----------------------------------------------------------
//
// SA:MP Multiplayer Modification For GTA:SA
// Copyright 2004-2005 SA:MP team
//
// Version: $Id: main.cpp,v 1.60 2006/05/21 11:20:49 kyeman Exp $
//
//----------------------------------------------------------

#include "main.h"
#include "game/util.h"
//#include "anticheat.h"
//#include <aclapi.h>

int						iGtaVersion=0;

GAME_SETTINGS			tSettings;
CChatWindow				*pChatWindow=0;
CCmdWindow				*pCmdWindow=0;
CDeathWindow			*pDeathWindow=0;
CSpawnScreen			*pSpawnScreen=0;
CNetGame				*pNetGame=0;
CFontRender				*pDefaultFont=0;
CCursor					*pCursor = NULL;

static bool				bGameInited=false;
static bool				bNetworkInited=false;
static bool				bGameModded = false;
static bool				bQuitGame=false;
static DWORD			dwStartQuitTick=0;

//IDirect3D9				*pD3D;
IDirect3DDevice9		*pD3DDevice	= NULL;
//D3DMATRIX				matView;

HINSTANCE				hInstance=0;
CPlayerTags				*pPlayerTags=NULL;
CScoreBoard				*pScoreBoard=NULL;
CLabel					*pLabel=NULL;
CNetStats				*pNetStats=NULL;
//CSvrNetStats			*pSvrNetStats=NULL;
//CHelpDialog				*pHelpDialog=NULL;
CAudioStream			*pAudioStream=NULL;
CConfigFile				*pConfigFile=NULL;
CChatBubble				*pChatBubble=NULL;
CTextDrawSelect         *pTextDrawSelect = NULL;

bool					bShowDebugLabels = false;
bool					bWantHudScaling = true;
bool 					bHeadMove = true;

CGame					*pGame=0;
//DWORD					dwGameLoop=0;
DWORD					dwGraphicsLoop=0;
//DWORD					dwUIMode=0;				// 0 = old mode, 1 = new MMOG mode, 2 = DXUT perhaps?
												// Have this settable from the server.. on Init.
CFileSystem				*pFileSystem=NULL;
//CAntiCheat				*pAntiCheat = NULL;

CDXUTDialogResourceManager	*pDialogResourceManager=NULL;
CDXUTDialog					*pGameUI=NULL;

char szUserDocPath[MAX_PATH];
char szCachePath[MAX_PATH];

// forwards
bool SubclassGameWindow();
void SetupCommands();
//void TheGameLoop();
void TheGraphicsLoop();
void MarkAsModdedGame();
void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext );
LONG WINAPI exc_handler(_EXCEPTION_POINTERS* exc_inf);
void InitSettings();

//UINT uiCounter=0;

static DWORD dwOrgRwSetState=0;
static DWORD dwSetStateCaller=0;
static DWORD dwSetStateOption=0;
static DWORD dwSetStateParam=0;
#ifdef _DEBUG
static char dbgstr[40];
#endif

// polls the game until it's able to run.
void LaunchMonitor(PVOID v)
{
	pGame = new CGame();
	pGame->InitGame();

	while(1) {
		if(*(PDWORD)ADDR_ENTRY == 7) {
			pGame->StartGame();
            break;
		}
		else {
			Sleep(5);
		}
	}

	ExitThread(0);
}

//----------------------------------------------------

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	if(DLL_PROCESS_ATTACH==fdwReason)
	{
		OutputDebugString("SA-MP Process Attached\n");

		hInstance = hinstDLL;
		InitSettings();

		if(tSettings.bDebug || tSettings.bPlayOnline)
		{
			DisableThreadLibraryCalls(hinstDLL);
			SetUnhandledExceptionFilter(exc_handler);
			//dwGameLoop = (DWORD)TheGameLoop;
			dwGraphicsLoop = (DWORD)TheGraphicsLoop;

			OutputDebugString("Loading Archive");

			CHAR szArchiveFile[MAX_PATH];
			GetModuleFileNameA((HMODULE)hInstance, szArchiveFile, MAX_PATH);
			DWORD dwFileNameLen = strlen(szArchiveFile);
			while (szArchiveFile[dwFileNameLen] != '\\')
				szArchiveFile[dwFileNameLen--] = '\0';
			strcat_s(szArchiveFile, "samp.saa");

			pFileSystem = new CArchiveFS();
			if(!pFileSystem->Load(szArchiveFile)) _asm int 3

			OutputDebugString("Installing filesystem hooks.");

			InstallFileSystemHooks();

			_beginthread(LaunchMonitor,0,NULL);	
			OutputDebugString("SA:MP Inited\n");			
		}
	}
	else if(DLL_PROCESS_DETACH==fdwReason)
	{
		if(tSettings.bDebug || tSettings.bPlayOnline) {
			OutputDebugString("Process Detached\n");
			/*if (pAntiCheat)	{
				pAntiCheat->Disable();
				delete pAntiCheat;
			}*/
			UninstallFileSystemHooks();

			Discord_Shutdown();
		}
	}

	return TRUE;
}

//----------------------------------------------------

DWORD dwFogEnabled = 0;
static DWORD dwFogColor = 0x00FF00FF;
static bool gDisableAllFog = false;

void SetupD3DFog(BOOL bEnable)
{
	float fFogStart = 500.0f;
	float fFogEnd = 700.0f;

	if(gDisableAllFog) bEnable = FALSE;

	if(pD3DDevice) {
		pD3DDevice->SetRenderState(D3DRS_FOGENABLE, bEnable);
		//pD3DDevice->SetRenderState(D3DRS_FOGCOLOR, dwFogColor);
		pD3DDevice->SetRenderState(D3DRS_FOGTABLEMODE, D3DFOG_NONE);
		pD3DDevice->SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_LINEAR);
		//pD3DDevice->SetRenderState(D3DRS_FOGSTART, *(DWORD*)(&fFogStart));
		//pD3DDevice->SetRenderState(D3DRS_FOGEND, *(DWORD*)(&fFogEnd));
	}
}

//----------------------------------------------------

void _declspec(naked) RwRenderStateSetHook()
{
	_asm mov eax, [esp]
	_asm mov dwSetStateCaller, eax
	_asm mov eax, [esp+4]
	_asm mov dwSetStateOption, eax
	_asm mov eax, [esp+8]
	_asm mov dwSetStateParam, eax

	if(dwSetStateOption == 14) {
		if(dwSetStateParam) {
			SetupD3DFog(TRUE);
			dwFogEnabled = 1;
		} else {
			SetupD3DFog(FALSE);
			dwFogEnabled = 0;
		}
		_asm mov [esp+8], 0 ; no fog
	}

	_asm mov eax, dwOrgRwSetState
	_asm jmp eax
}

//----------------------------------------------------

void HookRwRenderStateSet()
{
	DWORD dwNewRwSetState = (DWORD)RwRenderStateSetHook;

	_asm mov ebx, 0xC97B24
	_asm mov eax, [ebx]
	_asm mov edx, [eax+32]
	_asm mov dwOrgRwSetState, edx
	_asm mov edx, dwNewRwSetState
	_asm mov [eax+32], edx

#ifdef _DEBUG
	sprintf_s(dbgstr,"HookRwRenderStateSet(0x%X)",dwOrgRwSetState);
	OutputDebugString(dbgstr);
#endif

}

//----------------------------------------------------

/*void CallRwRenderStateSet(int state, int option)
{
	_asm push option
	_asm push state
	_asm mov ebx, 0xC97B24
	_asm mov eax, [ebx]
	_asm call dword ptr [eax+32]
	_asm add esp, 8
}*/

//----------------------------------------------------

void SetupGameUI()
{
	if(pGameUI) SAFE_DELETE(pGameUI);

	pGameUI = new CDXUTDialog();
    pGameUI->Init(pDialogResourceManager);
	pGameUI->SetCallback( OnGUIEvent );
	pGameUI->SetLocation(0,0);
	pGameUI->SetSize(pGame->GetScreenWidth(),pGame->GetScreenHeight());

	//pGameUI->AddButton(2,"Test Button",30,390,80,35,0);
	//pGameUI->AddEditBox(3,"",20,pGame->GetScreenHeight()-40,pGame->GetScreenWidth()-40,36,true);

	pGameUI->EnableMouseInput(false);
	pGameUI->EnableKeyboardInput(true);

	if(pChatWindow) pChatWindow->ResetDialogControls(pGameUI);
	if(pCmdWindow) pCmdWindow->ResetDialogControls(pGameUI);

}		

//----------------------------------------------------

void SetupCacheFolders()
{
	HKEY hKey;
	CHAR szPath[MAX_PATH];
	DWORD dwLength;

	SecureZeroMemory(szCachePath, sizeof(szCachePath));
	SecureZeroMemory(szPath, sizeof(szPath));

	dwLength = sizeof(szPath);

	sprintf_s(szCachePath, "%s\\cache", szUserDocPath);
	if(!RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\SAMP", 0, KEY_READ, &hKey) &&
		!RegQueryValueEx(hKey, "model_cache", NULL, NULL, (LPBYTE)szPath, &dwLength))
	{
		strncpy_s(szCachePath, szPath, sizeof(szCachePath));
		RegCloseKey(hKey);
	}

	if (!DirExists(szCachePath))
		CreateDirectory(szCachePath, NULL);

	sprintf_s(szPath, "%s\\local", szCachePath);
	if (!DirExists(szPath))
		CreateDirectory(szPath, NULL);
}

void SetupModUserFilesDirs()
{
	char szPathName[MAX_PATH];
	char* szUserDir;

	SecureZeroMemory(szUserDocPath, sizeof(szUserDocPath));

	szUserDir = (char*)0xC92368;
	while (*szUserDir)
		szUserDir++;

	if (szUserDir == (char*)0xC92369)
	{
		GetCurrentDirectory(sizeof(szUserDocPath), szUserDocPath);
	}
	else
	{
		sprintf_s(szUserDocPath, "%s\\SAMP", (char*)0xC92368);
		if (!DirExists(szUserDocPath))
			CreateDirectory(szUserDocPath, NULL);

		sprintf_s(szPathName, "%s\\screens", szUserDocPath);
		if (!DirExists(szPathName))
			CreateDirectory(szPathName, NULL);

		SetupCacheFolders();
	}
}

//----------------------------------------------------

//extern void CheckDuplicateD3D9Dlls();

// TODO: Add "nohudscalefix", "pagesize", "timestamp", "fpslimit", "multicore", "disableheadmove" config checks here
void DoInitStuff()
{
	// GAME INIT
	if(!bGameInited)
	{	
		OutputDebugString("Start of DoInitStuff()");

		SetupModUserFilesDirs();

		pConfigFile = new CConfigFile(szUserDocPath);

		Util_GetTime();

		timeBeginPeriod(5); // increases the accuracy of Sleep()
		SubclassGameWindow();

		//CheckDuplicateD3D9Dlls(); // Do this before any hooks are installed.

		// Grab the real IDirect3D9 * from the game.
		//pD3D = (IDirect3D9 *)pGame->GetD3D();

		/*if(!pD3D) {
			OutputDebugString("No D3D!!!");
			return;
		}*/

		OutputDebugString("Initializing Discord presence RPC");
		Discord_Initialize(APPLICATION_ID, NULL, 1, NULL);

		// Grab the real IDirect3DDevice9 * from the game.
		pD3DDevice = (IDirect3DDevice9 *)pGame->GetD3DDevice();

		if(!pD3DDevice) { // if(pD3D) ?!
			OutputDebugString("No D3DDevice!!!");
			return;
		}

		*(IDirect3DDevice9Hook**)ADDR_ID3D9DEVICE = new IDirect3DDevice9Hook();

		OutputDebugString("Font and chat window creating..");

		// Create instances of the chat and input classes.
		pDefaultFont = new CFontRender(pD3DDevice);
		pChatWindow = new CChatWindow(pD3DDevice,pDefaultFont, szUserDocPath);
		pCmdWindow = new CCmdWindow(pD3DDevice);

		AllocateBufferForColorEmbed();

		if (pConfigFile->GetInt("nohudscalefix") == 1)
			bWantHudScaling = false;

		// DXUT GUI INITIALISATION
		OutputDebugString("DXUTGUI creating..");

		pDialogResourceManager = new CDXUTDialogResourceManager();
		pDialogResourceManager->OnCreateDevice(pD3DDevice);
		pDialogResourceManager->OnResetDevice();
		
		OutputDebugString("SetupGameUI() creating..");

		SetupGameUI();

		pCursor = new CCursor;
		pCursor->Init();

		if(tSettings.bPlayOnline) {
			pDeathWindow = new CDeathWindow(pD3DDevice);
			pSpawnScreen = new CSpawnScreen;
			//pPlayerTags = new CPlayerTags(pD3DDevice);
			pPlayerTags = new CPlayerTags(pD3DDevice);
			pScoreBoard = new CScoreBoard(pD3DDevice, FALSE);
			pNetStats = new CNetStats(pD3DDevice);
			//pSvrNetStats = new CSvrNetStats(pD3DDevice);
			//pHelpDialog = new CHelpDialog(pD3DDevice);
			pAudioStream = new CAudioStream;
			pChatBubble = new CChatBubble();
			pTextDrawSelect = new CTextDrawSelect();

			pDeathWindow->CreateFonts();
		}
		
		OutputDebugString("Labels creating..");
		pLabel = new CLabel(pD3DDevice);

		// Setting up the commands.
		OutputDebugString("Setting up commands..");
		SetupCommands();

		OutputDebugString("Hooking RwSetState..");
		HookRwRenderStateSet();

		if(tSettings.bDebug) {
			CCamera *pGameCamera = pGame->GetCamera();
			pGameCamera->Restore();
			pGameCamera->SetBehindPlayer();
			pGame->DisplayHud(TRUE);
			pGame->EnableClock(0);
			if (tSettings.szDebugScript[0] != '\0')
				GameDebugLoadScript(tSettings.szDebugScript);
			UpdateDiscordPresence("In debug mode", NULL);
		}

		if (!pConfigFile->IsValidKey("multicore"))
			pConfigFile->SetInt("multicore", 1, false);
		if (!pConfigFile->GetInt("multicore"))
			SetProcessAffinityMask(GetCurrentProcess(), 1);

		bGameInited = true;
		OutputDebugString("End of DoInitStuff()");

		return;		
	}

	// NET GAME INIT
	if(!bNetworkInited && tSettings.bPlayOnline) {

		pNetGame = new CNetGame(tSettings.szConnectHost,atoi(tSettings.szConnectPort),
				tSettings.szNickName,tSettings.szConnectPass);

		bNetworkInited = true;
		return;
	}
}

//----------------------------------------------------

void MarkAsModdedGame()
{
	bGameModded = true;
}

//----------------------------------------------------

void TheGraphicsLoop()
{	
	_asm pushad // because we're called from a hook

	DoInitStuff();
	
	SetupD3DFog(TRUE);

	// Process the netgame if it's active.
	if(pNetGame) {
		//Sleep(0); // This hands the context over to raknet
		pNetGame->Process();
	}

	if(bQuitGame) {
		if((GetTickCount() - dwStartQuitTick) > 1000) {
			if (pNetGame)
			{
				delete pNetGame;
				pNetGame = NULL;
			}
			ExitProcess(0);
		}
		_asm popad
		return;
	}

	//pGame->UpdateFarClippingPlane();
	pGame->ProcessInputDisabling();
	if (bGameModded == true)
	{
		FORCE_EXIT(0x4);
	}

	// We have to call the real Render2DStuff
	// because we overwrote its call to get here.
	_asm popad
	_asm mov edx, ADDR_RENDER2DSTUFF
	_asm call edx
}

//----------------------------------------------------

/*void TheGameLoop()
{	
	_asm pushad

	//SetCursor(LoadCursor(NULL,IDC_ARROW));
	//ShowCursor(TRUE);
	//OutputDebugString("---- New Frame ----");

	_asm popad
}*/

//----------------------------------------------------

void QuitGame()
{
	if(pNetGame && pNetGame->GetGameState() == GAMESTATE_CONNECTED) {
		pNetGame->GetRakClient()->Disconnect(500);
	}	
	bQuitGame = true;
	dwStartQuitTick = GetTickCount();
}

//----------------------------------------------------

void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext )
{
	switch( nControlID )
    {
        case IDC_CMDEDIT:
		{
			if(nEvent == EVENT_EDITBOX_STRING) {
				if(pCmdWindow) pCmdWindow->ProcessInput();
			}
			break;
		}
	}
	return;
}

void SetStringFromCommandLine(char* szCmdLine, char* szString, int iLen)
{
	int iCount = 0;
	while (*szCmdLine == ' ') szCmdLine++;
	if (*szCmdLine == '"')
	{
		szCmdLine++;
		while (*szCmdLine && *szCmdLine != '"' && iCount < iLen)
		{
			*szString = *szCmdLine;
			szString++; szCmdLine++;
			iCount++;
		}
		*szString = '\0';
		szCmdLine++;
	}
	else
	{
		while (*szCmdLine &&
			*szCmdLine != ' ' &&
			*szCmdLine != '-' &&
			*szCmdLine != '/' &&
			iCount < iLen)
		{
			*szString = *szCmdLine;
			szString++; szCmdLine++;
			iCount++;
		}
		*szString = '\0';
	}
}

void InitSettings()
{
	PCHAR szCmdLine = GetCommandLineA();

	OutputDebugString(szCmdLine);
	OutputDebugString("\n");

	memset(&tSettings,0,sizeof(GAME_SETTINGS));

	while(*szCmdLine) {

		if(*szCmdLine == '-' || *szCmdLine == '/') {
			szCmdLine++;
			switch(*szCmdLine) {
				case 'd':
					tSettings.bDebug = TRUE;
					tSettings.bPlayOnline = FALSE;
					break;
				case 'c':
					tSettings.bPlayOnline = TRUE;
					tSettings.bDebug = FALSE;
					break;
				case 'z':
					szCmdLine++;
					SetStringFromCommandLine(szCmdLine,tSettings.szConnectPass,sizeof(tSettings.szConnectPass));
					break;
				/*
				// We'll do this using ALT+ENTER
				case 'w':
					tSettings.bWindowedMode = TRUE;
					break;
				*/
				case 'h':
					szCmdLine++;
					SetStringFromCommandLine(szCmdLine,tSettings.szConnectHost,sizeof(tSettings.szConnectHost));
					break;
				case 'p':
					szCmdLine++;
					SetStringFromCommandLine(szCmdLine,tSettings.szConnectPort,sizeof(tSettings.szConnectPort));
					break;
				case 'n':
					szCmdLine++;
					SetStringFromCommandLine(szCmdLine,tSettings.szNickName,sizeof(tSettings.szNickName));
					break;
				case 'l':
					szCmdLine++;
					SetStringFromCommandLine(szCmdLine, tSettings.szDebugScript, sizeof(tSettings.szDebugScript));
					break;
			}
		}

		szCmdLine++;
	}
}

void d3d9DestroyDeviceObjects()
{
	if (pDialogResourceManager)
		pDialogResourceManager->OnLostDevice();

	if (pCursor)
		pCursor->DeleteDeviceObjects();

	if (pPlayerTags)
		pPlayerTags->DeleteDeviceObjects();

	if (pScoreBoard)
		pScoreBoard->DeleteDeviceObjects();

	if (pLabel)
		pLabel->DeleteDeviceObjects();

	if (pDefaultFont)
		pDefaultFont->DeleteDeviceObjects();

	if (pDeathWindow)
		pDeathWindow->OnLostDevice();

	if (pChatWindow && pChatWindow->m_pChatTextSprite) pChatWindow->m_pChatTextSprite->OnLostDevice();
}

void d3d9RestoreDeviceObjects()
{
	if (pDialogResourceManager)
		pDialogResourceManager->OnResetDevice();

	if (pCursor)
		pCursor->RestoreDeviceObjects();

	if (pPlayerTags)
		pPlayerTags->RestoreDeviceObjects();

	if (pScoreBoard)
		pScoreBoard->RestoreDeviceObjects();

	if (pLabel)
		pLabel->RestoreDeviceObjects();

	if (pDefaultFont)
		pDefaultFont->RestoreDeviceObjects();

	if (pDeathWindow)
		pDeathWindow->OnResetDevice();
	
	if (pChatWindow && pChatWindow->m_pChatTextSprite) pChatWindow->m_pChatTextSprite->OnResetDevice();
}

void UpdateDiscordPresence(char* state, char* details)
{
	Discord_ClearPresence();

	DiscordRichPresence discordPresence;
	memset(&discordPresence, 0, sizeof(discordPresence));

	discordPresence.state = state;
	discordPresence.details = details;
	discordPresence.largeImageKey = "default";
	//discordPresence.smallImageKey = "default";
	discordPresence.instance = 0;

	Discord_UpdatePresence(&discordPresence);
}

//----------------------------------------------------