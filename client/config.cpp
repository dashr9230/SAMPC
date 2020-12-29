
#include "main.h"

CConfig::CConfig()
{
	GetCurrentDirectory(250, Directory);
	sprintf_s(Path, 260, "%s\\samp.cfg", Directory);
}

void CConfig::SetValue(char* szName, char* szValue)
{
	WritePrivateProfileString("SA-MP", szName, szValue, Path);
}

void CConfig::SetInt(char* szName, int iValue)
{
	char szValue[255];
	sprintf(szValue, "%d", iValue);
	WritePrivateProfileString("SA-MP", szName, szValue, Path);
}

char* CConfig::GetString(char *szName)
{
	char *szResult = new char[256];
	GetPrivateProfileString("SA-MP", szName, NULL, szResult, 256, Path);
	return szResult;
}
	
int CConfig::GetInt(char *szName)
{
	int iReturn = GetPrivateProfileInt("SA-MP", szName, 0, Path);
	return iReturn;
}