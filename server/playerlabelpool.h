
#ifndef SAMPSRV_PLAYERLABELPOOL_H
#define SAMPSRV_PLAYERLABELPOOL_H

typedef struct
{
	char* szText;
	WORD wLength;
	DWORD dwColor;
	float fX;
	float fY;
	float fZ;
	float fDrawDistance;
	bool bTestLOS;
	int iVirtualWorld;
	WORD wAttachedPlayerID;
	WORD wAttachedVehicleID;
} LABEL;

class CPlayerLabelPool
{
private:
	LABEL m_Labels[MAX_LABEL_PLAYER];
	bool m_bSlotState[MAX_LABEL_PLAYER];
	//BYTE m_byte37888[MAX_LABEL_PLAYER];
	WORD m_wPlayerID;

	void InitForPlayer(WORD wLabelID);
	void DeleteForPlayer(WORD wLabelID);

public:
	CPlayerLabelPool();
	~CPlayerLabelPool();

	bool GetSlotState(int iLabelID)
	{
		if (iLabelID >= 0 && iLabelID < MAX_LABEL_PLAYER)
		{
			return m_bSlotState[iLabelID];
		}
		return false;
	}

	WORD New(char* szText, DWORD dwColor, float fX, float fY, float fZ,
		float fDistance, WORD wAttachedPlayerID, WORD wAttachedVehicleID, bool bTestLOS);
	bool Delete(WORD wLabelID);
	bool UpdateText(WORD wLabelID, DWORD dwColor, char* szText);

	void SetPlayerID(WORD wPlayerID) { m_wPlayerID = wPlayerID; };
};

#endif // SAMPSRV_PLAYERLABELPOOL_H
