#pragma once
#include "GuiBuilder.h"

class OptionPanel
{
public:
	OptionPanel();

	void	Show();
	void	Hide();
	void	Move(int x, int y);
	void	Size(int cx, int cy);
	
	std::shared_ptr<IGuiBuilder>	Create(HWND parent);

private:
	struct Text
	{
		RECT			layout;
		std::wstring	text;
	};

	struct RadioButtonGroup
	{
		std::vector<HWND>	handles;
		RadioButtonCallback	callback;
	};

	HINSTANCE	instance_ = NULL;
	HWND		parent_ = NULL;
	HWND		frame_ = NULL;

	std::vector<Text>	texts_ = {};
	std::vector<Text>	labels_ = {};
	std::vector<RadioButtonGroup>	radioGroups_ = {};
	std::map<HWND, SliderCallback>	sliders_ = {};
	std::set<HWND>	customDrawRadioButtons_ = {};

	void	CheckRadioButtons(HWND radio, int index);
	void	OnCreate(LPCREATESTRUCT cs);
	void	OnSize(int cx, int cy);
	void	OnPaint();
	void	OnHScroll(HWND scroll);
	void	OnCommand(int id, int code, HWND handle);
	LRESULT	OnNotify(DWORD id, NMHDR* hdr);
	LRESULT	OnCustomDraw(NMCUSTOMDRAW* nmc);

	static LRESULT CALLBACK FrameWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
};

