#include "pch.h"
#include "AppConfig.h"

template<>
int AppConfig::Key<int>::Load(const int& defaultValue)
{
	return GetPrivateProfileInt(section, name, defaultValue, path.c_str());
}

template<>
void AppConfig::Key<int>::Save(const int& value)
{
	WritePrivateProfileString(section, name, std::to_wstring(value).c_str(), path.c_str());
}

template<>
float AppConfig::Key<float>::Load(const float& defaultValue)
{
	TCHAR buff[256];
	GetPrivateProfileString(section, name, std::to_wstring(defaultValue).c_str(), buff, _countof(buff), path.c_str());
	return std::stof(buff);
}

template<>
void AppConfig::Key<float>::Save(const float& value)
{
	WritePrivateProfileString(section, name, std::to_wstring(value).c_str(), path.c_str());
}

template<>
std::wstring AppConfig::Key<std::wstring>::Load(const std::wstring& defaultValue)
{
	TCHAR buff[1024];
	GetPrivateProfileString(section, name, defaultValue.c_str(), buff, _countof(buff), path.c_str());
	return buff;
}

template<>
void AppConfig::Key<std::wstring>::Save(const std::wstring& value)
{
	WritePrivateProfileString(section, name, value.c_str(), path.c_str());
}

AppConfig::AppConfig(LPCTSTR fileName)
{
	TCHAR buff[MAX_PATH + 1];
	GetCurrentDirectory(_countof(buff), buff);
	swprintf_s(buff, L"%s\\%s", buff, fileName);
	path_ = buff;
}


