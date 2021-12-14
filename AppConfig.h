#pragma once
#include "common.h"

class AppConfig
{
private:
	std::wstring	path_;

public:
	AppConfig(LPCTSTR fileName);

	template<typename Ty>
	struct Key
	{
		AppConfig*	config;
		LPCTSTR		section;
		LPCTSTR		name;
		Ty			value;

		void	Save(const Ty& saveValue) {
			WritePrivateProfileString(section, name, std::to_wstring(static_cast<int>(saveValue)).c_str(), config->path_.c_str());
		}

		void	Save() {
			Save(value);
		}

		Ty&		Load(const Ty& defaultValue) {
			value = static_cast<Ty>(GetPrivateProfileInt(section, name, static_cast<int>(defaultValue), config->path_.c_str()));
			return value;
		}

		operator const Ty&() const {
			return value;
		}

		Key& operator = (const Ty& newValue) {
			value = newValue;
			return *this;
		}
	};

#define SECTION_APP		L"App"
#define SECTION_GRAPH	L"Graph"
#define DEF_CONFIG_KEY(section, type, name) Key<type> name{this, section, L#name}

	DEF_CONFIG_KEY(SECTION_APP, int, WindowPosX);
	DEF_CONFIG_KEY(SECTION_APP, int, WindowPosY);
	DEF_CONFIG_KEY(SECTION_APP, int, WindowWidth);
	DEF_CONFIG_KEY(SECTION_APP, int, WindowHeight);
	DEF_CONFIG_KEY(SECTION_APP, int, FrameRate);

	DEF_CONFIG_KEY(SECTION_GRAPH, EHistogramMode, HistogramMode);
	DEF_CONFIG_KEY(SECTION_GRAPH, EViewMode, ViewMode);
	DEF_CONFIG_KEY(SECTION_GRAPH, int, MaxResolution);
	DEF_CONFIG_KEY(SECTION_GRAPH, float, Opacity);
	DEF_CONFIG_KEY(SECTION_GRAPH, float, Scale);
	DEF_CONFIG_KEY(SECTION_GRAPH, EColorPickMode, ColorPickMode);
};

template<>
inline void AppConfig::Key<float>::Save(const float& saveValue)
{
	WritePrivateProfileString(section, name, std::to_wstring(saveValue).c_str(), config->path_.c_str());
}

template<>
inline float& AppConfig::Key<float>::Load(const float& defaultValue)
{
	value = defaultValue;

	try {
		TCHAR buff[256];
		GetPrivateProfileString(section, name, std::to_wstring(defaultValue).c_str(), buff, _countof(buff), config->path_.c_str());
		value = std::stof(buff);
		return value;
	}
	catch (...) {
		return value;
	}
}

template<>
inline void AppConfig::Key<std::wstring>::Save(const std::wstring& saveValue)
{
	WritePrivateProfileString(section, name, saveValue.c_str(), config->path_.c_str());
}

template<>
inline std::wstring& AppConfig::Key<std::wstring>::Load(const std::wstring& defaultValue)
{
	value = defaultValue;

	try {
		TCHAR buff[1024];
		GetPrivateProfileString(section, name, defaultValue.c_str(), buff, _countof(buff), config->path_.c_str());
		value = buff;
		return value;
	}
	catch (...) {
		return value;
	}
}
