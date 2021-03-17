/*

	SA:MP Multiplayer Modification
	Copyright 2004-2006 SA:MP Team

	file:
		filterscripts.cpp
	desc:
		FilterScript Event Executive.

*/

#include "main.h"

//----------------------------------------------------------------------------------

CFilterScripts::CFilterScripts()
{
	//m_pScriptTimers = new CScriptTimers;

	m_iFilterScriptCount = 0;
	for (int i=0; i<MAX_FILTER_SCRIPTS; i++)
	{
		m_pFilterScripts[i] = NULL;
		m_szFilterScriptName[i][0] = { 0 };
	}
}

//----------------------------------------------------------------------------------

CFilterScripts::~CFilterScripts()
{
	UnloadFilterScripts();
	//SAFE_DELETE(m_pScriptTimers);
}

//----------------------------------------------------------------------------------

bool CFilterScripts::LoadFilterScript(char* pFileName)
{
	char szFilterScriptFile[255];
	sprintf(szFilterScriptFile, "filterscripts/%s.amx", pFileName);
	if (m_iFilterScriptCount >= MAX_FILTER_SCRIPTS)
		return false;

	FILE* f = fopen(&szFilterScriptFile[0], "rb");
	if (!f) return false;
	fclose(f);
	
	// Find a spare slot to load the script into
	int iSlot;
	for (iSlot = 0; iSlot < MAX_FILTER_SCRIPTS; iSlot++)
	{
		if (m_pFilterScripts[iSlot] == NULL) break;
		if (strcmp(pFileName, m_szFilterScriptName[iSlot]) == 0) return false;
	}
	if (iSlot == MAX_FILTER_SCRIPTS) return false;

	m_pFilterScripts[iSlot] = new AMX;
	AMX* amx = m_pFilterScripts[iSlot];

	memset((void*)amx, 0, sizeof(AMX));
	int err = aux_LoadProgram(amx, &szFilterScriptFile[0]);
	if (err != AMX_ERR_NONE)
	{
		logprintf("Failed to load '%s.amx' filter script.", szFilterScriptFile);
		return false;
	}

	strcpy(m_szFilterScriptName[iSlot], pFileName);

	amx_CoreInit(amx);
	amx_FloatInit(amx);
	amx_StringInit(amx);
	amx_FileInit(amx);
	amx_TimeInit(amx);
	amx_CustomInit(amx);
	amx_sampDbInit(amx);

	pPlugins->DoAmxLoad(amx);

	PrintMissingNatives(amx, szFilterScriptFile);

	int tmp;
	if (!amx_FindPublic(amx, "OnFilterScriptInit", &tmp))
		amx_Exec(amx, (cell*)&tmp, tmp);

	m_iFilterScriptCount++;

	return true;
}

//----------------------------------------------------------------------------------

bool CFilterScripts::LoadFilterScriptFromMemory(char* pFileName, char* pFileData)
{
	if (m_iFilterScriptCount >= MAX_FILTER_SCRIPTS)
		return false;

	// Find a spare slot to load the script into
	int iSlot;
	for (iSlot = 0; iSlot < MAX_FILTER_SCRIPTS; iSlot++)
	{
		if (m_pFilterScripts[iSlot] == NULL) break;
		if (strcmp(pFileName, m_szFilterScriptName[iSlot]) == 0) return false;
	}
	if (iSlot == MAX_FILTER_SCRIPTS) return false;

	m_pFilterScripts[iSlot] = new AMX;
	AMX* amx = m_pFilterScripts[iSlot];

	memset((void*)amx, 0, sizeof(AMX));
	int err = aux_LoadProgramFromMemory(amx, pFileData);
	if (err != AMX_ERR_NONE)
	{
		return false;
	}

	strcpy(m_szFilterScriptName[iSlot], pFileName);

	amx_CoreInit(amx);
	amx_FloatInit(amx);
	amx_StringInit(amx);
	amx_FileInit(amx);
	amx_TimeInit(amx);
	amx_CustomInit(amx);
	amx_sampDbInit(amx);

	pPlugins->DoAmxLoad(amx);

	int tmp;
	if (!amx_FindPublic(amx, "OnFilterScriptInit", &tmp))
		amx_Exec(amx, (cell*)&tmp, tmp);
	
	m_iFilterScriptCount++;

	return true;
}

//----------------------------------------------------------------------------------

void CFilterScripts::UnloadFilterScripts()
{
	for (int i=0; i<MAX_FILTER_SCRIPTS; i++)
	{
		if (m_pFilterScripts[i])
		{
			RemoveFilterScript(i);
		}
	}

	m_iFilterScriptCount = 0;
}

//----------------------------------------------------------------------------------
// Finds and unloads one filterscript

bool CFilterScripts::UnloadOneFilterScript(char* pFilterScript)
{
	int i;
	for (i = 0; i < MAX_FILTER_SCRIPTS; i++)
	{
		if (strcmp(pFilterScript, m_szFilterScriptName[i]) == 0) break;
	}
	if (i == MAX_FILTER_SCRIPTS) return false;
	if (m_pFilterScripts[i])
	{
		RemoveFilterScript(i);
		return true;
	}
	return false;
}

//----------------------------------------------------------------------------------
// Unloads the individual filterscript

void CFilterScripts::RemoveFilterScript(int iIndex)
{
	int tmp;
	if (!amx_FindPublic(m_pFilterScripts[iIndex], "OnFilterScriptExit", &tmp))
		amx_Exec(m_pFilterScripts[iIndex], (cell*)&tmp, tmp);

	// Kill the timers
	pNetGame->GetTimers()->DeleteForMode(m_pFilterScripts[iIndex]);
	
	// Do the other stuff from before
	aux_FreeProgram(m_pFilterScripts[iIndex]);
	pPlugins->DoAmxUnload(m_pFilterScripts[iIndex]);
	amx_sampDbCleanup(m_pFilterScripts[iIndex]);
	amx_TimeCleanup(m_pFilterScripts[iIndex]);
	amx_FileCleanup(m_pFilterScripts[iIndex]);
	amx_StringCleanup(m_pFilterScripts[iIndex]);
	amx_FloatCleanup(m_pFilterScripts[iIndex]);
	amx_CoreCleanup(m_pFilterScripts[iIndex]);
	SAFE_DELETE(m_pFilterScripts[iIndex]);
	m_szFilterScriptName[iIndex][0] = '\0';
}

char* CFilterScripts::GetFilterScriptName(AMX* amx)
{
	for (int i = 0; i < MAX_FILTER_SCRIPTS; i++)
	{
		if (m_pFilterScripts[i] == amx)
		{
			return &m_szFilterScriptName[i][0];
		}
	}
	return "";
}

//----------------------------------------------------------------------------------

void CFilterScripts::Frame(float fElapsedTime)
{
	//if (m_pScriptTimers)
		//m_pScriptTimers->Process((DWORD)(fElapsedTime * 1000.0f));
}
/*{
	if (!m_bInitialised)
		return;

	if (m_pScriptTimers)
		m_pScriptTimers->Process((DWORD)(fElapsedTime * 1000.0f));

	if (!m_bSleeping)
		return;*/

	/*if (m_fSleepTime > 0.0f)
	{
		m_fSleepTime -= fElapsedTime;
	}
	else
	{
		cell ret;
		int err = amx_Exec(&m_amx, &ret, AMX_EXEC_CONT);
		if (err == AMX_ERR_SLEEP)
		{
			m_bSleeping = true;
			m_fSleepTime = ((float)ret / 1000.0f);
		}
		else
		{
			m_bSleeping = false;
			AMXPrintError(this, &m_amx, err);
		}
	}
}*/

//----------------------------------------------------------------------------------

int CFilterScripts::CallPublic(char* szFuncName)
{
	int idx;
	cell ret = 0;

	for (int i=0; i<MAX_FILTER_SCRIPTS; i++)
	{
		if (m_pFilterScripts[i])
		{
			if (!amx_FindPublic(m_pFilterScripts[i], szFuncName, &idx))
			{
				amx_Exec(m_pFilterScripts[i], &ret, idx);
				if (!ret) return ret;
			}
		}
	}
	return (int)ret;
}

//----------------------------------------------------------------------------------

// forward OnPlayerConnect(playerid);
int CFilterScripts::OnPlayerConnect(cell playerid)
{
	int idx;
	cell ret = 0;

	for (int i=0; i<MAX_FILTER_SCRIPTS; i++)
	{
		if (m_pFilterScripts[i])
		{
			if (!amx_FindPublic(m_pFilterScripts[i], "OnPlayerConnect", &idx))
			{
				amx_Push(m_pFilterScripts[i], playerid);
				amx_Exec(m_pFilterScripts[i], &ret, idx);
				if (!ret) return ret;
			}
		}
	}
	return (int)ret;
}

//----------------------------------------------------------------------------------

// forward OnPlayerDisconnect(playerid, reason);
int CFilterScripts::OnPlayerDisconnect(cell playerid, cell reason)
{
	int idx;
	cell ret = 0;

	for (int i=0; i<MAX_FILTER_SCRIPTS; i++)
	{
		if (m_pFilterScripts && m_pFilterScripts[i])
		{
			if (!amx_FindPublic(m_pFilterScripts[i], "OnPlayerDisconnect", &idx))
			{
				amx_Push(m_pFilterScripts[i], reason);
				amx_Push(m_pFilterScripts[i], playerid);
				amx_Exec(m_pFilterScripts[i], &ret, idx);
				if (!ret) return ret;
			}
		}
	}
	return (int)ret;
}

//----------------------------------------------------------------------------------
int CFilterScripts::OnGameModeInit()
{
	int idx;
	cell ret = 0;

	for (int i=0; i<MAX_FILTER_SCRIPTS; i++)
	{
		if (m_pFilterScripts[i])
		{
			if (!amx_FindPublic(m_pFilterScripts[i], "OnGameModeInit", &idx))
			{
				//amx_Push(m_pFilterScripts[i], playerid);
				amx_Exec(m_pFilterScripts[i], &ret, idx);
				if (!ret) return ret;
			}
		}
	}
	return (int)ret;
}

//----------------------------------------------------------------------------------
int CFilterScripts::OnGameModeExit()
{
	int idx;
	cell ret = 0;

	for (int i=0; i<MAX_FILTER_SCRIPTS; i++)
	{
		if (m_pFilterScripts[i])
		{
			if (!amx_FindPublic(m_pFilterScripts[i], "OnGameModeExit", &idx))
			{
				//amx_Push(m_pFilterScripts[i], playerid);
				amx_Exec(m_pFilterScripts[i], &ret, idx);
				if (!ret) return ret;
			}
		}
	}
	return (int)ret;
}

//----------------------------------------------------------------------------------

// forward OnPlayerSpawn(playerid);
int CFilterScripts::OnPlayerSpawn(cell playerid)
{
	int idx;
	cell ret = 0;

	for (int i=0; i<MAX_FILTER_SCRIPTS; i++)
	{
		if (m_pFilterScripts[i])
		{
			if (!amx_FindPublic(m_pFilterScripts[i], "OnPlayerSpawn", &idx))
			{
				amx_Push(m_pFilterScripts[i], playerid);
				amx_Exec(m_pFilterScripts[i], &ret, idx);
				if (!ret) return ret;
			}
		}
	}
	return (int)ret;
}

//----------------------------------------------------------------------------------

// forward OnPlayerDeath(playerid, killerid, reason);
int CFilterScripts::OnPlayerDeath(cell playerid, cell killerid, cell reason)
{
	int idx;
	cell ret = 0;

	for (int i=0; i<MAX_FILTER_SCRIPTS; i++)
	{
		if (m_pFilterScripts[i])
		{
			if (!amx_FindPublic(m_pFilterScripts[i], "OnPlayerDeath", &idx))
			{
				amx_Push(m_pFilterScripts[i], reason);
				amx_Push(m_pFilterScripts[i], killerid);
				amx_Push(m_pFilterScripts[i], playerid);
				amx_Exec(m_pFilterScripts[i], &ret, idx);
				if (!ret) return ret;
			}
		}
	}
	return (int)ret;
}

//----------------------------------------------------------------------------------

// forward OnVehicleSpawn(vehicleid);
int CFilterScripts::OnVehicleSpawn(cell vehicleid)
{
	int idx;
	cell ret = 0;

	for (int i=0; i<MAX_FILTER_SCRIPTS; i++)
	{
		if (m_pFilterScripts[i])
		{
			if (!amx_FindPublic(m_pFilterScripts[i], "OnVehicleSpawn", &idx))
			{
				amx_Push(m_pFilterScripts[i], vehicleid);
				amx_Exec(m_pFilterScripts[i], &ret, idx);
				if (!ret) return ret;
			}
		}
	}
	return (int)ret;
}

//----------------------------------------------------------------------------------

// forward OnVehicleDeath(vehicleid, killerid);
int CFilterScripts::OnVehicleDeath(cell vehicleid, cell killerid)
{
	int idx;
	cell ret = 0;

	for (int i=0; i<MAX_FILTER_SCRIPTS; i++)
	{
		if (m_pFilterScripts[i])
		{
			if (!amx_FindPublic(m_pFilterScripts[i], "OnVehicleDeath", &idx))
			{
				amx_Push(m_pFilterScripts[i], killerid);
				amx_Push(m_pFilterScripts[i], vehicleid);
				amx_Exec(m_pFilterScripts[i], &ret, idx);
				//if (!ret) return ret;
			}
		}
	}
	return (int)ret;
}

//----------------------------------------------------------------------------------

// forward OnPlayerText(playerid, text[]);
int CFilterScripts::OnPlayerText(cell playerid, unsigned char* szText)
{
	int idx;
	cell ret = 1;	// DEFAULT TO 1!

	int orig_strlen = strlen((char*)szText) + 1;

	for (int i=0; i<MAX_FILTER_SCRIPTS; i++)
	{
		if (m_pFilterScripts[i])
		{
			if (!amx_FindPublic(m_pFilterScripts[i], "OnPlayerText", &idx))
			{
				cell amx_addr, *phys_addr;
				amx_PushString(m_pFilterScripts[i], &amx_addr, &phys_addr, (char*)szText, 0, 0);
				amx_Push(m_pFilterScripts[i], playerid);
				amx_Exec(m_pFilterScripts[i], &ret, idx);
				amx_GetString((char*)szText, phys_addr, 0, orig_strlen);
				amx_Release(m_pFilterScripts[i], amx_addr);
				if (!ret) return 0; // Callback returned 0, so exit and don't display the text.
			}
		}
	}
	return (int)ret;
}

//----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------

// forward OnPlayerCommandText(playerid, cmdtext[]);
int CFilterScripts::OnPlayerCommandText(cell playerid, unsigned char* szCommandText)
{
	int idx;
	cell ret = 0;

	int orig_strlen = strlen((char*)szCommandText);

	for (int i=0; i<MAX_FILTER_SCRIPTS; i++)
	{
		if (m_pFilterScripts[i])
		{
			if (!amx_FindPublic(m_pFilterScripts[i], "OnPlayerCommandText", &idx))
			{
				cell amx_addr, *phys_addr;
				amx_PushString(m_pFilterScripts[i], &amx_addr, &phys_addr, (char*)szCommandText, 0, 0);
				amx_Push(m_pFilterScripts[i], playerid);
				amx_Exec(m_pFilterScripts[i], &ret, idx);
				amx_Release(m_pFilterScripts[i], amx_addr);
				if (ret) return 1; // Callback returned 1, so the command was accepted!
			}
		}
	}
	return (int)ret;
}

//----------------------------------------------------------------------------------

// forward OnPlayerInfoChange(playerid);
int CFilterScripts::OnPlayerInfoChange(cell playerid)
{
	int idx;
	cell ret = 0;

	for (int i=0; i<MAX_FILTER_SCRIPTS; i++)
	{
		if (m_pFilterScripts[i])
		{
			if (!amx_FindPublic(m_pFilterScripts[i], "OnPlayerInfoChange", &idx))
			{
				amx_Push(m_pFilterScripts[i], playerid);
				amx_Exec(m_pFilterScripts[i], &ret, idx);
				//if (!ret) return ret;
			}
		}
	}
	return (int)ret;
}

//----------------------------------------------------------------------------------

// forward OnPlayerRequestClass(playerid, classid);
int CFilterScripts::OnPlayerRequestClass(cell playerid, cell classid)
{
	int idx;
	cell ret = 1;	// DEFAULT TO 1!

	for (int i=0; i<MAX_FILTER_SCRIPTS; i++)
	{
		if (m_pFilterScripts[i])
		{
			if (!amx_FindPublic(m_pFilterScripts[i], "OnPlayerRequestClass", &idx))
			{
				amx_Push(m_pFilterScripts[i], classid);
				amx_Push(m_pFilterScripts[i], playerid);
				amx_Exec(m_pFilterScripts[i], &ret, idx);
				//if (!ret) return ret;
			}
		}
	}
	return (int)ret;
}

//----------------------------------------------------------------------------------

// forward OnPlayerRequestSpawn(playerid);
int CFilterScripts::OnPlayerRequestSpawn(cell playerid)
{
	int idx;
	cell ret = 1;	// DEFAULT TO 1!

	for (int i=0; i<MAX_FILTER_SCRIPTS; i++)
	{
		if (m_pFilterScripts[i])
		{
			if (!amx_FindPublic(m_pFilterScripts[i], "OnPlayerRequestSpawn", &idx))
			{
				amx_Push(m_pFilterScripts[i], playerid);
				amx_Exec(m_pFilterScripts[i], &ret, idx);
				if (!ret) return ret;
			}
		}
	}
	return (int)ret;
}

//----------------------------------------------------------------------------------

// forward OnPlayerEnterVehicle(playerid, vehicleid, ispassenger);
int CFilterScripts::OnPlayerEnterVehicle(cell playerid, cell vehicleid, cell ispassenger)
{
	int idx;
	cell ret = 0;

	for (int i=0; i<MAX_FILTER_SCRIPTS; i++)
	{
		if (m_pFilterScripts[i])
		{
			if (!amx_FindPublic(m_pFilterScripts[i], "OnPlayerEnterVehicle", &idx))
			{
				amx_Push(m_pFilterScripts[i], ispassenger);
				amx_Push(m_pFilterScripts[i], vehicleid);
				amx_Push(m_pFilterScripts[i], playerid);
				amx_Exec(m_pFilterScripts[i], &ret, idx);
				//if (!ret) return ret;
			}
		}
	}
	return (int)ret;
}

//----------------------------------------------------------------------------------

// forward OnPlayerExitVehicle(playerid, vehicleid);
int CFilterScripts::OnPlayerExitVehicle(cell playerid, cell vehicleid)
{
	int idx;
	cell ret = 0;

	for (int i=0; i<MAX_FILTER_SCRIPTS; i++)
	{
		if (m_pFilterScripts[i])
		{
			if (!amx_FindPublic(m_pFilterScripts[i], "OnPlayerExitVehicle", &idx))
			{
				amx_Push(m_pFilterScripts[i], vehicleid);
				amx_Push(m_pFilterScripts[i], playerid);
				amx_Exec(m_pFilterScripts[i], &ret, idx);
				//if (!ret) return ret;
			}
		}
	}
	return (int)ret;
}

//----------------------------------------------------------------------------------

// forward OnPlayerStateChange(playerid, newstate, oldstate);
int CFilterScripts::OnPlayerStateChange(cell playerid, cell newstate, cell oldstate)
{
	int idx;
	cell ret = 0;

	for (int i=0; i<MAX_FILTER_SCRIPTS; i++)
	{
		if (m_pFilterScripts[i])
		{
			if (!amx_FindPublic(m_pFilterScripts[i], "OnPlayerStateChange", &idx))
			{
				amx_Push(m_pFilterScripts[i], oldstate);
				amx_Push(m_pFilterScripts[i], newstate);
				amx_Push(m_pFilterScripts[i], playerid);
				amx_Exec(m_pFilterScripts[i], &ret, idx);
				//if (!ret) return ret;
			}
		}
	}
	return (int)ret;
}

//----------------------------------------------------------------------------------

// forward OnPlayerInteriorChange(playerid, newinteriorid, oldinteriorid);

int CFilterScripts::OnPlayerInteriorChange(cell playerid, cell newid, cell oldid)
{
	int idx;
	cell ret = 0;

	for (int i=0; i<MAX_FILTER_SCRIPTS; i++)
	{
		if (m_pFilterScripts[i])
		{
			if (!amx_FindPublic(m_pFilterScripts[i], "OnPlayerInteriorChange", &idx))
			{
				amx_Push(m_pFilterScripts[i], oldid);
				amx_Push(m_pFilterScripts[i], newid);
				amx_Push(m_pFilterScripts[i], playerid);
				amx_Exec(m_pFilterScripts[i], &ret, idx);
				//if (!ret) return ret;
			}
		}
	}
	return (int)ret;
}

//----------------------------------------------------------------------------------

// forward OnPlayerEnterCheckpoint(playerid);
int CFilterScripts::OnPlayerEnterCheckpoint(cell playerid)
{
	int idx;
	cell ret = 0;

	for (int i=0; i<MAX_FILTER_SCRIPTS; i++)
	{
		if (m_pFilterScripts[i])
		{
			if (!amx_FindPublic(m_pFilterScripts[i], "OnPlayerEnterCheckpoint", &idx))
			{
				amx_Push(m_pFilterScripts[i], playerid);
				amx_Exec(m_pFilterScripts[i], &ret, idx);
				//if (!ret) return ret;
			}
		}
	}
	return (int)ret;
}

//----------------------------------------------------------------------------------

// forward OnPlayerLeaveCheckpoint(playerid);
int CFilterScripts::OnPlayerLeaveCheckpoint(cell playerid)
{
	int idx;
	cell ret = 0;

	for (int i=0; i<MAX_FILTER_SCRIPTS; i++)
	{
		if (m_pFilterScripts[i])
		{
			if (!amx_FindPublic(m_pFilterScripts[i], "OnPlayerLeaveCheckpoint", &idx))
			{
				amx_Push(m_pFilterScripts[i], playerid);
				amx_Exec(m_pFilterScripts[i], &ret, idx);
				//if (!ret) return ret;
			}
		}
	}
	return (int)ret;
}

//----------------------------------------------------------------------------------

// forward OnPlayerEnterRaceCheckpoint(playerid);
int CFilterScripts::OnPlayerEnterRaceCheckpoint(cell playerid)
{
	int idx;
	cell ret = 0;

	for (int i=0; i<MAX_FILTER_SCRIPTS; i++)
	{
		if (m_pFilterScripts[i])
		{
			if (!amx_FindPublic(m_pFilterScripts[i], "OnPlayerEnterRaceCheckpoint", &idx))
			{
				amx_Push(m_pFilterScripts[i], playerid);
				amx_Exec(m_pFilterScripts[i], &ret, idx);
				//if (!ret) return ret;
			}
		}
	}
	return (int)ret;
}

//----------------------------------------------------------------------------------

// forward OnPlayerLeaveRaceCheckpoint(playerid);
int CFilterScripts::OnPlayerLeaveRaceCheckpoint(cell playerid)
{
	int idx;
	cell ret = 0;

	for (int i=0; i<MAX_FILTER_SCRIPTS; i++)
	{
		if (m_pFilterScripts[i])
		{
			if (!amx_FindPublic(m_pFilterScripts[i], "OnPlayerLeaveRaceCheckpoint", &idx))
			{
				amx_Push(m_pFilterScripts[i], playerid);
				amx_Exec(m_pFilterScripts[i], &ret, idx);
				//if (!ret) return ret;
			}
		}
	}
	return (int)ret;
}

//----------------------------------------------------------------------------------

// forward OnPlayerKeyStateChange(playerid,newkeys,oldkeys);
int CFilterScripts::OnPlayerKeyStateChange(cell playerid, cell newkeys, cell oldkeys)
{
	int idx;
	cell ret = 0;

	for (int i=0; i<MAX_FILTER_SCRIPTS; i++)
	{
		if (m_pFilterScripts[i])
		{
			if (!amx_FindPublic(m_pFilterScripts[i], "OnPlayerKeyStateChange", &idx))
			{
				amx_Push(m_pFilterScripts[i], oldkeys);
				amx_Push(m_pFilterScripts[i], newkeys);
				amx_Push(m_pFilterScripts[i], playerid);
				amx_Exec(m_pFilterScripts[i], &ret, idx);
			}
		}
	}

	return (int)ret;
}

//----------------------------------------------------------------------------------

// forward OnPlayerRequestClass(playerid, classid);
int CFilterScripts::OnRconCommand(char* szCommand)
{
	int idx;
	cell ret = 1;

	for (int i=0; i<MAX_FILTER_SCRIPTS; i++)
	{
		if (m_pFilterScripts[i])
		{
			if (!amx_FindPublic(m_pFilterScripts[i], "OnRconCommand", &idx))
			{
				cell amx_addr, *phys_addr;
				amx_PushString(m_pFilterScripts[i], &amx_addr, &phys_addr, szCommand, 0, 0);
				amx_Exec(m_pFilterScripts[i], &ret, idx);
				//if (!ret) return ret;
			} 
		}
	}
	return (int)ret;
}


//----------------------------------------------------------------------------------

// forward OnObjectMoved(objectid);
int CFilterScripts::OnObjectMoved(cell objectid)
{
	int idx;
	cell ret = 0;

	for (int i=0; i<MAX_FILTER_SCRIPTS; i++)
	{
		if (m_pFilterScripts[i])
		{
			if (!amx_FindPublic(m_pFilterScripts[i], "OnObjectMoved", &idx))
			{
				amx_Push(m_pFilterScripts[i], objectid);
				amx_Exec(m_pFilterScripts[i], &ret, idx);
			}
		}
	}
	
	return (int)ret;
}

//----------------------------------------------------------------------------------

// forward OnPlayerObjectMoved(playerid, objectid);
int CFilterScripts::OnPlayerObjectMoved(cell playerid, cell objectid)
{
	int idx;
	cell ret = 0;

	for (int i=0; i<MAX_FILTER_SCRIPTS; i++)
	{
		if (m_pFilterScripts[i])
		{
			if (!amx_FindPublic(m_pFilterScripts[i], "OnPlayerObjectMoved", &idx))
			{
				amx_Push(m_pFilterScripts[i], objectid);
				amx_Push(m_pFilterScripts[i], playerid);
				amx_Exec(m_pFilterScripts[i], &ret, idx);
			}
		}
	}

	return (int)ret;
}

//----------------------------------------------------------------------------------

// forward OnPlayerPickedUpPickup(playerid, pickupid);
int CFilterScripts::OnPlayerPickedUpPickup(cell playerid, cell pickupid)
{
	int idx;
	cell ret = 0;

	for (int i=0; i<MAX_FILTER_SCRIPTS; i++)
	{
		if (m_pFilterScripts[i])
		{
			if (!amx_FindPublic(m_pFilterScripts[i], "OnPlayerPickUpPickup", &idx))
			{
				amx_Push(m_pFilterScripts[i], pickupid);
				amx_Push(m_pFilterScripts[i], playerid);
				amx_Exec(m_pFilterScripts[i], &ret, idx);
			}
		}
	}
	return (int)ret;
}

//----------------------------------------------------------------------------------

// forward OnPlayerExitedMenu(playerid);
int CFilterScripts::OnPlayerExitedMenu(cell playerid)
{
	int idx;
	cell ret = 0;

	for (int i=0; i<MAX_FILTER_SCRIPTS; i++)
	{
		if (m_pFilterScripts[i])
		{
			if (!amx_FindPublic(m_pFilterScripts[i], "OnPlayerExitedMenu", &idx))
			{
				amx_Push(m_pFilterScripts[i], playerid);
				amx_Exec(m_pFilterScripts[i], &ret, idx);
			}
		}
	}
	return (int)ret;
}

//----------------------------------------------------------------------------------

// forward OnPlayerSelectedMenuRow(playerid, row);
int CFilterScripts::OnPlayerSelectedMenuRow(cell playerid, cell row)
{
	int idx;
	cell ret = 0;

	for (int i=0; i<MAX_FILTER_SCRIPTS; i++)
	{
		if (m_pFilterScripts[i])
		{
			if (!amx_FindPublic(m_pFilterScripts[i], "OnPlayerSelectedMenuRow", &idx))
			{
				amx_Push(m_pFilterScripts[i], row);
				amx_Push(m_pFilterScripts[i], playerid);
				amx_Exec(m_pFilterScripts[i], &ret, idx);
			}
		}
	}
	return (int)ret;
}

//----------------------------------------------------------------------------------

// forward OnVehicleRespray(playerid, vehicleid, color1, color2);
int CFilterScripts::OnVehicleRespray(cell playerid, cell vehicleid, cell color1, cell color2)
{
	int idx;
	cell ret = 1;

	for (int i=0; i<MAX_FILTER_SCRIPTS; i++)
	{
		if (m_pFilterScripts[i])
		{
			if (!amx_FindPublic(m_pFilterScripts[i], "OnVehicleRespray", &idx))
			{
				amx_Push(m_pFilterScripts[i], color2);
				amx_Push(m_pFilterScripts[i], color1);
				amx_Push(m_pFilterScripts[i], vehicleid);
				amx_Push(m_pFilterScripts[i], playerid);
				amx_Exec(m_pFilterScripts[i], &ret, idx);
			}
		}
	}
	return (int)ret;
}

//----------------------------------------------------------------------------------

// forward OnVehicleMod(playerid, vehicleid, componentid);
int CFilterScripts::OnVehicleMod(cell playerid, cell vehicleid, cell componentid)
{
	int idx;
	cell ret = 1;
	int retval = 1;

	for (int i=0; i<MAX_FILTER_SCRIPTS; i++)
	{
		if (m_pFilterScripts[i])
		{
			if (!amx_FindPublic(m_pFilterScripts[i], "OnVehicleMod", &idx))
			{
				amx_Push(m_pFilterScripts[i], componentid);
				amx_Push(m_pFilterScripts[i], vehicleid);
				amx_Push(m_pFilterScripts[i], playerid);
				amx_Exec(m_pFilterScripts[i], &ret, idx);
				if (!ret) retval = 0;
			}
		}
	}
	return retval;
}

// forward OnEnterExitModShop(playerid, enterexit, interiorid);
int CFilterScripts::OnEnterExitModShop(cell playerid, cell enterexit, cell interiorid)
{
	int idx;
	cell ret = 1;
	int retval = 1;

	for (int i = 0; i < MAX_FILTER_SCRIPTS; i++)
	{
		if (m_pFilterScripts[i])
		{
			if (!amx_FindPublic(m_pFilterScripts[i], "OnEnterExitModShop", &idx))
			{
				amx_Push(m_pFilterScripts[i], interiorid);
				amx_Push(m_pFilterScripts[i], enterexit);
				amx_Push(m_pFilterScripts[i], playerid);
				amx_Exec(m_pFilterScripts[i], &ret, idx);
				if (!ret) retval = 0;
			}
		}
	}
	return retval;
}

// forward OnVehiclePaintjob(playerid, vehicleid, paintjobid);
int CFilterScripts::OnVehiclePaintjob(cell playerid, cell vehicleid, cell paintjobid)
{
	int idx;
	cell ret = 1;

	for (int i=0; i<MAX_FILTER_SCRIPTS; i++)
	{
		if (m_pFilterScripts[i])
		{
			if (!amx_FindPublic(m_pFilterScripts[i], "OnVehiclePaintjob", &idx))
			{
				amx_Push(m_pFilterScripts[i], paintjobid);
				amx_Push(m_pFilterScripts[i], vehicleid);
				amx_Push(m_pFilterScripts[i], playerid);
				amx_Exec(m_pFilterScripts[i], &ret, idx);
			}
		}
	}
	return (int)ret;
}

//----------------------------------------------------------------------------------
// forward OnPlayerUpdate(playerid)

int CFilterScripts::OnPlayerUpdate(cell playerid)
{
	int idx;
	cell ret = 1;

	for (int i=0; i<MAX_FILTER_SCRIPTS; i++) {
		if (m_pFilterScripts[i]) 
		{
			if(!amx_FindPublic(m_pFilterScripts[i], "OnPlayerUpdate", &idx))
			{
				amx_Push(m_pFilterScripts[i], playerid);
				amx_Exec(m_pFilterScripts[i], &ret, idx);
			}
		}
	}

	return (int)ret;
}

int CFilterScripts::OnIncomingConnection(cell playerid, const char* ip, cell port)
{
	int idx;
	cell ret = 1;
	for (int i = 0; i < MAX_FILTER_SCRIPTS; i++) {
		if (m_pFilterScripts[i])
		{
			if (!amx_FindPublic(m_pFilterScripts[i], "OnIncomingConnection", &idx))
			{
				cell amx_addr, * phys_addr;
				amx_Push(m_pFilterScripts[i], port);
				amx_PushString(m_pFilterScripts[i], &amx_addr, &phys_addr, ip, 0, 0);
				amx_Push(m_pFilterScripts[i], playerid);
				amx_Exec(m_pFilterScripts[i], &ret, idx);
				amx_Release(m_pFilterScripts[i], amx_addr);
				if (!ret) return ret;
			}
		}
	}
	return (int)ret;
}

//----------------------------------------------------------------------------------

int CFilterScripts::OnPlayerClickMap(cell playerid, float fX, float fY, float fZ)
{
	int idx;
	cell ret = 1;
	for (int i = 0; i < MAX_FILTER_SCRIPTS; i++) {
		if (m_pFilterScripts[i])
		{
			if (!amx_FindPublic(m_pFilterScripts[i], "OnPlayerClickMap", &idx))
			{
				amx_Push(m_pFilterScripts[i], amx_ftoc(fZ));
				amx_Push(m_pFilterScripts[i], amx_ftoc(fY));
				amx_Push(m_pFilterScripts[i], amx_ftoc(fX));
				amx_Push(m_pFilterScripts[i], playerid);
				amx_Exec(m_pFilterScripts[i], &ret, idx);
				if (!ret) return ret;
			}
		}
	}
	return (int)ret;
}

int CFilterScripts::OnTrailerUpdate(cell playerid, cell vehicleid)
{
	int idx;
	cell ret = 1;
	for (int i = 0; i < MAX_FILTER_SCRIPTS; i++) {
		if (m_pFilterScripts[i])
		{
			if (!amx_FindPublic(m_pFilterScripts[i], "OnTrailerUpdate", &idx))
			{
				amx_Push(m_pFilterScripts[i], vehicleid);
				amx_Push(m_pFilterScripts[i], playerid);
				amx_Exec(m_pFilterScripts[i], &ret, idx);
			}
		}
	}
	return (int)ret;
}

int CFilterScripts::OnRconLoginAttempt(char* szIP, char* szPassword, cell success)
{
	int idx;
	cell ret = 1;

	for (int i = 0; i < MAX_FILTER_SCRIPTS; i++)
	{
		if (!m_pFilterScripts[i])
			continue;

		if (!amx_FindPublic(m_pFilterScripts[i], "OnRconLoginAttempt", &idx))
		{
			cell amx_addr1, amx_addr2, * phys_addr;
			amx_Push(m_pFilterScripts[i], success);
			amx_PushString(m_pFilterScripts[i], &amx_addr2, &phys_addr, szPassword, 0, 0);
			amx_PushString(m_pFilterScripts[i], &amx_addr1, &phys_addr, szIP, 0, 0);
			amx_Exec(m_pFilterScripts[i], &ret, idx);
			amx_Release(m_pFilterScripts[i], amx_addr1);
			amx_Release(m_pFilterScripts[i], amx_addr2);
		}
	}
	return (int)ret;
}

void CFilterScripts::OnPlayerBeginTyping(cell playerid)
{
	int idx = 0;
	for (int i = 0; i < MAX_FILTER_SCRIPTS; i++)
	{
		if (m_pFilterScripts[i] == NULL)
			continue;

		if (!amx_FindPublic(m_pFilterScripts[i], "OnPlayerBeginTyping", &idx))
		{
			amx_Push(m_pFilterScripts[i], playerid);
			amx_Exec(m_pFilterScripts[i], NULL, idx);
		}
	}
}

void CFilterScripts::OnPlayerEndTyping(cell playerid)
{
	int idx = 0;
	for (int i = 0; i < MAX_FILTER_SCRIPTS; i++)
	{
		if (m_pFilterScripts[i] == NULL)
			continue;

		if (!amx_FindPublic(m_pFilterScripts[i], "OnPlayerEndTyping", &idx))
		{
			amx_Push(m_pFilterScripts[i], playerid);
			amx_Exec(m_pFilterScripts[i], NULL, idx);
		}
	}
}

int CFilterScripts::OnPlayerStunt(cell playerid, cell vehicleid)
{
	int idx = 0;
	for (int i = 0; i < MAX_FILTER_SCRIPTS; i++)
	{
		if (m_pFilterScripts[i] == NULL)
			continue;

		if (!amx_FindPublic(m_pFilterScripts[i], "OnPlayerStunt", &idx))
		{
			amx_Push(m_pFilterScripts[i], vehicleid);
			amx_Push(m_pFilterScripts[i], playerid);
			amx_Exec(m_pFilterScripts[i], NULL, idx);
		}
	}
	return 1;
}

void CFilterScripts::OnClientCheckResponse(cell playerid, cell type, cell address, cell checksum)
{
	int idx = 0;
	for (unsigned i = 0; i < MAX_FILTER_SCRIPTS; i++)
	{
		if (m_pFilterScripts[i] == NULL)
			continue;

		if (!amx_FindPublic(m_pFilterScripts[i], "OnClientCheckResponse", &idx))
		{
			amx_Push(m_pFilterScripts[i], checksum);
			amx_Push(m_pFilterScripts[i], address);
			amx_Push(m_pFilterScripts[i], type);
			amx_Push(m_pFilterScripts[i], playerid);
			amx_Exec(m_pFilterScripts[i], NULL, idx);
		}
	}
}

int CFilterScripts::OnVehicleSirenStateChange(cell playerid, cell vehicleid, cell newstate)
{
	int idx = 0;
	cell ret = 0;
	for (char i = 0; i < MAX_FILTER_SCRIPTS; i++) {
		if (m_pFilterScripts[i] != NULL) {
			if (!amx_FindPublic(m_pFilterScripts[i], "OnVehicleSirenStateChange", &idx)) {
				amx_Push(m_pFilterScripts[i], newstate);
				amx_Push(m_pFilterScripts[i], vehicleid);
				amx_Push(m_pFilterScripts[i], playerid);
				amx_Exec(m_pFilterScripts[i], &ret, idx);
				if (ret) return 1;
			}
		}
	}
	return ret;
}

void CFilterScripts::OnVehicleDamageStatusUpdate(cell vehicleid, cell playerid)
{
	int idx = 0;
	for (char i = 0; i < MAX_FILTER_SCRIPTS; i++)
	{
		if (m_pFilterScripts[i] == NULL)
			continue;

		if (!amx_FindPublic(m_pFilterScripts[i], "OnVehicleDamageStatusUpdate", &idx))
		{
			amx_Push(m_pFilterScripts[i], playerid);
			amx_Push(m_pFilterScripts[i], vehicleid);
			amx_Exec(m_pFilterScripts[i], NULL, idx);
		}
	}
}

void CFilterScripts::OnActorStreamIn(cell actorid, cell forplayerid)
{
	int idx = 0;

	for (char i = 0; i < MAX_FILTER_SCRIPTS; i++)
	{
		if (m_pFilterScripts[i] == NULL)
			continue;

		if (!amx_FindPublic(m_pFilterScripts[i], "OnActorStreamIn", &idx))
		{
			amx_Push(m_pFilterScripts[i], forplayerid);
			amx_Push(m_pFilterScripts[i], actorid);
			amx_Exec(m_pFilterScripts[i], NULL, idx);
		}
	}
}

void CFilterScripts::OnActorStreamOut(cell actorid, cell forplayerid)
{
	int idx = 0;

	for (char i = 0; i < MAX_FILTER_SCRIPTS; i++)
	{
		if (m_pFilterScripts[i] == NULL)
			continue;

		if (!amx_FindPublic(m_pFilterScripts[i], "OnActorStreamOut", &idx))
		{
			amx_Push(m_pFilterScripts[i], forplayerid);
			amx_Push(m_pFilterScripts[i], actorid);
			amx_Exec(m_pFilterScripts[i], NULL, idx);
		}
	}
}

//----------------------------------------------------------------------------------

void CFilterScripts::OnPlayerGiveDamageActor(cell playerid, cell actorid,
	float fDamage, cell weaponid, cell bodypart)
{
	int idx;

	for (int i = 0; i < MAX_FILTER_SCRIPTS; i++)
	{
		if (m_pFilterScripts[i])
		{
			if (amx_FindPublic(m_pFilterScripts[i], "OnPlayerGiveDamageActor", &idx) == AMX_ERR_NONE)
			{
				amx_Push(m_pFilterScripts[i], bodypart);
				amx_Push(m_pFilterScripts[i], weaponid);
				amx_Push(m_pFilterScripts[i], amx_ftoc(fDamage));
				amx_Push(m_pFilterScripts[i], actorid);
				amx_Push(m_pFilterScripts[i], playerid);
				amx_Exec(m_pFilterScripts[i], NULL, idx);
			}
		}
	}
}

//----------------------------------------------------------------------------------

void CFilterScripts::OnPlayerClickTextDraw(cell playerid, cell clickedid)
{
	int idx = 0;

	for (char i = 0; i < MAX_FILTER_SCRIPTS; i++)
	{
		if (m_pFilterScripts[i] == NULL)
			continue;

		if (!amx_FindPublic(m_pFilterScripts[i], "OnPlayerClickTextDraw", &idx))
		{
			amx_Push(m_pFilterScripts[i], clickedid);
			amx_Push(m_pFilterScripts[i], playerid);
			amx_Exec(m_pFilterScripts[i], NULL, idx);
		}
	}
}

void CFilterScripts::OnPlayerClickPlayerTextDraw(cell playerid, cell playertextid)
{
	int idx = 0;

	for (char i = 0; i < MAX_FILTER_SCRIPTS; i++)
	{
		if (m_pFilterScripts[i] == NULL)
			continue;

		if (!amx_FindPublic(m_pFilterScripts[i], "OnPlayerClickPlayerTextDraw", &idx))
		{
			amx_Push(m_pFilterScripts[i], playertextid);
			amx_Push(m_pFilterScripts[i], playerid);
			amx_Exec(m_pFilterScripts[i], NULL, idx);
		}
	}
}
