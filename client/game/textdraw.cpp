/*

SA:MP Multiplayer Modification
Copyright 2004-2005 SA:MP Team

Version: $Id: textdraw.cpp,v 1.4 2008-04-16 08:54:17 kyecvs Exp $

*/

#include "../main.h"
#include "font.h"

//char ProvideTmp[1024];

CTextDraw::CTextDraw(TEXT_DRAW_TRANSMIT *TextDrawTransmit, PCHAR szText)
{
	memset(&m_TextDrawData,0,sizeof(TEXT_DRAW_DATA));

	// Set standard parameters
	m_TextDrawData.fLetterWidth = TextDrawTransmit->fLetterWidth;
	m_TextDrawData.fLetterHeight = TextDrawTransmit->fLetterHeight;
	m_TextDrawData.dwLetterColor = TextDrawTransmit->dwLetterColor;
	m_TextDrawData.byteUnk12 = 0;
	m_TextDrawData.byteCentered = TextDrawTransmit->byteCenter;
	m_TextDrawData.byteBox = TextDrawTransmit->byteBox;
	m_TextDrawData.fLineWidth = TextDrawTransmit->fLineWidth;
	m_TextDrawData.fLineHeight = TextDrawTransmit->fLineHeight;
	m_TextDrawData.dwBoxColor = TextDrawTransmit->dwBoxColor;
	m_TextDrawData.byteProportional = TextDrawTransmit->byteProportional;
	m_TextDrawData.dwBackgroundColor = TextDrawTransmit->dwBackgroundColor;
	m_TextDrawData.byteShadow = TextDrawTransmit->byteShadow;
	m_TextDrawData.byteOutline = TextDrawTransmit->byteOutline;
	m_TextDrawData.byteAlignLeft = TextDrawTransmit->byteLeft;
	m_TextDrawData.byteAlignRight = TextDrawTransmit->byteRight;
	m_TextDrawData.dwStyle = TextDrawTransmit->byteStyle;
	m_TextDrawData.byteSelectable = TextDrawTransmit->byteSelectable;
	m_TextDrawData.fX = TextDrawTransmit->fX;
	m_TextDrawData.fY = TextDrawTransmit->fY;
	m_TextDrawData.dwParam1 = 0xFFFFFFFF;
	m_TextDrawData.dwParam2 = 0xFFFFFFFF;
	m_TextDrawData.dwSelectColor = 0;
	m_TextDrawData.bColorSelect = false;
	m_TextDrawData.rSelectArea.left = 0;
	m_TextDrawData.rSelectArea.top = 0;
	m_TextDrawData.rSelectArea.right = 0;
	m_TextDrawData.rSelectArea.bottom = 0;
	
	strncpy(m_szText, szText, MAX_TEXT_DRAW_LINE);
	m_szText[MAX_TEXT_DRAW_LINE - 1] = '\0';
}

void CTextDraw::SetText(char* szText)
{
	memset(m_szText,0,MAX_TEXT_DRAW_LINE);
	strncpy(m_szText, szText, MAX_TEXT_DRAW_LINE);
	m_szText[MAX_TEXT_DRAW_LINE-1] = 0;
}


void CTextDraw::Draw()
{
	if(!m_szText || !strlen(m_szText)) return;

	strcpy(m_szString,m_szText);

	int iScreenWidth, iScreenHeight;
	float fVertHudScale, fHorizHudScale;

    iScreenWidth = pGame->GetScreenWidth();
	iScreenHeight = pGame->GetScreenHeight();
	fVertHudScale = pGame->GetHudVertScale();
	fHorizHudScale = pGame->GetHudHorizScale();

    float fScaleY = (float)iScreenHeight * fVertHudScale * m_TextDrawData.fLetterHeight * 0.5f;
	float fScaleX = (float)iScreenWidth * fHorizHudScale * m_TextDrawData.fLetterWidth;

	Font_SetScale(fScaleX,fScaleY);
	if (m_TextDrawData.bColorSelect)
		Font_SetColor(m_TextDrawData.dwSelectColor);
	else
		Font_SetColor(m_TextDrawData.dwLetterColor);
    Font_Unk12(0);

	if(m_TextDrawData.byteAlignRight) Font_SetJustify(2);
	else if(m_TextDrawData.byteCentered) Font_SetJustify(0);
    else Font_SetJustify(1);

	//Font_SetRightJustifyWrap(0.0f);

	float fLineWidth = iScreenWidth * fHorizHudScale * m_TextDrawData.fLineWidth;
	Font_SetLineWidth(fLineWidth);

	float fLineHeight = iScreenWidth * fHorizHudScale * m_TextDrawData.fLineHeight;
	Font_SetLineHeight(fLineHeight);

	Font_UseBox(m_TextDrawData.byteBox,0);
	Font_UseBoxColor(m_TextDrawData.dwBoxColor);
	Font_SetProportional(m_TextDrawData.byteProportional);
	Font_SetDropColor(m_TextDrawData.dwBackgroundColor);

	if(m_TextDrawData.byteOutline) {
		Font_SetOutline(m_TextDrawData.byteOutline);
	} else {
		Font_SetShadow(m_TextDrawData.byteShadow);
	}

	Font_SetFontStyle(m_TextDrawData.dwStyle);

	Font_UnkConv(m_szString,-1,-1,-1,-1,-1,-1,m_szString);
	Font_UnkConv2(m_szString);

	float fPS2Height = 448.0f;
	float fPS2Width = 640.0f;
	float fScriptY =  m_TextDrawData.fY;
	float fScriptX =  m_TextDrawData.fX;
	float fUseX,fUseY;

	fUseY = iScreenHeight - ((fPS2Height - fScriptY) * (iScreenHeight * fVertHudScale));
    fUseX = iScreenWidth - ((fPS2Width - fScriptX) * (iScreenWidth * fHorizHudScale));
    
    Font_PrintString(fUseX,fUseY,m_szString);
	Font_SetOutline(0);

	if (m_TextDrawData.byteAlignRight)
	{
		m_TextDrawData.rSelectArea.left = (LONG)(fUseX - (fLineWidth - fUseX));
		m_TextDrawData.rSelectArea.right = (LONG)fUseX;
		m_TextDrawData.rSelectArea.bottom = (LONG)(fUseY + fLineHeight);
	}
	else if (m_TextDrawData.byteCentered)
	{
		m_TextDrawData.rSelectArea.left = (LONG)(fUseX - fLineHeight * 0.5);
		m_TextDrawData.rSelectArea.right = (m_TextDrawData.rSelectArea.left + (LONG)fLineHeight);
		m_TextDrawData.rSelectArea.bottom = (LONG)(fUseY + fLineWidth);
	}
	else
	{
		m_TextDrawData.rSelectArea.left = (LONG)fUseX;
		m_TextDrawData.rSelectArea.right = (LONG)(fLineWidth - (fUseX + fUseX));
		m_TextDrawData.rSelectArea.bottom = (LONG)(fUseY + fLineHeight);
	}

	m_TextDrawData.rSelectArea.top = (LONG)fUseY;
}
