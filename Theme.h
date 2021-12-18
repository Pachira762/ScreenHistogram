#pragma once

// windows Defender gets angry
//#define ENABLE_ACRYLIC

class Theme
{
public:
	struct Dark
	{
		static constexpr DWORD	BgColor = 0x202020;
		static constexpr DWORD	TextColor = 0xBBBBBB;
		static constexpr DWORD	AccentTextColor = 0xCCCCCC;
		static constexpr DWORD	DisableTextColor = 0x606060;
		
		static inline HBRUSH	BgBrush;
	};

#ifdef ENABLE_ACRYLIC
	static constexpr bool IsEnabledAcrylic = true;
#else
	static constexpr bool IsEnabledAcrylic = false;
#endif

	static inline HBRUSH CloseHotBrush = NULL;
	static inline HBRUSH CloseSelectedBrush = NULL;

	static inline std::wstring	FontName = L"Segoe UI";
	static inline std::wstring	MonoFontName = L"Consolas";
	static inline int			FontSize = 14;

	static inline HFONT	TextFont = NULL;
	static inline HFONT	LabelFont = NULL;
	static inline HFONT IconFont = NULL;

	static void SetWindowTheme_DarkMode_Explorer(HWND hwnd);
	static void	EnableRoundCorner(HWND hwnd, bool enable = true);
	static void	EnableBlurBackground(HWND hwnd, DWORD color = Dark::BgColor, DWORD opacity = 0x99);
	static void	EnableAcrylicWindow(HWND hwnd, DWORD color = Dark::BgColor, DWORD opacity = 0x99);

	static void SetDpi(int dpi);

	static Theme*	Get();

private:
	using fnSetWindowCompositionAttribute = BOOL(WINAPI*)(HWND, const struct WINDOWCOMPOSITIONATTRIBDATA*);

	static inline fnSetWindowCompositionAttribute SetWindowCompositionAttribute = nullptr;

	Theme();
	~Theme();
	static void	InitBrushes();
	static void	InitFonts(int dpi);
	static void	InitCompositionAttribute();
};

