#include "AppWindow.h"
#include "common.h"
#include "WinUtil.h"
#include "Theme.h"
#include "OptionPanel.h"

using namespace std;

static AppWindow* pThis = nullptr;

AppWindow::AppWindow(AppWindowListener* listener):
    listener_(listener),
    panel_(make_unique<OptionPanel>())
{
	pThis = this;
}

AppWindow::~AppWindow()
{
}

HWND AppWindow::Create(int x, int y, int width, int height, EHistogramMode histogramMode, EViewMode viewMode, float scale, float opacity)
{
    HINSTANCE instance = GetModuleHandle(NULL);

    WNDCLASSEX wcex = {};
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = 0;
    wcex.lpfnWndProc = AppWindow::WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = instance;
    wcex.hIcon = LoadIcon(instance, MAKEINTRESOURCE(IDC_ICON));
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wcex.lpszClassName = L"ScreenHistogramWnd";
    WINRT_VERIFY(RegisterClassEx(&wcex));

    style_ = WS_POPUP | WS_THICKFRAME | WS_SYSMENU;
    styleEx_ = WS_EX_LAYERED |WS_EX_TOPMOST | (UseComposition ? WS_EX_NOREDIRECTIONBITMAP : 0);

    CreateWindowEx(styleEx_, L"ScreenHistogramWnd", L"ScreenHistogram", style_,
        x, y, max(width, 480), max(height, 360), NULL, NULL, instance, NULL);
    WINRT_VERIFY(hwnd_);

    auto builder = panel_->Create(hwnd_);
    builder->AddLabel(L"Histogram");
    builder->AddRadioButtons({ L"RGB", L"RGB Brightness", L"Brightness" }, (int)histogramMode, [this](int index) {
        this->listener_->SetHistogramMode((EHistogramMode)index);
        });
    builder->AddBlank(12);

    builder->AddLabel(L"View");
    builder->AddRadioButtons({ L"Color", L"Brightness", L"Saturation(HSV)", L"Saturation(HLS)", L"Color Mask"}, (int)viewMode, [this](int index) {
        this->listener_->SetViewMode((EViewMode)index);
        });
    builder->AddCheckboxes({ L"R", L"G", L"B" }, 0, [this](const std::vector<bool>& checks) {
        if (checks.size() != 3) {
            return;
        }
        this->listener_->SetColorMask(checks[0], checks[1], checks[2]);
        });
    builder->AddBlank(12);

    builder->AddLabel(L"Scale");
    builder->AddSlider(0, 200, 10 * scale, [this](int value) {
        this->listener_->SetScale(0.1f * value);
        });
    builder->AddBlank(12);

    builder->AddLabel(L"Opacity");
    builder->AddSlider(0, 100, 100 * opacity, [this](int value) {
        this->listener_->SetOpacity(0.01 * value);
        });
    builder->AddBlank(12);

    builder->AddLabel(L"Color Pick");
    builder->AddRadioButtons({ L"None", L"RGB" ,L"HSV" ,L"HLS" }, 0, [this](int value) {
        this->listener_->SetColorPickMode((EColorPickMode)value);
        });
    builder->Build();

    return hwnd_;
}

void AppWindow::Show()
{
    ShowWindow(hwnd_, SW_SHOW);
}

void AppWindow::KillTimer()
{
    if (timer_) {
        ::KillTimer(hwnd_, timer_);
        timer_ = NULL;
    }
}

void AppWindow::CheckTransparency()
{
    if (transparency_) {
        POINT pos{};
        GetCursorPos(&pos);
        if (WinUtil::NonClientHitTest(hwnd_, pos.x, pos.y) != HTCLIENT) {
            SetTransparency(false);
        }
    }
}

void AppWindow::SetTransparency(bool transparency)
{
    if (transparency_ == transparency) {
        return;
    }

    DWORD newStyle = styleEx_ | (transparency ? WS_EX_TRANSPARENT : 0);
    SetWindowLongPtr(hwnd_, GWL_EXSTYLE, newStyle);
    transparency_ = transparency;
}

void AppWindow::OnCreate(LPCREATESTRUCT cs)
{
    WinUtil::SetDpi(hwnd_);

    SetWindowPos(hwnd_, NULL, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
    SetLayeredWindowAttributes(hwnd_, 0, 255, LWA_ALPHA);
    SetWindowDisplayAffinity(hwnd_, WDA_EXCLUDEFROMCAPTURE);

    auto size = CaptionSize;
    auto [cw, ch] = WinUtil::GetClientSize(hwnd_);
    close_ = WinUtil::Create(L"BUTTON", L"X", WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 0,
       cw - size, 0, size, size, hwnd_, (HMENU)0x100);

    timer_ = SetTimer(hwnd_, 1, 100, [](HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) {
        pThis->CheckTransparency();
        });
}

void AppWindow::OnMove(int x, int y)
{
    panel_->Move();
}

void AppWindow::OnSize(int cx, int cy)
{
    auto size = CaptionSize;
    WinUtil::SetWindowPosSize(close_, cx - size, 0, size, size);

    panel_->Size();
}

LRESULT AppWindow::OnNcHitTest(int x, int y)
{
    const auto hit = WinUtil::NonClientHitTest(hwnd_, x, y);
    if (hit == HTNOWHERE) {
        return HTNOWHERE;
    }

    auto margin = 40;
    auto cx = WinUtil::GetCursorClientPos(hwnd_, x, y).x;
    if (hit == HTCLIENT) {
        if (cx < margin) {
            panel_->Show();
        }
        else {
            panel_->Hide();
            SetTransparency(true);
        }
    }
    else {
        if (cx < margin) {
            panel_->Show();
        }
    }

    return hit;
}

LRESULT AppWindow::OnCustomDraw(NMCUSTOMDRAW* nmc)
{
    if (nmc->hdr.hwndFrom != close_) {
        return CDRF_SKIPDEFAULT;
    }

    if (nmc->dwDrawStage != CDDS_PREERASE) {
        return CDRF_SKIPDEFAULT;
    }

    auto [cx, cy] = WinUtil::GetClientSize(hwnd_);
    auto size = CaptionSize;

    if (auto state = nmc->uItemState; state & CDIS_SELECTED) {
        listener_->SetCloseButtonState(cx - size, 0, size, size, EButtonState::Pushed);
    }
    else if (state & CDIS_HOT) {
        listener_->SetCloseButtonState(cx - size, 0, size, size, EButtonState::Hover);
    }
    else {
        listener_->SetCloseButtonState(cx - size, 0, size, size, EButtonState::None);
    }

    return CDRF_SKIPDEFAULT;
}

void AppWindow::OnDpiChanged(int dpiX, int dpiY, RECT* rect)
{
    listener_->OnDpiChanged(dpiX, dpiY);

    SetWindowPos(hwnd_, NULL, rect->left, rect->top, rect->right - rect->left, rect->bottom - rect->top, SWP_NOZORDER | SWP_NOACTIVATE);

    auto [cx, cy] = WinUtil::GetClientSize(hwnd_);
    auto size = CaptionSize;
    WinUtil::SetWindowPosSize(close_, cx - size, 0, size, size);

    panel_->Layout();
}

LRESULT CALLBACK AppWindow::WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg) {
	case WM_CLOSE:
        pThis->listener_->OnClose();
		DestroyWindow(hwnd);
        return 0;

	case WM_DESTROY:
        pThis->KillTimer();
        pThis->listener_->OnDestory();
		PostQuitMessage(0);
        return 0;

    case WM_CREATE:
        pThis->hwnd_ = hwnd;
        pThis->OnCreate((LPCREATESTRUCT)lp);
        pThis->listener_->OnCreate(hwnd);
        return 0;

    case WM_MOVE:
        pThis->OnMove(WinUtil::Dpi(LOWORD(lp)), WinUtil::Dpi(HIWORD(lp)));
        return 0;

    case WM_SIZE:
        pThis->OnSize(WinUtil::Dpi(LOWORD(lp)), WinUtil::Dpi(HIWORD(lp)));
        return 0;

    case WM_NCHITTEST:
        return pThis->OnNcHitTest(GET_X_LPARAM(lp), GET_Y_LPARAM(lp));

    case WM_NCCALCSIZE:
        if (wp) {
            auto& rc = reinterpret_cast<LPNCCALCSIZE_PARAMS>(lp)->rgrc[0];
            auto frame = WinUtil::GetFrameSizePix();
            rc.top += 1;
            rc.left += frame.left;
            rc.right -= frame.right;
            rc.bottom -= frame.bottom;
            return 0;
        }
        break;

    case WM_ACTIVATEAPP:
        if (wp == 0) {
            pThis->panel_->Hide();
        }
        break;

    case WM_COMMAND:
        if (auto code = HIWORD(wp); code == BN_CLICKED) {
            SendMessage(hwnd, WM_CLOSE, 0, 0);
        }
        return 0;

    case WM_NOTIFY:
        if (reinterpret_cast<NMHDR*>(lp)->code == NM_CUSTOMDRAW) {
            return pThis->OnCustomDraw((NMCUSTOMDRAW*)lp);
        }
        return 0;

    case WM_DPICHANGED:
        pThis->OnDpiChanged(LOWORD(wp), HIWORD(wp), (RECT*)lp);
        return 0;
	}

    return DefWindowProc(hwnd, msg, wp, lp);
}

