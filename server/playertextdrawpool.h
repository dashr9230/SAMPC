
#ifndef _PLAYERTEXTDRAWPOOL_H
#define _PLAYERTEXTDRAWPOOL_H

#define RGBA_ABGR(n) (((n >> 24) & 0x000000FF) | ((n >> 8) & 0x0000FF00) | ((n << 8) & 0x00FF0000) | ((n << 24) & 0xFF000000))

class CPlayerTextDrawPool
{
private:
	unsigned char m_ucPlayerID;
	TEXT_DRAW_TRANSMIT* m_pTextDraws[MAX_PLAYER_TEXT_DRAWS];
	char* m_szFontText[MAX_PLAYER_TEXT_DRAWS];
	bool m_bHasText[MAX_PLAYER_TEXT_DRAWS];

public:
	CPlayerTextDrawPool(unsigned char ucPlayerID);
	~CPlayerTextDrawPool();

	bool IsValid(int iID);

	int New(float fX, float fY, char* szText);
	void Destroy(int iID);
	void SetTextString(int iID, char* szText);
	void SetLetterSize(int iID, float fWidth, float fHeight);
	void SetTextSize(int iID, float fWidth, float fHeight);
	void SetAlignment(int iID, int iAlignment);
	void SetColor(int iID, unsigned long ulColor);
	void SetBoxColor(int iID, unsigned long ulColor);
	void SetBackgroundColor(int iID, unsigned long ulColor);
	void SetUseBox(int iID, int iUse);
	void SetShadow(int iID, unsigned char ucShadow);
	void SetFont(int iID, unsigned char ucFont);
	void SetOutline(int iID, unsigned char ucOutline);
	void SetProportional(int iID, int iProp);
	void SetSelectable(int iID, bool bSelect);
	void Show(int iID);
	void Hide(int iID);
};

#endif // _PLAYERTEXTDRAWPOOL_H
