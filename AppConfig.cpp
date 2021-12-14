#include "pch.h"
#include "AppConfig.h"

AppConfig::AppConfig(LPCTSTR fileName)
{
	TCHAR buff[MAX_PATH + 1];
	GetCurrentDirectory(_countof(buff), buff);
	swprintf_s(buff, L"%s\\%s", buff, fileName);
	path_ = buff;
}
