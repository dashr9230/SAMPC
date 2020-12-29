
#pragma once

class CConfig
{
public:
	CConfig();
	~CConfig() {};

	void SetValue(char *szName, char *szValue);
	void SetInt(char* szName, int iValue);
	char* GetString(char *szName);
	int GetInt(char *szName);

private:
	char Directory[250];
	char Path[260];
	char szReturn;
	int iReturn;
};