#pragma once

/*

SA:MP Multiplayer Modification
Copyright 2004-2005 SA:MP Team

Version: $Id: textdrawpool.h,v 1.0 2007/05/18 19:26:45 Y_Less Exp $

*/

//----------------------------------------------------

class CTextDrawPool
{
private:

	bool				m_bSlotState[MAX_TEXT_DRAWS];
	TEXT_DRAW_TRANSMIT*	m_TextDraw[MAX_TEXT_DRAWS];
	PCHAR				m_szFontText[MAX_TEXT_DRAWS];
	bool				m_bHasText[MAX_TEXT_DRAWS][MAX_PLAYERS];

public:
	CTextDrawPool();
	~CTextDrawPool();
	
	WORD New(float fX, float fY, char* szText);
	void Delete(WORD wText);
	void ShowForPlayer(BYTE bytePlayer, WORD wText);
	void ShowForAll(WORD wText);
	void HideForPlayer(BYTE bytePlayer, WORD wText);
	void HideForAll(WORD wText);

	bool GetSlotState(int iText)
	{
		return (iText >= 0 && iText < MAX_TEXT_DRAWS) ? m_bSlotState[iText] : false;
	};
	
	void SetLetterSize(WORD wText, float fXSize, float fYSize);
	void SetTextSize(WORD wText, float fXSize, float fYSize);
	void SetAlignment(WORD wText, BYTE byteAlign);
	void SetColor(WORD wText, DWORD dwColor);
	void SetUseBox(WORD wText, BYTE byteUse) { m_TextDraw[wText]->byteBox = byteUse ? 1 : 0; };
	void SetBoxColor(WORD wText, DWORD dwColor);
	void SetShadow(WORD wText, BYTE byteShadow) { m_TextDraw[wText]->byteShadow = byteShadow; };
	void SetOutline(WORD wText, BYTE byteOutline) { m_TextDraw[wText]->byteOutline = byteOutline; };
	void SetBackgroundColor(WORD wText, DWORD dwColor);
	void SetFont(WORD wText, BYTE byteFont) { m_TextDraw[wText]->byteStyle = byteFont; };
	void SetTextString(WORD wText, char* szText);
	void SetProportional(WORD wText, BYTE byteSet) { m_TextDraw[wText]->byteProportional = byteSet ? 1 : 0; };
	void SetSelectable(WORD wText, BYTE byteSet);
};

//----------------------------------------------------
