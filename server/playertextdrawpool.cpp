
#include "main.h"

CPlayerTextDrawPool::CPlayerTextDrawPool(unsigned char ucPlayerID)
{
	m_ucPlayerID = ucPlayerID;
	for (int i = 0; i < MAX_PLAYER_TEXT_DRAWS; i++) {
		m_pTextDraws[i] = NULL;
		m_szFontText[i] = NULL;
		m_bHasText[i] = false;
	}
}

CPlayerTextDrawPool::~CPlayerTextDrawPool()
{
	for (int i = 0; i < MAX_PLAYER_TEXT_DRAWS; i++) {
		if (m_pTextDraws[i] != NULL) {
			free(m_pTextDraws[i]);
			m_pTextDraws[i] = NULL;
		}
		if (m_szFontText[i] != NULL) {
			free(m_szFontText[i]);
			m_szFontText[i] = NULL;
		}
		m_bHasText[i] = false;
	}
}

bool CPlayerTextDrawPool::IsValid(int iID)
{
	return ((iID >= 0 && iID < MAX_PLAYER_TEXT_DRAWS) && m_pTextDraws[iID] != NULL);
}

int CPlayerTextDrawPool::New(float fX, float fY, char* szText)
{
	// Find unused slot for new textdraw
	int iSlot = 0;
	while (iSlot < MAX_PLAYER_TEXT_DRAWS) {
		if (m_pTextDraws[iSlot] == NULL) break;
		iSlot++;
	}
	if (iSlot < MAX_PLAYER_TEXT_DRAWS) {
		m_pTextDraws[iSlot] = (TEXT_DRAW_TRANSMIT*)malloc(sizeof(TEXT_DRAW_TRANSMIT));
		m_szFontText[iSlot] = (char*)malloc(MAX_TEXT_DRAW_LINE + 1);
		if (m_pTextDraws[iSlot] != NULL && m_szFontText[iSlot] != NULL) {
			strcpy_s(m_szFontText[iSlot], MAX_TEXT_DRAW_LINE, szText);
			m_szFontText[iSlot][MAX_TEXT_DRAW_LINE] = 0;

			m_pTextDraws[iSlot]->fLetterWidth = 0.48f;
			m_pTextDraws[iSlot]->fLetterHeight = 1.12f;
			m_pTextDraws[iSlot]->dwLetterColor = 0xFFE1E1E1;
			m_pTextDraws[iSlot]->byteCenter = 0;
			m_pTextDraws[iSlot]->byteBox = 0;
			m_pTextDraws[iSlot]->fLineWidth = 1280.0f;
			m_pTextDraws[iSlot]->fLineHeight = 1280.0f;
			m_pTextDraws[iSlot]->dwBoxColor = 0x80808080;
			m_pTextDraws[iSlot]->byteProportional = 1;
			m_pTextDraws[iSlot]->dwBackgroundColor = 0xFF000000;
			m_pTextDraws[iSlot]->byteShadow = 2;
			m_pTextDraws[iSlot]->byteOutline = 0;
			m_pTextDraws[iSlot]->byteLeft = 0;
			m_pTextDraws[iSlot]->byteRight = 0;
			m_pTextDraws[iSlot]->byteStyle = 1;
			m_pTextDraws[iSlot]->byteSelectable = 0;
			m_pTextDraws[iSlot]->fX = fX;
			m_pTextDraws[iSlot]->fY = fY;

			return iSlot;
		}
	}
	return INVALID_PLAYER_TEXT_DRAW;
}

void CPlayerTextDrawPool::Destroy(int iID)
{
	// Hiding the textdraw to not get stuck drawing on client until its gets replaced by a new one
	Hide(iID);

	if (m_pTextDraws[iID]) {
		free(m_pTextDraws[iID]);
		m_pTextDraws[iID] = NULL;
	}
	if (m_szFontText[iID]) {
		free(m_szFontText[iID]);
		m_szFontText[iID] = NULL;
	}
}

void CPlayerTextDrawPool::SetTextString(int iID, char* szText)
{
	if (m_szFontText[iID]) {
		strcpy_s(m_szFontText[iID], MAX_TEXT_DRAW_LINE, szText);

		RakNet::BitStream bs;
		bs.Write((unsigned short)(iID + MAX_TEXT_DRAWS));
		bs.Write(szText, MAX_TEXT_DRAW_LINE);
		if (m_bHasText[iID])
			pNetGame->SendToPlayer(m_ucPlayerID, RPC_ScrEditTextDraw, &bs);
	}
}

void CPlayerTextDrawPool::SetLetterSize(int iID, float fWidth, float fHeight)
{
	m_pTextDraws[iID]->fLetterWidth = fWidth;
	m_pTextDraws[iID]->fLetterHeight = fHeight;
}

void CPlayerTextDrawPool::SetTextSize(int iID, float fWidth, float fHeight)
{
	m_pTextDraws[iID]->fLineWidth = fWidth;
	m_pTextDraws[iID]->fLineHeight = fHeight;
}

void CPlayerTextDrawPool::SetAlignment(int iID, int iAlignment)
{
	m_pTextDraws[iID]->byteLeft = 0;
	m_pTextDraws[iID]->byteCenter = 0;
	m_pTextDraws[iID]->byteRight = 0;

	if (iAlignment == 1)
		m_pTextDraws[iID]->byteLeft = 1;
	else if (iAlignment == 2)
		m_pTextDraws[iID]->byteCenter = 1;
	else if (iAlignment == 3)
		m_pTextDraws[iID]->byteRight = 1;
}

void CPlayerTextDrawPool::SetColor(int iID, unsigned long ulColor)
{
	m_pTextDraws[iID]->dwLetterColor = RGBA_ABGR(ulColor);
}

void CPlayerTextDrawPool::SetBoxColor(int iID, unsigned long ulColor)
{
	m_pTextDraws[iID]->dwBoxColor = RGBA_ABGR(ulColor);
}

void CPlayerTextDrawPool::SetBackgroundColor(int iID, unsigned long ulColor)
{
	m_pTextDraws[iID]->dwBackgroundColor = RGBA_ABGR(ulColor);
}

void CPlayerTextDrawPool::SetUseBox(int iID, int iUse)
{
	m_pTextDraws[iID]->byteBox = (iUse != 0) ? 1 : 0;
}

void CPlayerTextDrawPool::SetShadow(int iID, unsigned char ucShadow)
{
	m_pTextDraws[iID]->byteShadow = ucShadow;
}

void CPlayerTextDrawPool::SetFont(int iID, unsigned char ucFont)
{
	m_pTextDraws[iID]->byteStyle = ucFont;
}

void CPlayerTextDrawPool::SetOutline(int iID, unsigned char ucOutline)
{
	m_pTextDraws[iID]->byteOutline = ucOutline;
}

void CPlayerTextDrawPool::SetProportional(int iID, int iProp)
{
	m_pTextDraws[iID]->byteProportional = (iProp != 0) ? 1 : 0;
}

void CPlayerTextDrawPool::SetSelectable(int iID, bool bSelect)
{
	m_pTextDraws[iID]->byteSelectable = bSelect;
}

void CPlayerTextDrawPool::Show(int iID)
{
	RakNet::BitStream bs;
	bs.Write((unsigned short)(iID + MAX_TEXT_DRAWS));
	bs.Write((PCHAR)m_pTextDraws[iID], sizeof(TEXT_DRAW_TRANSMIT));
	bs.Write(m_szFontText[iID], MAX_TEXT_DRAW_LINE);
	pNetGame->SendToPlayer(m_ucPlayerID, RPC_ScrShowTextDraw, &bs);
	m_bHasText[iID] = true;
}

void CPlayerTextDrawPool::Hide(int iID)
{
	RakNet::BitStream bs;
	bs.Write((unsigned short)(iID + MAX_TEXT_DRAWS));
	if (m_bHasText[iID])
		pNetGame->SendToPlayer(m_ucPlayerID, RPC_ScrHideTextDraw, &bs);
	m_bHasText[iID] = false;
}
