#include "AppWindow.h"
#include "common.h"
#include "WindowCommon.h"
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

    InitCommonControls();

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

    RECT rc{};
    GetWindowRect(hwnd_, &rc);

    auto builder = panel_->Create(hwnd_);
    builder->AddLabel(L"Histogram");
    builder->AddRadioButtons({ L"RGB", L"RGB Brightness", L"Brightness", L"Saturation" }, (int)histogramMode, [this](int index) {
        this->listener_->SetHistogramMode((EHistogramMode)index);
        });
    builder->AddBlank(12);
    builder->AddLabel(L"View");
    builder->AddRadioButtons({ L"Color", L"Brightness", L"Saturation" }, (int)viewMode, [this](int index) {
        this->listener_->SetViewMode((EViewMode)index);
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
    builder->AddRadioButtons({ L"None", L"RGB",L"HSV" }, 0, [this](int value) {
        this->listener_->SetColorPickMode((EColorPickMode)value);
        });
    builder->Build();

    return hwnd_;
}

void AppWindow::Show()
{
    ShowWindow(hwnd_, SW_SHOW);
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
    SetWindowPos(hwnd_, NULL, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
    SetLayeredWindowAttributes(hwnd_, 0, 255, LWA_ALPHA);
    SetWindowDisplayAffinity(hwnd_, WDA_EXCLUDEFROMCAPTURE);
    SetTimer(hwnd_, 1, 100, NULL);

    auto [cw, ch] = GetClientSize(hwnd_);
    close_ = CreateWindowEx(0, L"BUTTON", L"X", WS_VISIBLE|WS_CHILD|BS_DEFPUSHBUTTON,
        cw - CloseButtonSize, 0, CloseButtonSize, CloseButtonSize, hwnd_, NULL, GetModuleHandle(NULL), NULL);
    WINRT_VERIFY(close_);
}

void AppWindow::OnMove(int x, int y)
{
    panel_->Move(x, y);
}

void AppWindow::OnSize(int cx, int cy)
{
    SetWindowPos(close_, NULL, cx - CloseButtonSize, 0, CloseButtonSize, CloseButtonSize, SWP_NOZORDER);

    panel_->Size(cx, cy);
}

LRESULT AppWindow::OnNcHitTest(int x, int y)
{
    const auto hit = NonClientHitTest(hwnd_, x, y);
    if (hit == HTNOWHERE) {
        return HTNOWHERE;
    }

    auto cx = ScreenToClient(hwnd_, x, y).x;
    if (hit == HTCLIENT) {
        if (cx < 40) {
            panel_->Show();
        }
        else {
            panel_->Hide();
            SetTransparency(true);
        }
    }
    else {
        if (cx < 40) {
            panel_->Show();
        }
    }

    return hit;
}

void AppWindow::OnCheckTransparency()
{
    if (transparency_) {
        auto[x, y] = GetCursorPos();
        if (NonClientHitTest(hwnd_, x, y) != HTCLIENT) {
            SetTransparency(false);
        }
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

    case WM_MOVE:
        pThis->OnMove(LOWORD(lp), HIWORD(lp));
        break;

    case WM_SIZE:
        pThis->OnSize(LOWORD(lp), HIWORD(lp));
        break;

    case WM_NCHITTEST:
        return pThis->OnNcHitTest(GET_X_LPARAM(lp), GET_Y_LPARAM(lp));

    case WM_NCCALCSIZE:
        if (wp) {
            auto& rc = reinterpret_cast<LPNCCALCSIZE_PARAMS>(lp)->rgrc[0];
            rc.top += 1;
            rc.left += GetSystemMetrics(SM_CXFRAME);
            rc.right -= GetSystemMetrics(SM_CXFRAME);
            rc.bottom -= GetSystemMetrics(SM_CYFRAME);
            return 0;
        }
        return DefWindowProc(hwnd, msg, wp, lp);

    case WM_ACTIVATEAPP:
        if (wp == 0) {
            pThis->panel_->Hide();
        }
        break;

    case WM_TIMER:
        pThis->OnCheckTransparency();
        break;

    case WM_COMMAND:
        if (auto code = HIWORD(wp); code == BN_CLICKED) {
            SendMessage(hwnd, WM_CLOSE, 0, 0);
        }
        return 0;

    case WM_NOTIFY:
        if (reinterpret_cast<NMHDR*>(lp)->code == NM_CUSTOMDRAW) {
            auto nmc = reinterpret_cast<NMCUSTOMDRAW*>(lp);

            if (nmc->dwDrawStage != CDDS_PREERASE){
                return CDRF_SKIPDEFAULT;
            }

            auto close = nmc->hdr.hwndFrom;
            auto [sx, sy] = ClientToScreen(close, 0, 0);
            auto [cx, cy] = ScreenToClient(pThis->hwnd_, sx, sy);
            auto [cw, ch] = GetClientSize(close);

            if (auto state = nmc->uItemState; state & CDIS_SELECTED) {
                pThis->listener_->SetCloseButtonState(cx, cy, cw, ch, EButtonState::Pushed);
            }
            else if (state & CDIS_HOT) {
                pThis->listener_->SetCloseButtonState(cx, cy, cw, ch, EButtonState::Hover);
            }
            else {
                pThis->listener_->SetCloseButtonState(cx, cy, cw, ch, EButtonState::None);
            }

            return CDRF_SKIPDEFAULT;
        }
        return 0;

	default:
		return DefWindowProc(hwnd, msg, wp, lp);
	}

	return 0;
}

