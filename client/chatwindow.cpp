//----------------------------------------------------------
//
// SA:MP Multiplayer Modification For GTA:SA
// Copyright 2004-2005 SA:MP team
//
// Version: $Id: chatwindow.cpp,v 1.20 2006/05/08 14:31:50 kyeman Exp $
//
//----------------------------------------------------------

#include "main.h"

//----------------------------------------------------

CChatWindow::CChatWindow(IDirect3DDevice9 *pD3DDevice, CFontRender* pFontRender, char* szPath)
{
	int x=0;
	FILE* pFile;

	m_pD3DDevice		= pD3DDevice;
	//m_pD3DFont			= pFont;
	m_pFontRender		= pFontRender;
	m_iEnabled			= CHAT_WINDOW_MODE_FULL;
	m_iCurrentPage		= 1;
	m_uiPageSize		= DISP_MESSAGES;
	m_bTimeStamp = false;
	m_bForcedHidden = false;
	m_bLogFileCreated = false;

	// Create a sprite to use when drawing text
	D3DXCreateSprite(pD3DDevice,&m_pChatTextSprite);

	// Init the chat window lines to 0
	while(x!=MAX_MESSAGES) {
		memset(&m_ChatWindowEntries[x],0,sizeof(CHAT_WINDOW_ENTRY));
		x++;
	}

	//RECT rectSize;
	//m_pD3DFont->DrawText(0,"Y",-1,&rectSize,DT_CALCRECT|DT_SINGLELINE|DT_LEFT,0xFF000000);
	//m_lFontSizeY = rectSize.bottom - rectSize.top;

	SIZE size;
	m_lFontSizeY = pFontRender->MeasureText(&size, "Y", DT_SINGLELINE | DT_LEFT).cy;
	if (szPath && szPath[0] != '\0') //&& strlen(szPath))
	{
		sprintf_s(m_szLogFile, "%s\\" CHAT_LOG_FILE, szPath);
		fopen_s(&pFile, m_szLogFile, "w");
		if (pFile)
		{
			m_bLogFileCreated = true;
			fclose(pFile);
		}
	}
	

	m_dwChatTextColor = D3DCOLOR_ARGB(255,255,255,255);
	m_dwChatInfoColor = D3DCOLOR_ARGB(255, 136, 170, 98);
	m_dwChatDebugColor = D3DCOLOR_ARGB(255, 169, 196, 228);
	m_dwChatBackgroundColor = D3DCOLOR_ARGB(255,0,0,0);
}

//----------------------------------------------------

CChatWindow::~CChatWindow() {}

//----------------------------------------------------

void CChatWindow::SetPageSize(unsigned int uiSize)
{
	m_uiPageSize = uiSize;

	UpdateScrollBar();
}

void CChatWindow::ToggleTimeStamp()
{
	m_bTimeStamp = !m_bTimeStamp;
}

void CChatWindow::ForceHide(bool bHide)
{
	m_bForcedHidden = bHide;
}

void CChatWindow::UpdateScrollBar()
{
	if (m_pScrollBar)
	{
		m_pScrollBar->SetLocation(5, 40);
		m_pScrollBar->SetSize(15, ((m_lFontSizeY + 1) * m_uiPageSize) - 60);
		m_pScrollBar->SetTrackRange(0, (MAX_MESSAGES - m_uiPageSize) - 1);
		m_pScrollBar->SetPageSize(5);
		m_pScrollBar->ShowItem(MAX_MESSAGES - 1);
	}
}

void CChatWindow::ResetDialogControls(CDXUTDialog *pGameUI)
{
	m_pGameUI = pGameUI;

	if(pGameUI) {

		m_pScrollBar = new CDXUTScrollBar(pGameUI);
		pGameUI->AddControl(m_pScrollBar);
		m_pScrollBar->SetVisible(true);
		m_pScrollBar->SetEnabled(true);

		UpdateScrollBar();
	}
}

//----------------------------------------------------

void CChatWindow::ScrollBarPosFromCurrentPage()
{
	if(m_pScrollBar) {
		if(m_iCurrentPage==1) m_pScrollBar->ShowItem(39);
		if(m_iCurrentPage==2) m_pScrollBar->ShowItem(29);
		if(m_iCurrentPage==3) m_pScrollBar->ShowItem(19);
		if(m_iCurrentPage==4) m_pScrollBar->ShowItem(9);
		if(m_iCurrentPage==5) m_pScrollBar->ShowItem(0);
	}
}

//----------------------------------------------------

void CChatWindow::PageUp()
{
	if(!m_iEnabled) return;

	m_iCurrentPage++;
	if(m_iCurrentPage > CHAT_WINDOW_PAGES) {
		m_iCurrentPage = CHAT_WINDOW_PAGES;
	} else {
		ScrollBarPosFromCurrentPage();
	}
}

//----------------------------------------------------

void CChatWindow::PageDown()
{
	if(!m_iEnabled) return;

	m_iCurrentPage--;
	if(m_iCurrentPage < 1) {
		m_iCurrentPage = 1;
	} else {
		ScrollBarPosFromCurrentPage();
	}
}

void CChatWindow::CycleMode()
{
	if (!m_bForcedHidden && (m_iEnabled--) <= 0)
		m_iEnabled = CHAT_WINDOW_MODE_FULL;
}

//----------------------------------------------------

void CChatWindow::Draw()
{
	if (m_bForcedHidden)
		return;

	DWORD dwColorChat=0;
	RECT rect;
	RECT rectSize;
	int x=0;
	int i=0;
	int iMessageAt;

	rect.top		= 10;
	rect.left		= 30;
	rect.bottom		= 110;
	rect.right		= 550;

	if(!m_iEnabled || m_iCurrentPage == 1) {
		m_pScrollBar->SetVisible(false);
	} else {
		m_pScrollBar->SetVisible(true);
	}

	iMessageAt = (m_iCurrentPage * m_uiPageSize) - 1;

	if(/*m_pD3DFont &&*/ m_iEnabled)
	{
		m_pChatTextSprite->Begin( D3DXSPRITE_ALPHABLEND /*| D3DXSPRITE_SORT_TEXTURE*/ );

		while(x!= m_uiPageSize) {

			if (m_bTimeStamp)
			{
				m_pFontRender->GetDXFont()->DrawText(0, m_ChatWindowEntries[iMessageAt].szTimeStamp, -1, &rectSize, DT_CALCRECT | DT_LEFT, 0xFF000000);
				RenderText(m_ChatWindowEntries[iMessageAt].szTimeStamp, rect, m_ChatWindowEntries[iMessageAt].dwTextColor);
				rect.left = 35 + (rectSize.right - rectSize.left);
			}

			switch(m_ChatWindowEntries[iMessageAt].eType) {

			case CHAT_TYPE_CHAT:

				i = strlen(m_ChatWindowEntries[iMessageAt].szNick);

				if(i) {
					m_pFontRender->GetDXFont()->DrawText(0,m_ChatWindowEntries[iMessageAt].szNick,-1,&rectSize,DT_CALCRECT|DT_LEFT,0xFF000000);
					RenderText(m_ChatWindowEntries[iMessageAt].szNick,rect,m_ChatWindowEntries[iMessageAt].dwNickColor);
					rect.left = (m_bTimeStamp) ? (rect.left + (rectSize.right - rectSize.left)) : (35 + (rectSize.right - rectSize.left));
					//rect.left = (m_bTimeStamp) ? rect.left + size.cx : 35 + size.cy;
				}

				RenderText(m_ChatWindowEntries[iMessageAt].szMessage,rect,m_ChatWindowEntries[iMessageAt].dwTextColor);

				break;

			case CHAT_TYPE_INFO:
			case CHAT_TYPE_DEBUG:

				RenderText(m_ChatWindowEntries[iMessageAt].szMessage,rect,m_ChatWindowEntries[iMessageAt].dwTextColor);
				break;
			}

			rect.top+=m_lFontSizeY+1;
			rect.bottom= rect.top + m_lFontSizeY+1;
			rect.left = 30;

			iMessageAt--;
			x++;
		}
		m_lChatWindowBottom = rect.bottom;

		m_pChatTextSprite->End();
	}
}

//----------------------------------------------------

void CChatWindow::AddChatMessage(CHAR *szNick, DWORD dwNickColor, CHAR *szMessage)
{
	FilterInvalidChars(szMessage);

	if(strlen(szMessage) > MAX_MESSAGE_LENGTH) return;

	AddToChatWindowBuffer(CHAT_TYPE_CHAT,szMessage,szNick,m_dwChatTextColor,dwNickColor);
}

//----------------------------------------------------

void CChatWindow::AddInfoMessage(CHAR * szFormat, ...)
{
	char tmp_buf[512];
	memset(tmp_buf,0,512);

	va_list args;
	va_start(args, szFormat);
	vsprintf_s(tmp_buf, szFormat, args);
	va_end(args);

	FilterInvalidChars(tmp_buf);

	if(strlen(tmp_buf) > MAX_MESSAGE_LENGTH) return;

	AddToChatWindowBuffer(CHAT_TYPE_INFO,tmp_buf,NULL,m_dwChatInfoColor,0);
}

//----------------------------------------------------

void CChatWindow::AddDebugMessage(CHAR * szFormat, ...)
{
	char tmp_buf[512];
	memset(tmp_buf,0,512);

	va_list args;
	va_start(args, szFormat);
	vsprintf_s(tmp_buf, szFormat, args);
	va_end(args);

	FilterInvalidChars(tmp_buf);

	if(strlen(tmp_buf) > MAX_MESSAGE_LENGTH) return;

	AddToChatWindowBuffer(CHAT_TYPE_DEBUG,tmp_buf,NULL,m_dwChatDebugColor,0);
	OutputDebugString(tmp_buf);
}

//----------------------------------------------------

void CChatWindow::AddClientMessage(DWORD dwColor, PCHAR szStr)
{
	dwColor = (dwColor >> 8) | 0xFF000000; // convert to ARGB

	FilterInvalidChars(szStr);

	if(strlen(szStr) > MAX_MESSAGE_LENGTH) return;

	AddToChatWindowBuffer(CHAT_TYPE_INFO,szStr,NULL,dwColor,0);
}

//----------------------------------------------------

void CChatWindow::FilterInvalidChars(PCHAR szString)
{
	while(*szString) {
		if(*szString > 0 && *szString < ' ') {
			*szString = ' ';
		}
		szString++;
	}
}

//----------------------------------------------------

void CChatWindow::AddToChatWindowBuffer(eChatMessageType eType,
	PCHAR szString, PCHAR szNick, DWORD dwTextColor, DWORD dwChatColor)
{
	int iBestLineLength=0;
	SYSTEMTIME time;

	PushBack();

	m_ChatWindowEntries[0].eType = eType;
	m_ChatWindowEntries[0].dwTextColor = dwTextColor;
	m_ChatWindowEntries[0].dwNickColor = dwChatColor;

	if(szNick) {
		strcpy_s(m_ChatWindowEntries[0].szNick,szNick);
		strcat_s(m_ChatWindowEntries[0].szNick,":");
	} else {
		m_ChatWindowEntries[0].szNick[0] = '\0';
	}

	GetSystemTime(&time);
	snprintf(m_ChatWindowEntries[0].szTimeStamp, sizeof(m_ChatWindowEntries[0].szTimeStamp),
		"[%02d:%02d:%02d]", time.wHour, time.wMinute, time.wSecond);

	ChatLog(m_ChatWindowEntries[0].szTimeStamp, szString, szNick, eType);

	if(m_ChatWindowEntries[0].eType == CHAT_TYPE_CHAT && strlen(szString) > MAX_LINE_LENGTH)
	{
		iBestLineLength = MAX_LINE_LENGTH;
		// see if we can locate a space.
		while(szString[iBestLineLength] != ' ' && iBestLineLength)
			iBestLineLength--;

		if((MAX_LINE_LENGTH - iBestLineLength) > 12) {
			// we should just take the whole line
			strncpy_s(m_ChatWindowEntries[0].szMessage,szString,MAX_LINE_LENGTH);
			m_ChatWindowEntries[0].szMessage[MAX_LINE_LENGTH] = '\0';

			PushBack();

			m_ChatWindowEntries[0].eType = eType;
			m_ChatWindowEntries[0].dwTextColor = dwTextColor;
			m_ChatWindowEntries[0].dwNickColor = dwChatColor;
			m_ChatWindowEntries[0].szNick[0] = '\0';

			strcpy_s(m_ChatWindowEntries[0].szMessage,szString+MAX_LINE_LENGTH);
		}
		else {
			// grab upto the found space.
			strncpy_s(m_ChatWindowEntries[0].szMessage,szString,iBestLineLength);
			m_ChatWindowEntries[0].szMessage[iBestLineLength] = '\0';

			PushBack();

			m_ChatWindowEntries[0].eType = eType;
			m_ChatWindowEntries[0].dwTextColor = dwTextColor;
			m_ChatWindowEntries[0].dwNickColor = dwChatColor;
			m_ChatWindowEntries[0].szNick[0] = '\0';

			strcpy_s(m_ChatWindowEntries[0].szMessage,szString+(iBestLineLength+1));
		}
	}
	else {
		strncpy_s(m_ChatWindowEntries[0].szMessage,szString,MAX_MESSAGE_LENGTH);
		m_ChatWindowEntries[0].szMessage[MAX_MESSAGE_LENGTH] = '\0';
	}

}

//----------------------------------------------------

void CChatWindow::ChatLog(PCHAR szTimeStamp, PCHAR szString,
	PCHAR szNick, eChatMessageType eType)
{
	FILE *fileOut;

	if (m_bLogFileCreated)
	{
		fopen_s(&fileOut, m_szLogFile, "a");
		if (fileOut == NULL)
			return;

		if (eType == CHAT_TYPE_CHAT)
			fprintf_s(fileOut, "%s <%s> %s\n", szTimeStamp, szNick, szString);
		else
			fprintf_s(fileOut, "%s %s\n", szTimeStamp, szString);

		fclose(fileOut);
	}
}

//----------------------------------------------------

void CChatWindow::PushBack()
{
	int x=MAX_MESSAGES-1;
	while(x) {
		memcpy(&m_ChatWindowEntries[x],&m_ChatWindowEntries[x-1],sizeof(CHAT_WINDOW_ENTRY));
		x--;
	}
}

//----------------------------------------------------
#define FONT_RENDER_FORMAT DT_NOCLIP | DT_SINGLELINE | DT_LEFT

void CChatWindow::RenderText(CHAR *sz,RECT rect,DWORD dwColor)
{
	ID3DXFont* pFont;
	ID3DXFontCE* pFontCE;
	char szModifiedStr[512] = { 0 };
	size_t nOriginalLen, nModifiedLen;

	pFont = m_pFontRender->GetDXFont();
	pFontCE = m_pFontRender->GetDXFontCE();

	nOriginalLen = strlen(sz);

	if (m_iEnabled == CHAT_WINDOW_MODE_FULL) {

		//memset(szModifiedStr, 0, sizeof(szModifiedStr));
		strncpy_s(szModifiedStr, sz, sizeof(szModifiedStr) - 1);
		RemoveColorEmbedsFromString(szModifiedStr);
		nModifiedLen = strlen(szModifiedStr);
		if (pGame->GetScreenWidth() <= 1280)
		{
			rect.top -= 1;
			pFont->DrawTextA(m_pChatTextSprite, szModifiedStr, nModifiedLen, &rect, FONT_RENDER_FORMAT, 0xFF000000);
			rect.top += 2;
			pFont->DrawTextA(m_pChatTextSprite, szModifiedStr, nModifiedLen, &rect, FONT_RENDER_FORMAT, 0xFF000000);
			rect.top -= 1;
			rect.left -= 1;
			pFont->DrawTextA(m_pChatTextSprite, szModifiedStr, nModifiedLen, &rect, FONT_RENDER_FORMAT, 0xFF000000);
			rect.left += 2;
			pFont->DrawTextA(m_pChatTextSprite, szModifiedStr, nModifiedLen, &rect, FONT_RENDER_FORMAT, 0xFF000000);
			rect.left -= 1;
		}
		else
		{
			rect.top -= 2;
			rect.bottom -= 2;
			pFont->DrawTextA(m_pChatTextSprite, szModifiedStr, nModifiedLen, &rect, FONT_RENDER_FORMAT, 0xFF000000);
			rect.top += 1;
			rect.bottom += 1;
			rect.right -= 1;
			pFont->DrawTextA(m_pChatTextSprite, szModifiedStr, nModifiedLen, &rect, FONT_RENDER_FORMAT, 0xFF000000);
			rect.left += 2;
			rect.right += 2;
			pFont->DrawTextA(m_pChatTextSprite, szModifiedStr, nModifiedLen, &rect, FONT_RENDER_FORMAT, 0xFF000000);
			rect.left -= 1;
			rect.right -= 1;
			rect.top += 3;
			rect.bottom += 3;
			pFont->DrawTextA(m_pChatTextSprite, szModifiedStr, nModifiedLen, &rect, FONT_RENDER_FORMAT, 0xFF000000);
			rect.left -= 1;
			rect.top -= 1;
			rect.bottom -= 1;
			rect.right -= 1;
			pFont->DrawTextA(m_pChatTextSprite, szModifiedStr, nModifiedLen, &rect, FONT_RENDER_FORMAT, 0xFF000000);
			rect.left += 2;
			rect.right += 2;
			pFont->DrawTextA(m_pChatTextSprite, szModifiedStr, nModifiedLen, &rect, FONT_RENDER_FORMAT, 0xFF000000);
			rect.top -= 1;
			rect.bottom -= 1;
			rect.left -= 3;
			rect.right -= 3;
			pFont->DrawTextA(m_pChatTextSprite, szModifiedStr, nModifiedLen, &rect, FONT_RENDER_FORMAT, 0xFF000000);
			rect.left += 4;
			rect.right += 4;
			pFont->DrawTextA(m_pChatTextSprite, szModifiedStr, nModifiedLen, &rect, FONT_RENDER_FORMAT, 0xFF000000);
			rect.left -= 2;
			rect.right -= 2;
		}
	}
	pFontCE->DrawTextA(m_pChatTextSprite, sz, nOriginalLen, &rect, FONT_RENDER_FORMAT, dwColor | 0xFF000000);
}

//----------------------------------------------------
// EOF