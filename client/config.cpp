
#include "main.h"

CConfig::CConfig()
{
	sprintf_s(szPath, "%s\\sa-mp.cfg", szUserDocPath);
}

void CConfig::SetString(char* szName, char* szValue)
{
	WritePrivateProfileString("SA-MP", szName, szValue, szPath);
}

void CConfig::SetInt(char* szName, int iValue)
{
	char szValue[256];
	sprintf(szValue, "%d", iValue);
	WritePrivateProfileString("SA-MP", szName, szValue, szPath);
}

char* CConfig::GetString(char* szName)
{
	char* szResult = new char[256];
	GetPrivateProfileString("SA-MP", szName, NULL, szResult, 256, szPath);
	return szResult;
}

int CConfig::GetInt(char* szName)
{
	int iReturn = GetPrivateProfileInt("SA-MP", szName, 0, szPath);
	return iReturn;
}