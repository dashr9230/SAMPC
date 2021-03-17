
#include "main.h"

CTextDrawSelect::CTextDrawSelect()
{
	m_pIsActive = false;
	m_pPointIs = false;
	m_pSelectID = -1;
	m_pSelectColor = 0;
}

void CTextDrawSelect::Enable(DWORD dwHoveredColor)
{
	m_pIsActive = true;
	m_pPointIs = false;
	m_pSelectID = -1;
	m_pSelectColor = (((dwHoveredColor << 16) | dwHoveredColor & 0xFF00) << 8) |
		((dwHoveredColor >> 16 | dwHoveredColor & 0xFF0000) >> 8);
}

void CTextDrawSelect::Disable()
{
	if (m_pIsActive)
	{
		pGame->DisableCamera(false);
		pGame->DisplayHud(true);

		pCursor->m_ucShowForTextDraw = false;

		if (pCmdWindow->isEnabled()) {
			pCmdWindow->Disable();
			pCmdWindow->Enable();
		}

		m_pSelectID = -1;
		m_pSelectColor = 0;
		m_pIsActive = false;
		m_pPointIs = false;

		for (int x = 0; x < (MAX_TEXT_DRAWS + MAX_PLAYER_TEXT_DRAWS); x++) {
			CTextDraw* pTextDraw = pNetGame->GetTextDrawPool()->GetAt(x);
			if (pTextDraw) {
				pTextDraw->m_TextDrawData.bColorSelect = 0;
				pTextDraw->m_TextDrawData.dwSelectColor = 0;
			}
		}
	}
}

void CTextDrawSelect::SendNotification()
{
	RakNet::BitStream bsSend;

	bsSend.Write(m_pSelectID);

	pNetGame->Send(RPC_ClickTextDraw, &bsSend);
}

void CTextDrawSelect::Process()
{
	if (m_pIsActive)
	{
		POINT Point;

		GetCursorPos(&Point);
		ScreenToClient(pGame->GetMainWindowHwnd(), &Point);

		for (int x = 0; x < (MAX_TEXT_DRAWS + MAX_PLAYER_TEXT_DRAWS); x++)
		{
			CTextDrawPool* pTextDrawPool = pNetGame->GetTextDrawPool();
			if (pTextDrawPool && pTextDrawPool->GetAt(x)) {
				CTextDraw* pTextDraw = pTextDrawPool->GetAt(x);
				if (pTextDraw && pTextDraw->m_TextDrawData.byteSelectable)
				{
					m_pSelectID = -1;

					m_pPointIs = false;

					pGame->DisableCamera(true);
					pGame->DisplayHud(false);

					pCursor->m_ucShowForTextDraw = true;

					pTextDraw->m_TextDrawData.bColorSelect = false;
					pTextDraw->m_TextDrawData.dwSelectColor = 0;

					if (PtInRect(&pTextDraw->m_TextDrawData.rSelectArea, Point))
					{
						m_pPointIs = true;

						pTextDraw->m_TextDrawData.bColorSelect = true;
						pTextDraw->m_TextDrawData.dwSelectColor = m_pSelectColor;
					}
				}
			}
		}
	}
}

void CTextDrawSelect::MsgProc(UINT uMsg, WPARAM wParam)
{
	if (m_pIsActive)
	{
		switch (uMsg)
		{
			case WM_LBUTTONUP:
			{
				if (m_pPointIs) {
					for (int x=0; x < (MAX_TEXT_DRAWS + MAX_PLAYER_TEXT_DRAWS); x++) {
						if (pNetGame->GetTextDrawPool()->GetAt(x)) {
							m_pSelectID = x;
							SendNotification();
							m_pSelectID = -1;

							break;
						}
					}
				}
			}
		}
	}
}