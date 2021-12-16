#pragma once
#include "WinUtil.h"
#include "GuiBuilder.h"
#include "GuiLayout.h"

class OptionPanel
{
public:
	OptionPanel();

	void	Show();
	void	Hide();
	void	Move();
	void	Size();
	
	std::shared_ptr<IGuiBuilder>	Create(HWND parent);

private:
	static constexpr auto PanelSize = 200;
	static constexpr auto MarginX = 12; 
	static constexpr auto MarginY = 8;

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

	HWND	parent_ = NULL;
	HWND	hwnd_ = NULL;

	std::unique_ptr<GuiLayout> layout_;
	std::vector<Text>	texts_ = {};
	std::vector<Text>	labels_ = {};
	std::vector<RadioButtonGroup>	radioGroups_ = {};
	std::map<HWND, SliderCallback>	sliders_ = {};
	std::set<HWND>	customDrawRadioButtons_ = {};

	void	CheckRadioButtons(HWND radio, int index);
	int		UpdateScrollPage(int page);

	void	OnCreate(LPCREATESTRUCT cs);
	void	OnSize(int cx, int cy);
	void	OnPaint();
	void	OnHScroll(HWND slider);
	void	OnVScroll(int code, int delta);
	void	OnCommand(int id, int code, HWND handle);
	LRESULT	OnNotify(DWORD id, NMHDR* hdr);
	LRESULT	OnCustomDraw(NMCUSTOMDRAW* nmc);

	static LRESULT CALLBACK FrameWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

private:
	POINT CalcFramePos()
	{
		auto [x, y] = WinUtil::GetClientPos(parent_);
		return { x - 1, y - 1 };
	}

	SIZE CalcFrameSize()
	{
		auto [cx, cy] = WinUtil::GetClientSize(parent_);
		return { PanelSize, cy + 2 };
	}

	std::tuple<int, int, int, int> CalcScrollbarPositionSize(HWND hwnd)
	{
		auto [cx, cy] = WinUtil::GetClientSize(hwnd);
		return { cx - VScrollWidth - 2, 0, VScrollWidth, cy };
	}

	int GetScrollPos()
	{
		SCROLLINFO si{};
		si.cbSize = sizeof(si);
		si.fMask = SIF_POS;
		GetScrollInfo(hwnd_, SB_VERT, &si);
		return si.nPos;
	}

public:

	void Layout()
	{
		Size();

		auto hdc = GetDC(hwnd_);
		auto [cx, cy] = WinUtil::GetClientSize(hwnd_);
		layout_->Layout(hdc, cx - MarginX - VScrollWidth);
		ReleaseDC(hwnd_, hdc);

		SCROLLINFO si{};
		si.cbSize = sizeof(si);
		si.fMask = SIF_ALL;
		GetScrollInfo(hwnd_, SB_VERT, &si);

		ScrollWindowEx(hwnd_, 0, WinUtil::Pix(-si.nPos), nullptr, nullptr, nullptr, nullptr, SW_ERASE | SW_INVALIDATE | SW_SCROLLCHILDREN); // reset scroll

		si.fMask = SIF_RANGE | SIF_PAGE;
		si.nMin = 0;
		si.nMax = layout_->Height();
		si.nPage = WinUtil::GetClientSize(this->hwnd_).cy;
		SetScrollInfo(hwnd_, SB_VERT, &si, TRUE);

		si.fMask = SIF_POS;
		GetScrollInfo(hwnd_, SB_VERT, &si);

		ScrollWindowEx(hwnd_, 0, WinUtil::Pix(si.nPos), nullptr, nullptr, nullptr, nullptr, SW_ERASE | SW_INVALIDATE | SW_SCROLLCHILDREN);
		UpdateWindow(hwnd_);
	}
};

