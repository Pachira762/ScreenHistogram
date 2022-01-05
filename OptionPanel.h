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

	struct CheckboxGroup
	{
		std::vector<HWND>	handles;
		CheckboxCallback	callback;
	};

	HWND	parent_ = NULL;
	HWND	hwnd_ = NULL;
	int		scroll_ = 0; // pixel unit

	std::unique_ptr<GuiLayout> layout_;
	std::vector<Text>	texts_ = {};
	std::vector<Text>	labels_ = {};
	std::vector<RadioButtonGroup>	radioGroups_ = {};
	std::vector<CheckboxGroup>		checkboxGroups_ = {};
	std::map<HWND, SliderCallback>	sliders_ = {};
	std::set<HWND>	customDrawTargets_ = {};

	void	UpdateScrollRange(int range);
	void	UpdateScrollPage(int page);
	void	ScrollContent(int dy);
	void	OnCreate(LPCREATESTRUCT cs);
	void	OnSize(int cx, int cy);
	void	OnPaint();
	void	OnHScroll(HWND slider);
	void	OnVScroll(int code, int delta);
	void	OnCommand(int id, int code, HWND handle);
	void	OnButtonClicked(HWND hwnd, int id);
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
		return WinUtil::Dpi(si.nPos);
	}

public:

	void Layout()
	{
		ScrollContent(-scroll_);	// reset scroll pos

		auto hdc = GetDC(hwnd_);
		auto [cx, cy] = WinUtil::GetClientSize(hwnd_);
		layout_->Layout(hdc, cx - MarginX - VScrollWidth);
		ReleaseDC(hwnd_, hdc);

		SCROLLINFO si = {};
		si.cbSize = sizeof(si);
		si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
		si.nPos = 0;
		si.nMin = 0;
		si.nMax = WinUtil::Pix(layout_->Height());
		si.nPage = WinUtil::Pix(WinUtil::GetClientSize(hwnd_).cy);
		SetScrollInfo(hwnd_, SB_VERT, &si, TRUE);

		InvalidateRect(hwnd_, NULL, TRUE);
		UpdateWindow(hwnd_);
	}
};

