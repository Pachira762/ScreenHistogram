#include "GuiLayout.h"
#include "WinUtil.h"
#include "Theme.h"

using namespace std;

//
// GuiSpace
//
GuiSpace::GuiSpace(int x, int y, int width, int height)
{
	rect_.left = x;
	rect_.top = y;
	rect_.right = x + width;
	rect_.bottom = y + height;
}

void GuiSpace::Layout(int x, int y, int width, int height, HDC hdc)
{
	auto size = GetSize();
	rect_.left = x;
	rect_.top = y;
	rect_.right = x + size.cx;
	rect_.bottom = y + size.cy;
}

//
// Text
//
GuiText::GuiText(LPCTSTR text, int x, int y, int width, HDC hdc, bool label):
	text_(text),
	isLabel_(label)
{
	Layout(x, y, width, 0, hdc);
}

void GuiText::Layout(int x, int y, int width, int height, HDC hdc)
{
	RECT rc = { 0,0,WinUtil::Pix(width),0 };
	SelectObject(hdc, isLabel_ ? Theme::LabelFont : Theme::TextFont);
	DrawText(hdc, text_.c_str(), text_.length(), &rc, DT_CALCRECT | (isLabel_ ? DT_SINGLELINE : 0));
	rect_.left = x;
	rect_.top = y;
	rect_.right = x + WinUtil::Dpi(rc.right);
	rect_.bottom = y + WinUtil::Dpi(rc.bottom) + (isLabel_ ? 4 : 0);
}

void GuiText::Draw(HDC hdc, int scroll)
{
	auto rc = WinUtil::Pix(rect_);
	rc.top -= WinUtil::Pix(scroll);
	rc.bottom -= WinUtil::Pix(scroll);
	SetTextColor(hdc, isLabel_ ? Theme::Dark::AccentTextColor : Theme::Dark::TextColor);
	SelectObject(hdc, isLabel_ ? Theme::LabelFont : Theme::TextFont);
	DrawText(hdc, text_.c_str(),  text_.length(), &rc, isLabel_ ? (DT_VCENTER | DT_SINGLELINE) : 0);
}

//
// Radio Button
//
GuiRadioButtons::GuiRadioButtons(HWND parent, int x, int y, int width, std::vector<LPCTSTR> options, int selected)
{
	rect_.left = x;
	rect_.top = y;
	rect_.right = x + width;
	rect_.bottom = y + 40*options.size();

	bool isTop = true;

	for (int i = 0; i < options.size(); ++i) {
		HWND radio = WinUtil::Create(L"BUTTON", options[i], WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON | (isTop ? WS_GROUP : 0), 0,
			rect_.left, rect_.top + 40*i, (rect_.right - rect_.left), 40, parent, (HMENU)i);
		WINRT_VERIFY(radio);

		SendMessage(radio, WM_SETFONT, (WPARAM)Theme::TextFont, TRUE);

		if (i == selected) {
			SendMessage(radio, BM_SETCHECK, TRUE, TRUE);
		}

		isTop = false;
		radios_.push_back(radio);
	}
}

void GuiRadioButtons::Layout(int x, int y, int width, int height, HDC hdc)
{
	rect_.left = x;
	rect_.top = y;
	rect_.right = x + width;
	rect_.bottom = y + 40 * radios_.size();

	for (int i = 0; i < radios_.size(); ++i) {
		WinUtil::SetWindowPosSize(radios_[i], x, y + 40 * i, width, 40);
	}
}

//
// CheckBox
//
GuiCheckboxes::GuiCheckboxes(HWND parent, int x, int y, int width, std::vector<LPCTSTR> options)
{
	rect_.left = x;
	rect_.top = y;
	rect_.right = 20 + x + 40 * options.size();
	rect_.bottom = y + 40;

	for (int i = 0; i < options.size(); ++i) {
		HWND checkbox = WinUtil::Create(L"BUTTON", options[i], WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, 0,
			20 + x + i * 60, y, 40, 40, parent, (HMENU)i);
		WINRT_VERIFY(checkbox);

		SendMessage(checkbox, WM_SETFONT, (WPARAM)Theme::TextFont, TRUE);
		SendMessage(checkbox, BM_SETCHECK, TRUE, TRUE);

		checkboxes_.push_back(checkbox);
	}
}

void GuiCheckboxes::Layout(int x, int y, int width, int height, HDC hdc)
{
	rect_.left = x;
	rect_.top = y;
	rect_.right = 20 + x + 40 * checkboxes_.size();
	rect_.bottom = y + 40;

	for (int i = 0; i < checkboxes_.size(); ++i) {
		WinUtil::SetWindowPosSize(checkboxes_[i], 20 + x + i * 40, y, 40, 40);
	}
}

//
// Slider
//
GuiSlider::GuiSlider(HWND parent, int x, int y, int width, int min, int max, int value)
{
	rect_.left = x;
	rect_.top = y;
	rect_.right = x + width;
	rect_.bottom = y + 24;

	slider_ = WinUtil::Create(TRACKBAR_CLASS, NULL, WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP | TBS_NOTICKS,0,
		x, y, width, 24, parent);
	WINRT_VERIFY(slider_);

	SendMessage(slider_, TBM_SETRANGEMIN, FALSE, min);
	SendMessage(slider_, TBM_SETRANGEMAX, FALSE, max);
	SendMessage(slider_, TBM_SETPOS, TRUE, value);
}

void GuiSlider::Layout(int x, int y, int width, int height, HDC hdc)
{
	rect_.left = x;
	rect_.top = y;
	rect_.right = x + width;
	rect_.bottom = y + 24;
	WinUtil::SetWindowPosSize(slider_, x, y, width, 24);
}

//
// Layout
//
GuiLayout::GuiLayout(int marginX, int marginY) :
	marginX_(marginX),
	marginY_(marginY)
{
}

void GuiLayout::AddElement(std::unique_ptr<GuiElement>&& elem)
{
	elements_.push_back(move(elem));
}

void GuiLayout::Layout(HDC hdc, int width)
{
	int curX = marginX_;
	int curY = marginY_;

	for (auto& elem : elements_) {
		elem->Layout(curX, curY, width, 0, hdc);

		auto[w, h] = elem->GetSize();
		curY += h;
	}
}

void GuiLayout::Draw(HDC hdc, int scroll)
{
	for (auto& elem : elements_) {
		elem->Draw(hdc, scroll);
	}
}

