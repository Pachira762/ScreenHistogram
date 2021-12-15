#pragma once
#include "WindowCommon.h"
#include "GuiBuilder.h"

class OptionPanel
{
public:
	OptionPanel();

	void	Show();
	void	Hide();
	void	Move(int x, int y);
	void	Size(int cx, int cy);
	
	std::shared_ptr<IGuiBuilder>	Create(HWND parent, int dpi);

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
	int			dpi_ = 96;
	int			panelSize_ = 200;

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

private:
	std::tuple<int, int, int, int> CalcFramePosSize(HWND parent)
	{
		auto [x, y] = ClientToScreen(parent, 0, 0);
		auto [cx, cy] = GetClientSize(parent);
		return { x, y, panelSize_, cy };
	}

	 std::tuple<int, int, int, int> CalcContentPosSize(HWND frame)
	{
		auto [cx, cy] = GetClientSize(frame);
		auto th = TitleHeight;
		auto sw = 0;
		return { 0, 0, cx, cy };
	}

	std::tuple<int, int, int, int> CalcScrollbarPositionSize(HWND hwnd)
	{
		auto [cx, cy] = GetClientSize(hwnd);
		return { cx - VScrollWidth - 2, 0, VScrollWidth, cy };
	}
};

