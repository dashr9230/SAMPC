//
// Version: $Id: netstats.cpp,v 1.4 2006/04/15 22:43:03 kyeman Exp $
//

#include "main.h"
#include <stdio.h>

static char szDispBuf[16384];
static char szStatBuf[16384];
static char szDrawLine[1024];

CNetStats::CNetStats(IDirect3DDevice9 *pD3DDevice)
{
	m_dwLastUpdateTick = GetTickCount();
	m_dwLastTotalBytesRecv = 0;
	m_dwLastTotalBytesSent = 0;
	m_dwBPSDownload = 0;
	m_dwBPSUpload = 0;
	m_pD3DDevice = pD3DDevice;
}

void CNetStats::Draw()
{
	RakNetStatisticsStruct *pRakStats = pNetGame->GetRakClient()->GetStatistics();
	float fDown,fUp;

	if((GetTickCount() - m_dwLastUpdateTick) > 1000) {
		m_dwLastUpdateTick = GetTickCount();
		
		m_dwBPSDownload = ((UINT)(pRakStats->bitsReceived / 8)) - m_dwLastTotalBytesRecv;
		m_dwLastTotalBytesRecv = (UINT)(pRakStats->bitsReceived / 8);

		m_dwBPSUpload = ((UINT)(pRakStats->totalBitsSent / 8)) - m_dwLastTotalBytesSent;
		m_dwLastTotalBytesSent = (UINT)(pRakStats->totalBitsSent / 8);
	}

	if(m_dwBPSDownload != 0) {
		fDown = (float)m_dwBPSDownload / 1024;
	} else {
		fDown = 0.0f;
	}

	if(m_dwBPSUpload != 0) {
		fUp = (float)m_dwBPSUpload / 1024;
	} else {
		fUp = 0.0f;
	}

	CLocalPlayer *pLocalPlayer = pNetGame->GetPlayerPool()->GetLocalPlayer();
	CPlayerPed *pPlayerPed = pLocalPlayer->GetPlayerPed();
	int iPlayersInRange = pLocalPlayer->DetermineNumberOfPlayersInLocalRange();
	if(!iPlayersInRange) iPlayersInRange = 20;

	sprintf_s(szDispBuf,"Download Rate: %.2f KB/s\nUpload Rate: %.2f KB/s\n",
		fDown,fUp);

	if(pPlayerPed) {
		if(!pPlayerPed->IsInVehicle()) {
			sprintf_s(szStatBuf,"OnFoot Send Rate: %u\n",pLocalPlayer->GetOptimumOnFootSendRate(iPlayersInRange));
		} else {
			sprintf_s(szStatBuf,"InCar Send Rate: %u\n",pLocalPlayer->GetOptimumInCarSendRate(iPlayersInRange));
		}
		strcat_s(szDispBuf,szStatBuf);
	}

	if (pGame)
	{
		extern DWORD dwTotalSystemMemoryInMB;
		DWORD dwStreamingMemory = pGame->GetUsedStreamingMemory();
		DWORD dwTotal = pGame->GetStreamingMemory();
		if (dwStreamingMemory && dwTotal)
		{
			sprintf_s(szStatBuf, "Streaming Mem: %uMB Total: %uMB System: %uMB\n",
				dwStreamingMemory >> 20,
				dwTotal >> 20,
				dwTotalSystemMemoryInMB);
		}
		strcat_s(szDispBuf, szStatBuf);
	}

	PROCESS_MEMORY_COUNTERS psMemCounter;

	SecureZeroMemory(&psMemCounter, sizeof(PROCESS_MEMORY_COUNTERS));

	if (GetProcessMemoryInfo(GetCurrentProcess(), &psMemCounter, sizeof(psMemCounter)))
	{
		sprintf_s(szStatBuf, "Process Mem: %uKB Working Set: %uKB\n",
			psMemCounter.PagefileUsage >> 10,
			psMemCounter.WorkingSetSize >> 10);

		strcat_s(szDispBuf, szStatBuf);
	}

	StatisticsToString(pRakStats, szStatBuf, 4);

	strcat_s(szDispBuf,szStatBuf);

	//m_pD3DDevice->GetDisplayMode(0,&dDisplayMode);

	SIZE size;
	pDefaultFont->MeasureText(&size, szDispBuf, DT_LEFT);

	RECT rect;
	rect.top		= 10;
	rect.right		= pGame->GetScreenWidth() - 150;
	rect.left		= 10;
	rect.bottom		= rect.top + 16;

	PCHAR pBuf = szDispBuf;
	
	int x=0;

	pDefaultFont->RenderText("Client Network Stats", rect, 0xFF8888EE);
	rect.top += 16;
	rect.bottom += 16;

	while(*pBuf) {
		szDrawLine[x] = *pBuf;
		if(szDrawLine[x] == '\n') {
			szDrawLine[x] = '\0';
			pDefaultFont->RenderText(szDrawLine,rect,0xFFFFFFFF);
			rect.top += 16;
			rect.bottom += 16;
			x=0;
		} else {
			x++;
		}
		pBuf++;
	}
}
