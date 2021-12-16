#include "WinUtil.h"

void WinUtil::SetDpi(int Dpi)
{
	dpi_ = (int)Dpi;

    RECT rc = {};
    AdjustWindowRectExForDpi(&rc, WS_OVERLAPPEDWINDOW, FALSE, 0, 96);
    frame_.left = -rc.left;
    frame_.top = rc.bottom;
    frame_.right = rc.right;
    frame_.bottom = rc.bottom;
    caption_ = -rc.top;

    rc.left = rc.top = rc.right = rc.bottom = 0;
    AdjustWindowRectExForDpi(&rc, WS_OVERLAPPEDWINDOW, FALSE, 0, dpi_);
    frame_pix_.left = -rc.left;
    frame_pix_.top = rc.bottom;
    frame_pix_.right = rc.right;
    frame_pix_.bottom = rc.bottom;
    caption_pix_ = -rc.top;

    vscroll_.cx = GetSystemMetricsForDpi(SM_CXVSCROLL, 96);
    vscroll_.cy = GetSystemMetricsForDpi(SM_CYVSCROLL, 96);
    hscroll_.cx = GetSystemMetricsForDpi(SM_CXHSCROLL, 96);
    hscroll_.cy = GetSystemMetricsForDpi(SM_CYHSCROLL, 96);
}

void WinUtil::SetDpi(HWND hwnd)
{
    auto ret = GetDpiForWindow(hwnd);
    SetDpi(static_cast<int>(ret != 0 ? ret : GetDpiForSystem()));
}

HWND WinUtil::Create(LPCTSTR className, LPCTSTR windowName, DWORD style, DWORD styleEx, int x, int y, int width, int height, HWND parent, HMENU menu)
{
	static auto instance = GetModuleHandle(NULL);
	HWND hwnd = CreateWindowEx(styleEx, className, windowName, style, Pix(x), Pix(y), Pix(width), Pix(height), parent, menu, instance, NULL);
	WINRT_VERIFY(hwnd);
	return hwnd;
}

LRESULT WinUtil::NonClientHitTest(HWND hwnd, int x, int y)
{
    int row = 1;
    int col = 1;
    bool cap = false;

    RECT rc{};
    GetWindowRect(hwnd, &rc);

    if (x < rc.left || rc.right < x || y < rc.top || rc.bottom < y) {
        return HTNOWHERE;
    }

    if (y < rc.top + caption_pix_) {
        row = 0;
        cap = (y > rc.top + frame_pix_.top);
    }
    else if (y > rc.bottom - caption_pix_) {
        row = 2;
        cap = (y < rc.bottom - frame_pix_.bottom);
    }

    if (x < rc.left + caption_pix_) {
        col = 0;
        cap = (x > rc.left + frame_pix_.left);
    }
    else if (x > rc.right - caption_pix_) {
        col = 2;
        cap = (x < rc.right - frame_pix_.right);
    }

    if (cap) {
        return HTCAPTION;
    }

    constexpr LRESULT HitTestResults[3][3] = {
        {HTTOPLEFT, HTTOP, HTTOPRIGHT },
        {HTLEFT, HTCLIENT, HTRIGHT},
        {HTBOTTOMLEFT, HTBOTTOM,HTBOTTOMRIGHT}
    };

    return HitTestResults[row][col];
}

