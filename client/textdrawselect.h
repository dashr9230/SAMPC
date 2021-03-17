
#pragma once

class CTextDrawSelect
{
private:
    int m_pSelectID;
    bool m_pIsActive;
    bool m_pPointIs;
    DWORD m_pSelectColor;

public:
    CTextDrawSelect();

    void Enable(DWORD dwHoveredColor);
    void Disable();
    void SendNotification();
    void Process();
    void MsgProc(UINT uiMsg, WPARAM wParam);
    bool IsActive() { return m_pIsActive; };
};