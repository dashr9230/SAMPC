//----------------------------------------------------------
//
// SA:MP Multiplayer Modification For GTA:SA
// Copyright 2004-2005 SA:MP team
//
// Version: $Id: subclass.cpp,v 1.18 2006/05/08 13:28:46 kyeman Exp $
//
//----------------------------------------------------------

#include "main.h"
#include <string>

BOOL g_bTakeScreenshot = FALSE;
static WNDPROC hOldProc;
LRESULT APIENTRY NewWndProc(HWND,UINT,WPARAM,LPARAM);

void cmdRawSave(PCHAR szCmd);

//----------------------------------------------------

static DWORD dwWorkingSkins[] =
{ 
  0, 1, 2,
  47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
  64, 66, 67, 68, 69, 70, 71, 72, 73, 75, 76, 77, 78, 79, 80, 81, 82, 83,
  84, 85, 87, 88, 89, 91, 92, 93, 95, 96, 97, 98, 99, 100, 101, 102,
  103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116,
  117, 118, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 131, 133,
  134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147,
  148, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162,
  163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176,
  177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190,
  191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204,
  205, 206, 207, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219,
  220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233,
  234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247,
  248, 249, 250, 251, 253, 254, 255, 256, 257, 258, 259, 260, 261, 262,
  263, 264, 274, 275, 276, 277, 278, 279, 280, 281, 282, 283, 284, 285,
  286, 287, 288, 290, 291, 292, 293, 294, 295, 296, 297, 298, 299 };

VOID SwitchWindowedMode()
{
	int *pnVideoMode = reinterpret_cast<int*>(0xC97C18);
	int nVideoMode = *pnVideoMode;

	int *pVideoModeInfo = *reinterpret_cast<int**>(0xC97C48) + (nVideoMode*5);

	// Change the windowed flag
	if (pVideoModeInfo[4])
		pVideoModeInfo[4] = 0;
	else
		pVideoModeInfo[4] = 1;

#ifdef _DEBUG

	char szBuffer[256];
	sprintf_s(szBuffer, "Video Mode Change: %d (windowed=%d)\n", nVideoMode, pVideoModeInfo[4]);
	OutputDebugString(szBuffer);

#endif

	// Reset the video mode to a non existant one so that we can change it
	*pnVideoMode = -1;

	// .text:00745C70                         ChangeVideoModeWrapper

	__asm
	{
		mov eax, 0x745C70;
		push nVideoMode;
		call eax;
		add esp, 4;
	}

	*pnVideoMode = nVideoMode;

}

BOOL HandleKeyPress(DWORD vKey) 
{
	static int i = -1;
	switch(vKey)
	{
		case VK_F6:
			if(!pCmdWindow->isEnabled()) {
				pCmdWindow->Enable();
			} else {
				pCmdWindow->Disable();
			}
			break;

		case VK_F7:			
			pChatWindow->CycleMode();
			break;

		case VK_F8:
			g_bTakeScreenshot = TRUE;
			break;
			
		case VK_F9:			
			pDeathWindow->ToggleEnabled();
			break;

		case VK_PRIOR:
	
			if (GetAsyncKeyState(VK_TAB))
			{
				if(!pNetGame) break;

				pScoreBoard->m_iOffset -= 20;
				if (pScoreBoard->m_iOffset < 0)
					pScoreBoard->m_iOffset = 0;
			} 
			else {
				if(pChatWindow) pChatWindow->PageUp();
			}

			break;

		case VK_NEXT:
		{
			if(GetAsyncKeyState(VK_TAB))
			{
				if(!pNetGame) break;

				pScoreBoard->m_iOffset += 20;
			
				CPlayerPool* pPlayerPool = pNetGame->GetPlayerPool();
				int playercount = 0;
				for (int x=0; x<MAX_PLAYERS; x++)
					if (pPlayerPool->GetSlotState(x) == TRUE)
						playercount++;
				if (pScoreBoard->m_iOffset > (playercount-19))
					pScoreBoard->m_iOffset = (playercount-19);
				if (pScoreBoard->m_iOffset < 0)
					pScoreBoard->m_iOffset = 0;
			}
			else {
				if(pChatWindow) pChatWindow->PageDown();
			}
			break;
		}

		case VK_SUBTRACT:
		{
			if (!tSettings.bDebug) break;
			cmdRawSave("");
			break;
		}

		case VK_F11:
		{
			if(!tSettings.bDebug) break;

			i++; if (i > (sizeof(dwWorkingSkins) / 4)) i = (sizeof(dwWorkingSkins) / 4);
			pGame->FindPlayerPed()->SetModelIndex(dwWorkingSkins[i]);
			pChatWindow->AddDebugMessage("SkinId = %d", dwWorkingSkins[i]);
			break;
		}

		case VK_F12:
		{
			if(!tSettings.bDebug) break;

			i--; if (i < 0) i = 0;
			pGame->FindPlayerPed()->SetModelIndex(dwWorkingSkins[i]);
			pChatWindow->AddDebugMessage("SkinId = %d", dwWorkingSkins[i]);
			break;
		}
		case VK_ESCAPE:
		{
			if(pCmdWindow->isEnabled()) {
				pCmdWindow->Disable();
			}
			break;
		}
		/*case VK_F1:
			GUI_ToggleHelp();
			break;*/

		case 38: // Will be up
			if (pCmdWindow->isEnabled())
			{
				pCmdWindow->RecallUp();
			}
			break;
		case 40: // Will be down
			if (pCmdWindow->isEnabled())
			{
				pCmdWindow->RecallDown();
			}
			break;
	}


	return FALSE;
}

//----------------------------------------------------

BOOL HandleCharacterInput(DWORD dwChar)
{
	if (pGame->IsMenuActive())
		return FALSE;

	if(pCmdWindow->isEnabled()) {
		switch (dwChar)
		{
			case 27: // escape
				pCmdWindow->Disable();
				return TRUE;
		}
	}
	else {
		switch(dwChar) {
			case '`':
			case 't':
			case 'T':
				pCmdWindow->Enable();
				return TRUE;
		}
	}
	return FALSE;
}

//----------------------------------------------------

bool SubclassGameWindow()
{
	HWND hwndGameWnd = pGame->GetMainWindowHwnd();

	SetWindowText(hwndGameWnd,"GTA:SA:MP");

	/*
	if(IsWindowUnicode(hwndGameWnd)) {
		OutputDebugString("GTA is unicode");
	} else {
		OutputDebugString("GTA is not unicode");
	}*/
	
	if(hwndGameWnd) {
		hOldProc = (WNDPROC)GetWindowLong(hwndGameWnd,GWL_WNDPROC);
		SetWindowLong(hwndGameWnd,GWL_WNDPROC,(LONG)NewWndProc);
		return true;
	}
	return false;
}

//----------------------------------------------------

//extern CDXUTDialog * pTestUI;

LRESULT APIENTRY NewWndProc( HWND hwnd,UINT uMsg,
							 WPARAM wParam,LPARAM lParam ) 
{
	if(pCmdWindow) {
		pCmdWindow->MsgProc(uMsg,wParam,lParam);
	}
	if (pSpawnScreen)
		pSpawnScreen->MsgProc(hwnd, uMsg, wParam, lParam);

	switch(uMsg) {
		case WM_SYSKEYUP:
			if (wParam == VK_RETURN)
				SwitchWindowedMode();
			break;
		case WM_KEYDOWN:
			if(wParam == VK_ESCAPE) {
				if(pCmdWindow->isEnabled()) {
					return 0;
				}
			}
			break;
		case WM_KEYUP:
			HandleKeyPress((DWORD)wParam);
			break;
		case WM_CHAR:
			HandleCharacterInput((DWORD)wParam);
			break;
	}

	return CallWindowProc(hOldProc,hwnd,uMsg,wParam,lParam);
}

//----------------------------------------------------

int GetScreenshotFileName(std::string & FileName)
{
    char Buf[MAX_PATH];
    WIN32_FIND_DATA ffd;
    HANDLE h;
	int i;
    for (i = 0; i < 1000; i++)
    {
		sprintf_s(Buf, "%s\\screens\\sa-mp-%03i.jpg", szUserDocPath, i);
        h = FindFirstFile(Buf, &ffd);
        if(h != INVALID_HANDLE_VALUE) {   
			FindClose(h);
		}
        else {   
			break;
		}
    }
    FileName = Buf;
	return i;
}

//----------------------------------------------------
// EOF