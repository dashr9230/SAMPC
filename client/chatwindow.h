//----------------------------------------------------------
//
// SA:MP Multiplayer Modification For GTA:SA
// Copyright 2004-2005 SA:MP team
//
// Version: $Id: chatwindow.h,v 1.10 2006/05/08 13:28:46 kyeman Exp $
//
//----------------------------------------------------------

#pragma once

#define MAX_MESSAGE_LENGTH			128
#define MAX_LINE_LENGTH				MAX_MESSAGE_LENGTH / 2
#define MAX_MESSAGES				50
#define DISP_MESSAGES				10
#define CHAT_WINDOW_PAGES			MAX_MESSAGES / DISP_MESSAGES

enum eChatMessageType {
	CHAT_TYPE_NONE=0,
	CHAT_TYPE_CHAT,
	CHAT_TYPE_INFO,
	CHAT_TYPE_DEBUG
};

typedef struct _CHAT_WINDOW_ENTRY
{
	eChatMessageType eType;
	CHAR szMessage[MAX_MESSAGE_LENGTH+1];
	CHAR szNick[MAX_PLAYER_NAME+1];
	DWORD dwTextColor;
	DWORD dwNickColor;
	char szTimeStamp[12];
} CHAT_WINDOW_ENTRY;

#define CHAT_WINDOW_MODE_OFF	0
#define CHAT_WINDOW_MODE_LIGHT	1
#define CHAT_WINDOW_MODE_FULL	2

//----------------------------------------------------

class CChatWindow
{
private:
	unsigned int	m_uiPageSize;
	bool m_bTimeStamp;
	int					m_iEnabled;
	int					m_iCurrentPage;
	CHAT_WINDOW_ENTRY	m_ChatWindowEntries[MAX_MESSAGES];
	DWORD				m_dwChatTextColor;
	DWORD				m_dwChatInfoColor;
	DWORD				m_dwChatDebugColor;
	DWORD				m_dwChatBackgroundColor;
	LONG				m_lChatWindowBottom;
	bool m_bForcedHidden;

	CDXUTDialog			*m_pGameUI;
	CDXUTEditBox		*m_pEditBackground;
	CDXUTScrollBar		*m_pScrollBar;

	void PushBack();
	void FilterInvalidChars(PCHAR szString);
	void AddToChatWindowBuffer(eChatMessageType eType,PCHAR szString,
		PCHAR szNick,DWORD dwTextColor,DWORD dwChatColor);
	void ChatLog(PCHAR szString, PCHAR szNick, eChatMessageType eType);
	
public:
	void Draw();
	void AddChatMessage(CHAR *szNick, DWORD dwNickColor, CHAR *szMessage);
	void AddInfoMessage(CHAR *szFormat, ...);
	void AddDebugMessage(CHAR *szFormat, ...);
	void AddClientMessage(DWORD dwColor, PCHAR szStr);

	void PageUp();
	void PageDown();

	void CycleMode();

	void RenderText(CHAR *sz,RECT rect,DWORD dwColor);
	void ResetDialogControls(CDXUTDialog *pGameUI);

	CChatWindow::CChatWindow(IDirect3DDevice9 *pD3DDevice, ID3DXFont *pFont);
	~CChatWindow();

	LONG GetChatWindowBottom() { return m_lChatWindowBottom; };
	void ScrollBarPosFromCurrentPage();

	LONG				m_lFontSizeY;
	ID3DXFont			*m_pD3DFont;
	ID3DXSprite			*m_pChatTextSprite;
	IDirect3DDevice9	*m_pD3DDevice;

	void SetPageSize(unsigned int uiSize);
	void ToggleTimeStamp();
	void ForceHide(bool bHide);
};

//----------------------------------------------------
// EOF
