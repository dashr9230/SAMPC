/*

SA:MP Multiplayer Modification
Copyright 2004-2005 SA:MP Team

Version: $Id: textdrawpool.cpp,v 1.0 2007/05/18 19:26:45 Y_Less Exp $

*/

#include "main.h"

//----------------------------------------------------

CTextDrawPool::CTextDrawPool()
{
	for (WORD wText = 0; wText < MAX_TEXT_DRAWS; wText++)
	{
		m_bSlotState[wText] = false;
		m_TextDraw[wText] = NULL;
		m_szFontText[wText] = NULL;
	}
}

CTextDrawPool::~CTextDrawPool()
{
	for (WORD wText = 0; wText < MAX_TEXT_DRAWS; wText++)
	{
		if (m_TextDraw[wText])
		{
			free(m_TextDraw[wText]);
			m_TextDraw[wText] = NULL;
		}
		if (m_szFontText[wText])
		{
			free(m_szFontText[wText]);
			m_szFontText[wText] = NULL;
		}
	}
}

WORD CTextDrawPool::New(float fX, float fY, char* szText)
{
	WORD wText = 0;
	while (wText < MAX_TEXT_DRAWS)
	{
		if (!m_bSlotState[wText]) break;
		wText++;
	}
	if (wText == MAX_TEXT_DRAWS) return 0xFFFF;
	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		m_bHasText[wText][i] = false;
	}
	
	TEXT_DRAW_TRANSMIT* TextDraw = (TEXT_DRAW_TRANSMIT*)malloc(sizeof (TEXT_DRAW_TRANSMIT));
	PCHAR Text = (PCHAR)calloc(MAX_TEXT_DRAW_LINE,1);
	
	if (TextDraw && Text)
	{
		strncpy(Text, szText, MAX_TEXT_DRAW_LINE);
		Text[MAX_TEXT_DRAW_LINE - 1] = '\0';
		TextDraw->fLetterWidth = 0.48f;
		TextDraw->fLetterHeight = 1.12f;
		TextDraw->dwLetterColor = 0xFFE1E1E1; // ABGR
		TextDraw->byteCenter = 0;
		TextDraw->byteBox = 0;
		TextDraw->fLineWidth = 1280.0f;
		TextDraw->fLineHeight = 1280.0f;
		TextDraw->dwBoxColor = 0x80808080; // ABGR
		TextDraw->byteProportional = 1;
		TextDraw->dwBackgroundColor = 0xFF000000; // ABGR
		TextDraw->byteShadow = 2;
		TextDraw->byteOutline = 0;
		TextDraw->byteLeft = 0;
		TextDraw->byteRight = 0;
		TextDraw->byteStyle = 1;
		TextDraw->byteSelectable = 0;
		TextDraw->fX = fX;
		TextDraw->fY = fY;
		m_TextDraw[wText] = TextDraw;
		m_szFontText[wText] = Text;
		m_bSlotState[wText] = true;
		return wText;
	}
	return 0xFFFF;
}

void CTextDrawPool::Delete(WORD wText)
{
	if (m_TextDraw[wText])
	{
		free(m_TextDraw[wText]);
		m_TextDraw[wText] = NULL;
	}
	if (m_szFontText[wText])
	{
		free(m_szFontText[wText]);
		m_szFontText[wText] = NULL;
	}
	m_bSlotState[wText] = false;
	HideForAll(wText);
}

void CTextDrawPool::ShowForPlayer(BYTE bytePlayer, WORD wText)
{
	RakServerInterface* pRak = pNetGame->GetRakServer();
	RakNet::BitStream bsParams;
	bsParams.Write(wText);
	bsParams.Write((PCHAR)m_TextDraw[wText], sizeof (TEXT_DRAW_TRANSMIT));
	bsParams.Write(m_szFontText[wText], MAX_TEXT_DRAW_LINE);
	pRak->RPC(RPC_ScrShowTextDraw, &bsParams, HIGH_PRIORITY, RELIABLE, 0, pRak->GetPlayerIDFromIndex(bytePlayer), false, false);
	m_bHasText[wText][bytePlayer] = true;
}

void CTextDrawPool::ShowForAll(WORD wText)
{
	RakNet::BitStream bsParams;
	bsParams.Write(wText);
	bsParams.Write((PCHAR)m_TextDraw[wText], sizeof (TEXT_DRAW_TRANSMIT));
	bsParams.Write(m_szFontText[wText], MAX_TEXT_DRAW_LINE);
	pNetGame->GetRakServer()->RPC(RPC_ScrShowTextDraw, &bsParams, HIGH_PRIORITY, RELIABLE, 0, UNASSIGNED_PLAYER_ID, true, false);
	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		m_bHasText[wText][i] = true;
	}
}

void CTextDrawPool::HideForPlayer(BYTE bytePlayer, WORD wText)
{
	RakServerInterface* pRak = pNetGame->GetRakServer();
	RakNet::BitStream bsParams;
	bsParams.Write(wText);
	if (m_bHasText[wText][bytePlayer]) pRak->RPC(RPC_ScrHideTextDraw, &bsParams, HIGH_PRIORITY, RELIABLE, 0, pRak->GetPlayerIDFromIndex(bytePlayer), false, false);
	m_bHasText[wText][bytePlayer] = false;
}

void CTextDrawPool::HideForAll(WORD wText)
{
	RakNet::BitStream bsParams;
	bsParams.Write(wText);
	RakServerInterface* pRak = pNetGame->GetRakServer();
	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		if (m_bHasText[wText][i])
		{
			pRak->RPC(RPC_ScrHideTextDraw, &bsParams, HIGH_PRIORITY, RELIABLE, 0, pRak->GetPlayerIDFromIndex(i), false, false);
			m_bHasText[wText][i] = false;
		}
	}
}

void CTextDrawPool::SetLetterSize(WORD wText, float fXSize, float fYSize)
{
	m_TextDraw[wText]->fLetterWidth = fXSize;
	m_TextDraw[wText]->fLetterHeight = fYSize;
}

void CTextDrawPool::SetTextSize(WORD wText, float fXSize, float fYSize)
{
	m_TextDraw[wText]->fLineWidth = fXSize;
	m_TextDraw[wText]->fLineHeight = fYSize;
}

void CTextDrawPool::SetTextString(WORD wText, char* szText)
{
	if (m_szFontText[wText])
	{
		strncpy(m_szFontText[wText], szText, MAX_TEXT_DRAW_LINE);
		RakServerInterface* pRak = pNetGame->GetRakServer();
		RakNet::BitStream bsParams;
		bsParams.Write(wText);
		bsParams.Write(szText, MAX_TEXT_DRAW_LINE);
		for (int i = 0; i < MAX_PLAYERS; i++)
		{
			if (m_bHasText[wText][i])
			{
				pRak->RPC(RPC_ScrEditTextDraw, &bsParams, HIGH_PRIORITY, RELIABLE, 0, pRak->GetPlayerIDFromIndex(i), false, false);
			}
		}
	}
}

void CTextDrawPool::SetAlignment(WORD wText, BYTE byteAlign)
{
	// 0, 0, 0 is the default
	m_TextDraw[wText]->byteLeft = 0;
	m_TextDraw[wText]->byteCenter = 0;
	m_TextDraw[wText]->byteRight = 0;
	if (byteAlign == 1) m_TextDraw[wText]->byteLeft = 1;
	else if (byteAlign == 2) m_TextDraw[wText]->byteCenter = 1;
	else if (byteAlign == 3) m_TextDraw[wText]->byteRight = 1;
}

void CTextDrawPool::SetColor(WORD wText, DWORD dwColor)
{
	// Needs converting from RGBA to ABGR
	m_TextDraw[wText]->dwLetterColor = RGBA_ABGR(dwColor);
}

void CTextDrawPool::SetBoxColor(WORD wText, DWORD dwColor)
{
	m_TextDraw[wText]->dwBoxColor = RGBA_ABGR(dwColor);
}

void CTextDrawPool::SetBackgroundColor(WORD wText, DWORD dwColor)
{
	m_TextDraw[wText]->dwBackgroundColor = RGBA_ABGR(dwColor);
}

void CTextDrawPool::SetSelectable(WORD wText, BYTE byteSelect)
{
	m_TextDraw[wText]->byteSelectable = byteSelect;
}