
#include "main.h"

#define BASS_STREAM_CREATE_URL_FLAG \
	BASS_STREAM_BLOCK | BASS_STREAM_STATUS | BASS_STREAM_AUTOFREE

CAudioStream::CAudioStream()
{
	m_Flags.bInited = false;
	m_Flags.bPlaying = false;
	m_hStream = 0;
	m_fX = m_fY = m_fZ = 0.0f;
	m_fDistance = -1.0f;
	m_ullLastUpdate = 0;
	m_szTitle = NULL;
	m_szLabel = NULL;

	InitBass();
}

CAudioStream::~CAudioStream()
{
	BASS_Free();

	free(m_szTitle);
	free(m_szLabel);
}

void CAudioStream::InitBass()
{
	DWORD dwVolume;

	if (HIWORD(BASS_GetVersion()) != BASSVERSION ||
		!BASS_Init(-1, 44100, 0, NULL, NULL))
	{
		return;
	}

	if(pGame)
		dwVolume = DWORD(CGame::GetRadioVolume() * 8000.0f);
	else
		dwVolume = 8000;

	BASS_SetConfigPtr(BASS_CONFIG_NET_AGENT, "SA-MP/0.3");
	BASS_SetConfig(BASS_CONFIG_GVOL_STREAM, dwVolume); // 0-10000
	BASS_SetConfig(BASS_CONFIG_NET_PLAYLIST, 1);
	BASS_SetConfig(BASS_CONFIG_NET_TIMEOUT, 10000);

	if (pConfig->GetInt("audioproxyoff"))
		BASS_SetConfigPtr(BASS_CONFIG_NET_PROXY, NULL);
	
	BASS_SetEAXParameters(-1, 0.0f, -1.0f, -1.0f);

	m_Flags.bInited = true;
}

void CAudioStream::Play(char* szURL,
	float fX, float fY, float fZ, float fDist)
{
	if (!m_Flags.bInited)
		return;

	BASS_StreamFree(m_hStream);

	free(m_szTitle);
	free(m_szLabel);

	m_fX = fX;
	m_fY = fY;
	m_fZ = fZ;
	m_fDistance = fDist;

	m_hStream = BASS_StreamCreateURL(szURL, 0, BASS_STREAM_CREATE_URL_FLAG, NULL, NULL);
	if (m_hStream == 0)
		return;

	if ((m_szTitle = (char*)malloc(256)) != NULL)
		SecureZeroMemory(m_szTitle, 256);
	if ((m_szLabel = (char*)malloc(256)) != NULL)
		SecureZeroMemory(m_szLabel, 256);

	BASS_ChannelSetSync(m_hStream, BASS_SYNC_META, 0, AudioStreamMetaSync, NULL);
	BASS_ChannelSetSync(m_hStream, BASS_SYNC_OGG_CHANGE, 0, AudioStreamMetaSync, NULL);

	BASS_ChannelPlay(m_hStream, FALSE);

	if (!pConfig->GetInt("audiomsgoff"))
		if (pChatWindow)
			pChatWindow->AddInfoMessage("Audio stream: %.30s", szURL);

	m_Flags.bPlaying = true;
}

void CAudioStream::Stop()
{
	if (!m_Flags.bInited)
		return;

	BASS_StreamFree(m_hStream);
	m_hStream = 0;

	free(m_szTitle);
	m_szTitle = NULL;
	free(m_szLabel);
	m_szLabel = NULL;

	m_Flags.bPlaying = false;
}

void CAudioStream::Process()
{
	if (!m_Flags.bPlaying)
		return;

	ULONGLONG ullTickNow = GetTickCount64();
	DWORD dwVolume = 8000;

	CGame::StartRadio(-1);
	CGame::StopRadio();

	if(ullTickNow - m_ullLastUpdate >= 100)
	{
		if (pGame->IsMenuActive() || GetForegroundWindow() != pGame->GetMainWindowHwnd())
		{
			dwVolume = 0;
		}
		else if (0.0f <= m_fDistance)
		{
			CPlayerPed* pPlayerPed;
			float fDistance;

			if ((pPlayerPed = pGame->FindPlayerPed()) &&
				(fDistance = pPlayerPed->GetDistanceFromPoint(m_fX, m_fY, m_fZ)) <= m_fDistance)
			{
				dwVolume = DWORD(1.0f - fDistance / m_fDistance * (CGame::GetRadioVolume() * 8000.0f));
			}
			else
			{
				dwVolume = 0;
			}
		}
		else
		{
			dwVolume = DWORD(CGame::GetRadioVolume() * 8000.0f);
		}

		BASS_SetConfig(BASS_CONFIG_GVOL_STREAM, dwVolume);
		
		if (m_szLabel && (m_szTitle && m_szTitle[0] != 0))
		{
			const char* szTag = BASS_ChannelGetTags(m_hStream, BASS_TAG_ICY);
			if (!szTag) szTag = BASS_ChannelGetTags(m_hStream, BASS_TAG_HTTP);
			if (szTag) {
				for (; *szTag; szTag += strlen(szTag) + 1) {
					if (!_strnicmp(szTag, "icy-name:", 9))
					{
						sprintf_s(m_szLabel, 256, "%s - %s", m_szTitle, szTag + 9);
					}
				}
			}
		}

		m_ullLastUpdate = ullTickNow;
	}

	if (pDefaultFont &&
		!pGame->IsMenuActive() &&
		(m_szLabel && m_szLabel[0] != 0))
	{
		RECT rect;

		rect.left = 15;
		rect.top = pGame->GetScreenHeight() - 20;
		rect.right = pGame->GetScreenWidth();
		rect.bottom = rect.top + 30;

		pDefaultFont->RenderText(m_szLabel, rect, 0x99FFFFFF);
	}
}

void CALLBACK CAudioStream::AudioStreamMetaSync(HSYNC handle, DWORD channel, DWORD data, void* user)
{
	if (pAudioStream->m_szTitle == NULL)
		return;

	const char* meta = BASS_ChannelGetTags(pAudioStream->m_hStream, BASS_TAG_META);
	if (meta)
	{
		const char* p1 = strstr(meta, "StreamTitle='");
		if (p1)
		{
			const char* p2 = strstr(p1, "';");
			if (p2)
			{
				char* t = _strdup(p1 + 13);
				t[p2 - (p1 + 13)] = 0;
				sprintf_s(pAudioStream->m_szTitle, 256, "%s", t);
				free(t);
			}
		}
	}
	else
	{
		meta = BASS_ChannelGetTags(pAudioStream->m_hStream, BASS_TAG_OGG);
		if(meta)
		{
			const char* artist = NULL, * title = NULL, * p = meta;
			for (; *p; p += strlen(p) + 1)
			{
				if (!_strnicmp(p, "artist=", 7))
					artist = p + 7;
				if (!_strnicmp(p, "title=", 6))
					title = p + 6;
			}
			if (title)
			{
				if(artist)
					sprintf_s(pAudioStream->m_szTitle, 256, "%s - %s", artist, title);
				else
					sprintf_s(pAudioStream->m_szTitle, 256, "%s", title);
			}
		}
	}
}
