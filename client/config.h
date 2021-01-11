
#pragma once

class CConfig
{
private:
	char szPath[261];

public:
	CConfig();
	~CConfig() {};

	void SetString(char* szName, char* szValue);
	void SetInt(char* szName, int iValue);
	char* GetString(char* szName);
	int GetInt(char* szName);
};