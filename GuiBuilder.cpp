#include "pch.h"
#include "GuiBuilder.h"
#include "WindowCommon.h"
#include "Theme.h"

using namespace std;

GuiBuilderImpl::GuiBuilderImpl(HWND hwnd, const RECT& margin, int dpi) :
    hwnd_(hwnd),
    tree_(GetClientRect(hwnd)),
    hdc_(GetDC(hwnd)),
    dpi_(dpi)
{
    tree_.left += DPISCALE(margin.left, dpi_);
    tree_.top += DPISCALE(margin.top, dpi_);
    tree_.right -= DPISCALE(margin.right, dpi_);
    tree_.bottom -= DPISCALE(margin.bottom, dpi_);
}

GuiBuilderImpl::~GuiBuilderImpl()
{
    ReleaseDC(hwnd_, hdc_);
}

void GuiBuilderImpl::Build()
{
    if (OnBuild) {
        OnBuild(this, tree_.right - tree_.left, tree_.top + 12);
    }
}

void GuiBuilderImpl::AddBlank(int height)
{
    tree_.top += DPISCALE(height, dpi_);
}

void GuiBuilderImpl::AddText(LPCTSTR text)
{
    RECT layout = tree_;
    SelectObject(hdc_, Theme::TextFont);
    DrawText(hdc_, text, -1, &layout, DT_CALCRECT);

    tree_.top += DPISCALE(layout.bottom - layout.top, dpi_);

    if (OnTextAdded) {
        OnTextAdded(text, layout);
    }
}

void GuiBuilderImpl::AddLabel(LPCTSTR text)
{
    RECT layout = tree_;
    SelectObject(hdc_, Theme::LabelFont);
    DrawText(hdc_, text, -1, &layout, DT_SINGLELINE | DT_CALCRECT);

    //tree_.top += DPISCALE((layout.bottom - layout.top) + 4, dpi_);
    tree_.top += (layout.bottom - layout.top) + DPISCALE(4, dpi_);

    if (OnLabelAdded) {
        OnLabelAdded(text, layout);
    }
}

void GuiBuilderImpl::AddRadioButtons(const std::vector<LPCTSTR>& options, int initial, const RadioButtonCallback& callback)
{
    bool isTop = true;
    vector<HWND> handles = {};

    for (int i = 0; i < options.size(); ++i) {
        HWND radio = CreateWindowEx(0, L"BUTTON", options[i], WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON | (isTop ? (WS_GROUP | WS_TABSTOP) : 0),
            tree_.left, tree_.top, DPISCALE(tree_.right - tree_.left, dpi_), DPISCALE(32, dpi_), hwnd_, (HMENU)i, instance_, NULL);
        WINRT_VERIFY(radio);

        SendMessage(radio, WM_SETFONT, (WPARAM)Theme::TextFont, FALSE);
        if (i == initial) {
            SendMessage(radio, BM_SETCHECK, TRUE, TRUE);
        }

        isTop = false;
        handles.push_back(radio);

        tree_.top += DPISCALE(32, dpi_);
    }

    if (OnRadioGroupAdded) {
        OnRadioGroupAdded(handles, callback);
    }
}

void GuiBuilderImpl::AddSlider(int min, int max, int initial, const SliderCallback& callback)
{
    HWND slider = CreateWindow(TRACKBAR_CLASS, L"track", WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP | TBS_NOTICKS,
        tree_.left - 4, tree_.top, tree_.right - tree_.left, 24, hwnd_, (HMENU)0x10, instance_, NULL);
    WINRT_VERIFY(slider);

    SendMessage(slider, TBM_SETRANGE, FALSE, MAKELPARAM(min, max));
    SendMessage(slider, TBM_SETPOS, TRUE, initial);

    tree_.top += DPISCALE(24, dpi_);

    if (OnSliderAdded) {
        OnSliderAdded(slider, callback);
    }
}

static LRESULT CALLBACK CloseButtonProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR id, DWORD_PTR ptr);

HWND GuiBuilderImpl::CreateCloseButton(HWND hwnd)
{
    const int size = CloseButtonSize;
    const auto[cx, cy] = GetClientSize(hwnd);

    HWND close = CreateWindowEx(WS_EX_TOPMOST, L"BUTTON", NULL, WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        cx - size, 0, size, size, hwnd, (HMENU)0x101, instance_, NULL);
    WINRT_VERIFY(close);

    SendMessage(close, WM_SETFONT, (WPARAM)Theme::IconFont, TRUE);
    SetWindowSubclass(close, CloseButtonProc, NULL, NULL);

    return close;
}

HWND GuiBuilderImpl::CreateScrollBar(HWND hwnd, int x, int y, int width, int height, int page, int content, bool darkmode)
{
    bool show = page < content;

    HWND scroll = CreateWindowEx(0, L"SCROLLBAR", NULL, WS_CHILD | SBS_VERT | (show ? WS_VISIBLE : 0),
        x, y, width, height, hwnd, NULL, instance_, NULL);
    WINRT_VERIFY(scroll);

    SCROLLINFO info{};
    info.cbSize = sizeof(info);
    info.fMask = SIF_RANGE | SIF_PAGE;
    info.nMin = 0;
    info.nMax = content;
    info.nPage = page;
    SetScrollInfo(scroll, SB_CTL, &info, TRUE);

    if (darkmode) {
        SetWindowTheme(scroll, L"DarkMode_Explorer", NULL);
    }

    return scroll;
}

static LRESULT CloseButtonHittest(HWND hwnd, int x, int y)
{
    return HTCLIENT;
}

static LRESULT CALLBACK CloseButtonProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR id, DWORD_PTR ptr)
{
    switch (msg) {
    case WM_NCHITTEST:
        return CloseButtonHittest(hwnd, GET_X_LPARAM(lp), GET_Y_LPARAM(lp));
    }

    return DefSubclassProc(hwnd, msg, wp, lp);
}