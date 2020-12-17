//----------------------------------------------------------
//
//   SA:MP Multiplayer Modification For GTA:SA
//   Copyright 2004-2006 SA:MP team
//
//----------------------------------------------------------

#pragma once

class CFontRender
{
public:
	CFontRender(IDirect3DDevice9* pD3DDevice);
	~CFontRender();

	void CreateFonts();

	void DeleteDeviceObjects();
	void RestoreDeviceObjects();

	SIZE MeasureText(SIZE* size, char* szString, DWORD dwFormat);
	SIZE MeasureSmallerText(SIZE* size, char* szString, DWORD dwFormat);

	void RenderText(char* sz, RECT rect, DWORD dwColor);
	void RenderText(char* sz, DWORD x, DWORD y, DWORD dwColor);

	void RenderText(ID3DXSprite* pCustomSprite, char* szBuf, RECT rect, DWORD dwFormat, D3DCOLOR color, bool bShadowed);
	void RenderSmallerText(ID3DXSprite* pCustomSprite, char* szBuf, RECT rect, DWORD dwFormat, D3DCOLOR color, bool bShadowed);
	void DrawTextForPlate(char* szBuf, RECT rect, D3DCOLOR color);

	ID3DXFontCE* GetDXFontCE() { return m_pD3DFontEmbed; };
	ID3DXFont* GetDXFont() { return m_pD3DFont; };

private:
	ID3DXFont* m_pD3DFont;
	ID3DXFontCE* m_pD3DFontEmbed;
	ID3DXFont* m_pD3DFontSmaller;
	ID3DXFontCE* m_pD3DFontSmallerEmbed;
	ID3DXFontCE* m_pD3DFontArialEmbed;
	IDirect3DDevice9* m_pD3DDevice;
	ID3DXSprite* m_pD3DSprite;
	char* m_szBuffer;
	long m_lFontSizeY;
	long m_lSmallerFontSizeY;

};
extern signed int uiFontSize;
