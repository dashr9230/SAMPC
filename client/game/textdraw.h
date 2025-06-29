#pragma once

/*

SA:MP Multiplayer Modification
Copyright 2004-2005 SA:MP Team

Version: $Id: textdraw.h,v 1.3 2008-02-11 22:30:15 kyecvs Exp $

*/

//-----------------------------------------------------------

typedef struct _TEXT_DRAW_DATA
{
	float fLetterWidth;		// -4
	float fLetterHeight;	// 0
	DWORD dwLetterColor;	// 4
	BYTE byteUnk12;			// 8
	BYTE byteCentered;		// 9
	BYTE byteBox;			// 10
	//PADDING(_pad15, 1);		// 11
	float fLineWidth;		// 12
	float fLineHeight;		// 16
	DWORD dwBoxColor;		// 20
	BYTE byteProportional;	// 24
	DWORD dwBackgroundColor;// 25-29
	BYTE byteShadow;		// 29
	BYTE byteOutline;		// 30
	BYTE byteAlignLeft;		// 31
	BYTE byteAlignRight;	// 32
	//PADDING(_pad37, 3);		// 33,34,35
	DWORD dwStyle;			// 36
	float fX;				// 40
	float fY;				// 44
	char cGXT[8];			// 48
	DWORD dwParam1;			// 60-64
	DWORD dwParam2;			// 64-68
	int iSpriteID;
	BYTE byteSelectable;
	bool bHasBoundingBox; // or BYTE?
} TEXT_DRAW_DATA;

//----------------------------------------------------

class CTextDraw
{
public:

	CHAR			m_szText[MAX_TEXT_DRAW_LINE+1];
	CHAR			m_szString[MAX_TEXT_DRAW_LINE*4];

	TEXT_DRAW_DATA  m_TextDrawData;
	
	RECT m_rcBoundingBox;
	bool m_bMouseover;
	DWORD m_dwHoverColor;

	CTextDraw(TEXT_DRAW_TRANSMIT *TextDrawTransmit, PCHAR szText);
	~CTextDraw(){};
	
	void SetupSprite();
	PCHAR GetText() { return m_szText; };
	void SetText(char* szText);

	void Draw();
};


//----------------------------------------------------
