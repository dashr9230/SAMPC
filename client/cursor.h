
#pragma once

class CCursor
{
public:
	CCursor();
	//~CCursor();

	void Init();
	void DeleteDeviceObjects();
	void RestoreDeviceObjects();
	void Process();

	union {
		unsigned char m_ucVisible;
		struct {
			unsigned char m_ucShowForChatbox : 1;
			unsigned char m_ucShowForSpawnScreen : 1;
			unsigned char m_ucShowForTextDraw : 1;
		};
	};

	IDirect3DTexture9* m_pTexture;
	//ID3DXFont* m_pFont;
	ID3DXSprite* m_pSprite;
	int m_iSize;
};
