#pragma once

class AppConfig
{
private:

	std::wstring	path_;

public:
	template<typename Ty>
	struct Key
	{
		LPCTSTR			section;
		LPCTSTR			name;
		std::wstring&	path;

		Ty		Load(const Ty& defaultValue);
		void	Save(const Ty& value);
	};

	using IntKey = Key<int>;
	using FloatKey = Key<float>;
	using StringKey = Key<std::wstring>;

#define DEF_INT_KEY(section, name) IntKey name{section, L#name, path_}
#define DEF_FLOAT_KEY(section, name) FloatKey name{section, L#name, path_}
#define DEF_STRING_KEY(section, name) StringKey name{section, L#name, path_}

	AppConfig(LPCTSTR fileName);

#define SECTION_APP		L"App"
#define SECTION_GRAPH	L"Graph"

	DEF_INT_KEY(SECTION_APP, WindowPosX);
	DEF_INT_KEY(SECTION_APP, WindowPosY);
	DEF_INT_KEY(SECTION_APP, WindowWidth);
	DEF_INT_KEY(SECTION_APP, WindowHeight);
	DEF_INT_KEY(SECTION_APP, FrameRate);

	DEF_INT_KEY(SECTION_GRAPH, MaxResolution);
	DEF_FLOAT_KEY(SECTION_GRAPH, Opacity);
	DEF_FLOAT_KEY(SECTION_GRAPH, Scale);
};

/*
onInit()
{
	auto value = Config.Load(L"name", default);
}

onFin()
{
	Config.Save(L"value, default)
	Config.Flush();
}

*/