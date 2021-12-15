#pragma once

RECT GetAdjustedWindowRect(DWORD style, DWORD styleEx = 0, BOOL menu = FALSE);

inline const RECT FrameSize = { -GetAdjustedWindowRect(WS_OVERLAPPEDWINDOW).left, GetAdjustedWindowRect(WS_OVERLAPPEDWINDOW).bottom, GetAdjustedWindowRect(WS_OVERLAPPEDWINDOW).right,GetAdjustedWindowRect(WS_OVERLAPPEDWINDOW).bottom };
inline const RECT DragSize = { -GetAdjustedWindowRect(WS_OVERLAPPEDWINDOW).top,-GetAdjustedWindowRect(WS_OVERLAPPEDWINDOW).top ,-GetAdjustedWindowRect(WS_OVERLAPPEDWINDOW).top ,-GetAdjustedWindowRect(WS_OVERLAPPEDWINDOW).top };
inline const int TitleHeight = -GetAdjustedWindowRect(WS_OVERLAPPEDWINDOW).top;
inline const int CloseButtonSize = -GetAdjustedWindowRect(WS_OVERLAPPEDWINDOW).top;
inline const int VScrollWidth = GetSystemMetrics(SM_CXVSCROLL);

#define DPISCALE(x, dpi) (((x)*dpi)/96)

inline POINT GetCursorPos()
{
    POINT p{};
    GetCursorPos(&p);
    return p;
}

inline RECT GetWindowRect(HWND hwnd)
{
    RECT rc{};
    GetWindowRect(hwnd, &rc);
    return rc;
}

inline RECT GetClientRect(HWND hwnd)
{
    RECT rc{};
    GetClientRect(hwnd, &rc);
    return rc;
}

inline SIZE GetClientSize(HWND hwnd)
{
    RECT rc{};
    GetClientRect(hwnd, &rc);
    return { rc.right, rc.bottom };
}

inline POINT ScreenToClient(HWND hwnd, int x, int y)
{
    POINT p{ x, y };
    ScreenToClient(hwnd, &p);
    return p;
}

inline POINT ClientToScreen(HWND hwnd, int x, int y)
{
    POINT p{ x, y };
    ClientToScreen(hwnd, &p);
    return p;
}

inline RECT GetAdjustedWindowRect(DWORD style, DWORD styleEx, BOOL menu)
{
    RECT rc{ 0,0,0,0 };
    AdjustWindowRectEx(&rc, style, menu, styleEx);
    return rc;
}

inline HRESULT NonClientHitTest(HWND hwnd, int x, int y, int dpi)
{
    const auto rc = GetWindowRect(hwnd);
    int row = 1;
    int col = 1;
    bool cap = false;

    if (x < rc.left || rc.right < x || y < rc.top || rc.bottom < y) {
        return HTNOWHERE;
    }

    if (y < rc.top + DragSize.top) {
        row = 0;
        cap = (y > rc.top + FrameSize.top);
    }
    else if (y > rc.bottom - DragSize.bottom) {
        row = 2;
        cap = (y < rc.bottom - FrameSize.bottom);
    }

    if (x < rc.left + DragSize.left) {
        col = 0;
        cap = (x > rc.left + FrameSize.left);
    }
    else if (x > rc.right - DragSize.right) {
        col = 2;
        cap = (x < rc.right - FrameSize.right);
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

int GetScrollPos(HWND scroll, int bar = SB_VERT);

int VScrollProc(HWND scroll, WPARAM wp, int bar = SB_VERT);

int UpdateScroll(HWND scroll, int page, int range = -1, int bar = SB_VERT);

HRESULT NonClientHitTest(HWND hwnd, int x, int y, int dpi);

LRESULT	DrawRadioButton(NMCUSTOMDRAW* nmc, int dpi);
