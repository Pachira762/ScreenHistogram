#pragma once

class ThemeManager
{
public:
	~ThemeManager();

	void	EnableAcrylicWindow(HWND hwnd);

	HFONT	GetGuiFont();
	void	SetGuiFont(HWND hwnd);

	static ThemeManager* GetInstance();

private:
	HFONT	hGuiFont_{};

	ThemeManager();

	void	InitFonts();
};

