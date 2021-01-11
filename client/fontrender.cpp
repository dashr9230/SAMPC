//----------------------------------------------------------
//
//   SA:MP Multiplayer Modification For GTA:SA
//   Copyright 2004-2006 SA:MP team
//
//----------------------------------------------------------

#include "main.h"

//----------------------------------------------------

CFontRender::CFontRender(IDirect3DDevice9* pD3DDevice)
{
	m_pD3DDevice = pD3DDevice;

	m_pD3DFont = NULL;
	m_pD3DFontEmbed = NULL;
	m_pD3DFontSmaller = NULL;
	m_pD3DFontSmallerEmbed = NULL;
	m_pD3DFontArialEmbed = NULL;
	m_pD3DSprite = NULL;
	m_szBuffer = NULL;

	CreateFonts();
}

CFontRender::~CFontRender()
{
	SAFE_RELEASE(m_pD3DFontEmbed);
	SAFE_RELEASE(m_pD3DFontSmallerEmbed);
	SAFE_RELEASE(m_pD3DFont);
	SAFE_RELEASE(m_pD3DFontSmaller);
	SAFE_RELEASE(m_pD3DSprite);
	SAFE_RELEASE(m_pD3DFontArialEmbed);
}

// Font size multiplier ranges: -3 to 5
int GetFontSize()
{
	int n, w = pGame->GetScreenWidth();

	if (w >= 1600)
		n = 20;
	else if (w >= 1400)
		n = 18;
	else if (w >= 1024)
		n = 16;
	else
		n = 14;

	return n + 2 * pConfig->GetInt("fontsize");
}

int GetFontWeight()
{
	int r = pConfig->GetInt("fontweight");

	return r != 1 ? FW_BOLD : FW_NORMAL;
}

char* GetFontFace()
{
	if (pConfig->GetString("fontface") == NULL) return "Arial";
	return pConfig->GetString("fontface");
}

void CFontRender::CreateFonts()
{	
	int iFontSize, iFontWeight, iSmallerFont;
	char* szFontFace;
	ID3DXFont* pFont;
	SIZE size;

	if (!m_pD3DDevice) return;

	SAFE_RELEASE(m_pD3DFontEmbed);
	SAFE_RELEASE(m_pD3DFontSmallerEmbed);
	SAFE_RELEASE(m_pD3DFont);
	SAFE_RELEASE(m_pD3DFontSmaller);
	SAFE_RELEASE(m_pD3DSprite);
	SAFE_RELEASE(m_pD3DFontArialEmbed);

	iFontSize = GetFontSize();
	iFontWeight = GetFontWeight();
	szFontFace = GetFontFace();

	iSmallerFont = iFontSize - 2;

	D3DXCreateFontA(m_pD3DDevice, iFontSize, 0, iFontWeight, 1, FALSE, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, szFontFace, &pFont);

	m_pD3DFontEmbed = new ID3DXFontCE;
	m_pD3DFontEmbed->SetDXFont(pFont);

	D3DXCreateFontA(m_pD3DDevice, iFontSize, 0, iFontWeight, 1, FALSE, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, szFontFace, &m_pD3DFont);

	D3DXCreateFontA(m_pD3DDevice, iSmallerFont, 0, iFontWeight, 1, FALSE, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, szFontFace, &pFont);
		
	m_pD3DFontSmallerEmbed = new ID3DXFontCE;
	m_pD3DFontSmallerEmbed->SetDXFont(pFont);

	D3DXCreateFontA(m_pD3DDevice, iSmallerFont, 0, iFontWeight, 1, FALSE, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, szFontFace, &m_pD3DFontSmaller);

	D3DXCreateSprite(m_pD3DDevice, &m_pD3DSprite);

	m_szBuffer = (char*)calloc(1, MAX_STRING_LENGTH + 1);

	D3DXCreateFontA(m_pD3DDevice, 38, 10, FW_BOLD, 1, FALSE, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, "Arial", &pFont);

	m_pD3DFontArialEmbed = new ID3DXFontCE;
	m_pD3DFontArialEmbed->SetDXFont(pFont);

	m_lFontSizeY = MeasureText(&size, "Y", 0).cy;
	m_lSmallerFontSizeY = MeasureSmallerText(&size, "Y", 0).cy;
}

void CFontRender::DeleteDeviceObjects() 
{
	m_pD3DFont->OnLostDevice();
	m_pD3DFontEmbed->OnLostDevice();
	m_pD3DFontSmaller->OnLostDevice();
	m_pD3DFontSmallerEmbed->OnLostDevice();
	m_pD3DFontArialEmbed->OnLostDevice();
	m_pD3DSprite->OnLostDevice();
}

void CFontRender::RestoreDeviceObjects()
{
	m_pD3DFont->OnResetDevice();
	m_pD3DFontEmbed->OnResetDevice();
	m_pD3DFontSmaller->OnResetDevice();
	m_pD3DFontSmallerEmbed->OnResetDevice();
	m_pD3DFontArialEmbed->OnResetDevice();
	m_pD3DSprite->OnResetDevice();
}

SIZE CFontRender::MeasureText(SIZE* size, char* szString, DWORD dwFormat)
{
	RECT rect;

	// Calculating the length of string every frame? That's not really nice
	/*if (strlen(szString) >= MAX_STRING_LENGTH)
	{
		size->cx = size->cy = 0;
		return;
	}*/

	strcpy_s(m_szBuffer, MAX_STRING_LENGTH, szString);
	
	RemoveColorEmbedsFromString(m_szBuffer);

	m_pD3DFontEmbed->DrawTextA(NULL, m_szBuffer, -1, &rect, dwFormat | DT_CALCRECT, 0xFF000000);

	size->cx = rect.right - rect.left;
	size->cy = rect.bottom - rect.top;

	return { size->cx, size->cy };
}

SIZE CFontRender::MeasureSmallerText(SIZE* size, char* szString, DWORD dwFormat)
{
	RECT rect;

	// Calculating the length of string every frame? That's not really nice
	/*if (strlen(szString) >= MAX_STRING_LENGTH)
	{
		size->cx = size->cy = 0;
		return;
	}*/

	strcpy_s(m_szBuffer, MAX_STRING_LENGTH, szString);

	RemoveColorEmbedsFromString(m_szBuffer);

	m_pD3DFontSmallerEmbed->DrawTextA(NULL, m_szBuffer, -1, &rect, dwFormat | DT_CALCRECT, 0xFF000000);

	size->cx = rect.right - rect.left;
	size->cy = rect.bottom - rect.top;

	return { size->cx, size->cy };
}

void CFontRender::RenderText(ID3DXSprite* pCustomSprite, char* szBuf, RECT rect,
	DWORD dwFormat, D3DCOLOR color, bool bShadowed)
{
	ID3DXSprite* pSprite;
	size_t nCount;
	bool bInternalSprite;

	if (szBuf && *szBuf)
	{
		bInternalSprite = false;
		pSprite = pCustomSprite;

		if (!pCustomSprite)
		{
			pSprite = m_pD3DSprite;
			bInternalSprite = true;
			pSprite->Begin(D3DXSPRITE_ALPHABLEND);
		}

		if (bShadowed)
		{
			//if (strlen(szBuf) > 100000)
				//return;

			strcpy_s(m_szBuffer, MAX_STRING_LENGTH, szBuf);

			RemoveColorEmbedsFromString(m_szBuffer);

			nCount = strlen(m_szBuffer);

			rect.top -= 1;
			m_pD3DFont->DrawTextA(pSprite, m_szBuffer, nCount, &rect, DT_NOCLIP, 0xFF000000);
			rect.top += 2;
			m_pD3DFont->DrawTextA(pSprite, m_szBuffer, nCount, &rect, DT_NOCLIP, 0xFF000000);
			rect.top -= 1;
			rect.left -= 1;
			m_pD3DFont->DrawTextA(pSprite, m_szBuffer, nCount, &rect, DT_NOCLIP, 0xFF000000);
			rect.left += 2;
			m_pD3DFont->DrawTextA(pSprite, m_szBuffer, nCount, &rect, DT_NOCLIP, 0xFF000000);
			rect.left -= 1;
		}

		m_pD3DFontEmbed->DrawTextA(pSprite, szBuf, -1, &rect, dwFormat, color);

		if (bInternalSprite)
			pSprite->End();
	}
}

void CFontRender::RenderSmallerText(ID3DXSprite* pCustomSprite, char* szBuf, RECT rect,
	DWORD dwFormat, D3DCOLOR color, bool bShadowed)
{
	ID3DXSprite* pSprite;
	size_t nCount;
	bool bInternalSprite;

	if (szBuf && *szBuf)
	{
		bInternalSprite = false;
		pSprite = pCustomSprite;
		
		if (!pCustomSprite)
		{
			pSprite = m_pD3DSprite;
			bInternalSprite = true;
			pSprite->Begin(D3DXSPRITE_ALPHABLEND);
		}
		
		if (bShadowed)
		{
			//if (strlen(szBuf) > MAX_STRING_LENGTH)
				//return;
			
			strcpy_s(m_szBuffer, MAX_STRING_LENGTH, szBuf);
			
			RemoveColorEmbedsFromString(m_szBuffer);
			
			nCount = strlen(m_szBuffer);

			rect.top -= 1;
			m_pD3DFontSmaller->DrawTextA(pSprite, m_szBuffer, nCount, &rect, dwFormat, 0xFF000000);
			rect.top += 2;
			m_pD3DFontSmaller->DrawTextA(pSprite, m_szBuffer, nCount, &rect, dwFormat, 0xFF000000);
			rect.top -= 1;
			rect.left -= 1;
			m_pD3DFontSmaller->DrawTextA(pSprite, m_szBuffer, nCount, &rect, dwFormat, 0xFF000000);
			rect.left += 2;
			m_pD3DFontSmaller->DrawTextA(pSprite, m_szBuffer, nCount, &rect, dwFormat, 0xFF000000);
			rect.left -= 1;
		}

		m_pD3DFontSmallerEmbed->DrawTextA(pSprite, szBuf, -1, &rect, dwFormat, color);

		if (bInternalSprite)
			pSprite->End();
	}
}

// Used for vehicle license plate rendering?
void CFontRender::DrawTextForPlate(char* szBuf, RECT rect, D3DCOLOR color)
{
	size_t nCount;

	if (szBuf && szBuf[0] != 0)
	{
		m_pD3DSprite->Begin(D3DXSPRITE_ALPHABLEND);

		//if (strlen(szBuf) <= MAX_STRING_LENGTH)
		{
			strcpy_s(m_szBuffer, MAX_STRING_LENGTH, szBuf);

			RemoveColorEmbedsFromString(m_szBuffer);

			nCount = strlen(m_szBuffer);

			rect.top -= 2;
			m_pD3DFontArialEmbed->DrawTextA(m_pD3DSprite, m_szBuffer, nCount, &rect,
				DT_NOCLIP | DT_VCENTER | DT_CENTER, 0xB4F0EEE4);
			rect.left += 2;
			rect.top += 2;
			m_pD3DFontArialEmbed->DrawTextA(m_pD3DSprite, m_szBuffer, nCount, &rect,
				DT_NOCLIP | DT_VCENTER | DT_CENTER, 0xB4F0EEE4);
			rect.top += 2;
			rect.left -= 2;
			m_pD3DFontArialEmbed->DrawTextA(m_pD3DSprite, m_szBuffer, nCount, &rect,
				DT_NOCLIP | DT_VCENTER | DT_CENTER, 0xB4F0EEE4);
			rect.left -= 2;
			rect.top -= 2;
			m_pD3DFontArialEmbed->DrawTextA(m_pD3DSprite, m_szBuffer, nCount, &rect,
				DT_NOCLIP | DT_VCENTER | DT_CENTER, 0xB4F0EEE4);
			rect.left += 2;

			m_pD3DFontArialEmbed->DrawTextA(m_pD3DSprite, m_szBuffer, -1, &rect,
				DT_NOCLIP | DT_VCENTER | DT_CENTER, color);

			m_pD3DSprite->End();
		}
	}
}

void CFontRender::RenderText(char* sz, RECT rect, DWORD dwColor)
{
	rect.left += 1;
	rect.top += 1;
	m_pD3DFont->DrawText(0, sz, -1, &rect, DT_NOCLIP | DT_LEFT, 0xFF000000);
	rect.left -= 1;
	rect.top -= 1;

	// the text
	m_pD3DFont->DrawTextA(0, sz, -1, &rect, DT_NOCLIP | DT_LEFT, dwColor);
}

void CFontRender::RenderText(char* sz, DWORD x, DWORD y, DWORD dwColor)
{
	RECT rect;

	rect.left = x;
	rect.top = y;
	rect.right = rect.left + 400;
	rect.bottom = rect.top + 400;

	RenderText(sz, rect, dwColor);
}

//----------------------------------------------------