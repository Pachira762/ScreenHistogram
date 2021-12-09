#include "AppWindow.h"
#include "common.h"
#include "ThemeManager.h"

using namespace std;

static AppWindow* pThis = nullptr;

AppWindow::AppWindow(AppWindowListener* listener):
    listener_(listener)
{
	pThis = this;
}

AppWindow::~AppWindow()
{
}

HWND AppWindow::Create(int x, int y, int width, int height)
{
    HINSTANCE instance = GetModuleHandle(NULL);

    INITCOMMONCONTROLSEX icce{};
    icce.dwSize = sizeof(icce);
    icce.dwICC = ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&icce);

    WNDCLASSEX wcex = {};
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = instance;
    wcex.hIcon = LoadIcon(instance, MAKEINTRESOURCE(IDI_APPLICATION));
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = L"ScreenHistogramWnd";
    WINRT_VERIFY(RegisterClassEx(&wcex));

    style_ = WS_POPUP | WS_THICKFRAME | 0;
    styleEx_ = WS_EX_LAYERED | WS_EX_TOPMOST | (UseComposition ? WS_EX_NOREDIRECTIONBITMAP : 0);

    HWND hwnd = CreateWindowEx(styleEx_, L"ScreenHistogramWnd", L"ScreenHistogram", style_,
        x, y, max(width, 800), max(height, 600), NULL, NULL, instance, NULL);
    WINRT_VERIFY(hwnd);

    SetWindowPos(hwnd_, NULL, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);

    return hwnd;
}

void AppWindow::Show()
{
    ShowWindow(hwnd_, SW_SHOW);
}

void AppWindow::SetTransparency(bool transparency)
{
    DWORD newStyle = styleEx_ | (transparency ? WS_EX_TRANSPARENT : 0);
    SetWindowLongPtr(hwnd_, GWL_EXSTYLE, newStyle);
    tranparency_ = transparency;
}

LRESULT AppWindow::HitTest(int x, int y)
{
    constexpr int BorderSize = 7;
    constexpr int CaptionSize = 21;
    
    RECT rc{};
    GetWindowRect(hwnd_, &rc);

    if (x < rc.left || rc.right < x || y < rc.top || rc.bottom < y) {
        return HTNOWHERE;
    }

    int row = 1;
    int col = 1;
    bool cap = false;

    if (y < rc.top + CaptionSize) {
        row = 0;
        cap = (y > rc.top + BorderSize);
    }
    else if(y > rc.bottom - CaptionSize) {
        row = 2;
        cap = (y < rc.bottom - BorderSize);
    }

    if (x < rc.left + CaptionSize) {
        col = 0;
        cap = (x > rc.left + BorderSize);
    }
    else if (x > rc.right - CaptionSize) {
        col = 2;
        cap = (x < rc.right - BorderSize);
    }

    if (cap) {
        return HTCAPTION;
    }

    constexpr LRESULT HitTestResults[3][3] = {
        {HTTOPLEFT, HTTOP, HTTOPRIGHT },
        {HTLEFT, HTCLIENT,HTRIGHT},
        {HTBOTTOMLEFT, HTBOTTOM,HTBOTTOMRIGHT}
    };

    return HitTestResults[row][col];
}

void AppWindow::OnCreate(LPCREATESTRUCT cs)
{
    SetLayeredWindowAttributes(hwnd_, 0, 255, LWA_ALPHA);
    SetWindowDisplayAffinity(hwnd_, WDA_EXCLUDEFROMCAPTURE);

    SetTimer(hwnd_, 1, 100, NULL);
}

LRESULT AppWindow::OnNcHitTest(int x, int y)
{
    auto hit = HitTest(x, y);

    if (hit == HTCLIENT) {
        SetTransparency(true);
    }

    return hit;
}

void AppWindow::OnCheckTransparency()
{
    if (!tranparency_) {
        return;
    }

    POINT cursorPos;
    GetCursorPos(&cursorPos);

    if (HitTest(cursorPos.x, cursorPos.y) != HTCLIENT) {
        SetTransparency(false);
    }
}

LRESULT CALLBACK AppWindow::WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg) {
	case WM_CLOSE:
        pThis->listener_->OnClose();
		DestroyWindow(hwnd);
		break;

	case WM_DESTROY:
        pThis->listener_->OnDestory();
		PostQuitMessage(0);
		break;

    case WM_CREATE:
        pThis->hwnd_ = hwnd;
        pThis->OnCreate((LPCREATESTRUCT)lp);
        pThis->listener_->OnCreate(hwnd);
        break;

    case WM_NCHITTEST:
        return pThis->OnNcHitTest(GET_X_LPARAM(lp), GET_Y_LPARAM(lp));

    case WM_NCCALCSIZE:
        if (wp) {
            auto& rc = reinterpret_cast<LPNCCALCSIZE_PARAMS>(lp)->rgrc[0];
            rc.top += 1;
            rc.left += 1;
            rc.right -= 1;
            rc.bottom -= 1;
            return 0;
        }
        return DefWindowProc(hwnd, msg, wp, lp);

    case WM_TIMER:
        pThis->OnCheckTransparency();
        break;

	default:
		return DefWindowProc(hwnd, msg, wp, lp);
	}

	return 0;
}

