#include "pch.h"
#include "OptionPanel.h"
#include "GuiLayout.h"
#include "WinUtil.h"
#include "Theme.h"

using namespace std;

static OptionPanel* pThis = nullptr;

OptionPanel::OptionPanel():
    layout_(make_unique<GuiLayout>(MarginX, MarginY))
{
    pThis = this;
}

void OptionPanel::Show()
{
    ShowWindow(hwnd_, SW_SHOW);
}

void OptionPanel::Hide()
{
    ShowWindow(hwnd_, SW_HIDE);
}

void OptionPanel::Move()
{
    auto [x, y] = CalcFramePos();
    WinUtil::SetWindowPos(hwnd_, x, y);
}

void OptionPanel::Size()
{
    auto [x, y] = CalcFramePos();
    auto [w, h] = CalcFrameSize();
    WinUtil::SetWindowPosSize(hwnd_, x, y, w, h, SWP_NOZORDER | SWP_NOACTIVATE);
}

std::shared_ptr<IGuiBuilder> OptionPanel::Create(HWND parent)
{
    parent_ = parent;
    
    WNDCLASSEX wcex = {};
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = 0;
    wcex.lpfnWndProc = OptionPanel::FrameWndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = GetModuleHandle(NULL);
    wcex.hIcon = NULL;
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = Theme::Dark::BgBrush;
    wcex.lpszClassName = L"ScreenHistogramOptionPanelFrameWnd";
    WINRT_VERIFY(RegisterClassEx(&wcex));

    auto [x, y] = CalcFramePos();
    auto [w, h] = CalcFrameSize();
    hwnd_ = WinUtil::Create(L"ScreenHistogramOptionPanelFrameWnd", NULL, WS_VISIBLE | WS_POPUP | WS_VSCROLL, WS_EX_LAYERED,
        x, y, w, h, parent);

    auto [cx, cy] = WinUtil::GetClientSize(hwnd_);
    auto builder = make_shared<GuiBuilderImpl>((HWND)hwnd_, MarginX, MarginY, cx - MarginX - VScrollWidth, layout_);

    builder->OnRadioGroupAdded = [this](const std::vector<HWND>& handles, const RadioButtonCallback& callback) {
        radioGroups_.push_back(RadioButtonGroup{ handles, callback });
        customDrawTargets_.insert(handles.begin(), handles.end());
    };

    builder->OnCheckboxGroupAddes = [this](const std::vector<HWND>& handles, const CheckboxCallback& callback) {
        checkboxGroups_.push_back(CheckboxGroup{ handles, callback });
        customDrawTargets_.insert(handles.begin(), handles.end());
    };

    builder->OnSliderAdded = [this](HWND handle, const SliderCallback& callback) {
        sliders_.insert({ handle, callback });
    };

    builder->OnBuild = [this](GuiBuilderImpl* builder, int contentWidth, int contentHeight) {
        auto hdc = GetDC(hwnd_);
        auto [cx, cy] = WinUtil::GetClientSize(hwnd_);
        layout_->Layout(hdc, cx - MarginX - VScrollWidth);
        ReleaseDC(hwnd_, hdc);

        UpdateScrollRange(layout_->Height());
        UpdateScrollPage(WinUtil::GetClientSize(this->hwnd_).cy);
    };

	return builder;
}

void OptionPanel::UpdateScrollRange(int range)
{
    SCROLLINFO si{};
    si.cbSize = sizeof(si);
    si.fMask = SIF_POS;
    GetScrollInfo(hwnd_, SB_VERT, &si);

    const int pos = si.nPos;

    si.fMask = SIF_RANGE;
    si.nMax = WinUtil::Pix(range);
    SetScrollInfo(hwnd_, SB_VERT, &si, TRUE);

    si.fMask = SIF_POS;
    GetScrollInfo(hwnd_, SB_VERT, &si);

    ScrollContent(pos - si.nPos);
}

void OptionPanel::UpdateScrollPage(int page)
{
    SCROLLINFO si{};
    si.cbSize = sizeof(si);
    si.fMask = SIF_POS;
    GetScrollInfo(hwnd_, SB_VERT, &si);

    const int pos = si.nPos;

    si.fMask = SIF_PAGE;
    si.nPage = WinUtil::Pix(page);
    SetScrollInfo(hwnd_, SB_VERT, &si, TRUE);

    si.fMask = SIF_POS;
    GetScrollInfo(hwnd_, SB_VERT, &si);

    ScrollContent(pos - si.nPos);
}

void OptionPanel::ScrollContent(int dy)
{
    scroll_ += dy;
    ScrollWindowEx(hwnd_, 0, dy, nullptr, nullptr, nullptr, nullptr, SW_ERASE | SW_INVALIDATE | SW_SCROLLCHILDREN);
    UpdateWindow(hwnd_);
}

void OptionPanel::OnCreate(LPCREATESTRUCT cs)
{
    
    Theme::SetWindowTheme_DarkMode_Explorer(hwnd_);
    Theme::EnableRoundCorner(hwnd_);
   
    if (Theme::IsEnabledAcrylic) {
        Theme::EnableAcrylicWindow(hwnd_);
    }

    SetLayeredWindowAttributes(hwnd_, 0, 255, LWA_ALPHA);
    SetWindowDisplayAffinity(hwnd_, WDA_EXCLUDEFROMCAPTURE);
}

void OptionPanel::OnSize(int cx, int cy)
{
    auto page = WinUtil::GetClientSize(this->hwnd_).cy;
    UpdateScrollPage(page);
}

void OptionPanel::OnPaint()
{
    int y = WinUtil::Dpi(-scroll_);

    PAINTSTRUCT ps{};
    HDC hdc = BeginPaint(hwnd_, &ps);

    if (Theme::IsEnabledAcrylic) {
        FillRect(hdc, &ps.rcPaint, (HBRUSH)GetStockObject(BLACK_BRUSH));
    }

    SetBkMode(hdc, TRANSPARENT);

    layout_->Draw(hdc, y);

    EndPaint(hwnd_, &ps);
}

void OptionPanel::OnHScroll(HWND scroll)
{
    if (auto itr = sliders_.find(scroll); itr != sliders_.end()) {
        itr->second((int)SendMessage(scroll, TBM_GETPOS, 0, 0));
    }
}

void OptionPanel::OnVScroll(int code, int delta)
{
    SCROLLINFO si{};
    si.cbSize = sizeof(si);
    si.fMask = SIF_ALL;
    GetScrollInfo(hwnd_, SB_VERT, &si);

    int pos = si.nPos;
    auto line = delta == 0 ? 1 : delta;

    switch (code) {
    case SB_LINEUP:
        si.nPos -= line;
        break;

    case SB_LINEDOWN:
        si.nPos += line;
        break;

    case SB_PAGEUP:
        si.nPos -= si.nPage;
        break;

    case SB_PAGEDOWN:
        si.nPos += si.nPage;
        break;

    case SB_THUMBTRACK:
        si.nPos = si.nTrackPos;
        break;

    case SB_TOP:
        si.nPos = si.nMin;
        break;

    case SB_BOTTOM:
        si.nPos = si.nMax;
        break;

    default:
        return;
    }

    si.fMask = SIF_POS;
    SetScrollInfo(hwnd_, SB_VERT, &si, TRUE);
    GetScrollInfo(hwnd_, SB_VERT, &si);

    auto dy = (pos - si.nPos);
    if (dy == 0) {
        return;
    }

    ScrollContent(dy);
}

void OptionPanel::OnCommand(int id, int code, HWND handle)
{
    switch (code) {
    case BN_CLICKED:
        OnButtonClicked(handle, id);
        break;
    }
}

static bool IsInGroup(std::vector<HWND>& group, HWND hwnd, int index)
{
    if (index < group.size() && hwnd == group.at(index)) {
        return true;
    }

    return false;
}

void OptionPanel::OnButtonClicked(HWND hwnd, int id)
{
    for (auto& [radios, callback] : radioGroups_) {
        if (!callback) {
            continue;
        }

        if (IsInGroup(radios, hwnd, id)) {
            callback(id);
            return;
        }
    }

    for (auto& [checkboxes, callback] : checkboxGroups_) {
        if (!callback) {
            continue;
        }

        if (IsInGroup(checkboxes, hwnd, id)) {
            vector<bool> checked{};
            for (auto& checkbox : checkboxes) {
                checked.push_back(static_cast<bool>(SendMessage(checkbox, BM_GETCHECK, 0, 0)));
            }
            callback(checked);
            return;
        }
    }
}

LRESULT OptionPanel::OnNotify(DWORD id, NMHDR* hdr)
{
    switch (hdr->code) {
    case  NM_CUSTOMDRAW:
        return OnCustomDraw((NMCUSTOMDRAW*)hdr);

    default:
        return 0;
    }
}

LRESULT OptionPanel::OnCustomDraw(NMCUSTOMDRAW* nmc)
{
    static TCHAR buff[256];
    const auto hwnd = nmc->hdr.hwndFrom;

    if (customDrawTargets_.find(hwnd) != customDrawTargets_.end()) {
        if (nmc->dwDrawStage == CDDS_PREERASE) {
            return CDRF_NOTIFYPOSTERASE;
        }
        else if (nmc->dwDrawStage != CDDS_PREPAINT) {
            return CDRF_DODEFAULT;
        }

        GetWindowText(nmc->hdr.hwndFrom, buff, _countof(buff));

        auto hdc = nmc->hdc;
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, Theme::Dark::TextColor);
        SelectObject(hdc, Theme::TextFont);

        nmc->rc.left += WinUtil::Pix(17);
        DrawText(hdc, buff, -1, &nmc->rc, DT_VCENTER | DT_SINGLELINE);
        return CDRF_SKIPDEFAULT;
    }
    else {
        return CDRF_DODEFAULT;
    }
}

LRESULT CALLBACK OptionPanel::FrameWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_CREATE:
        pThis->hwnd_ = hwnd;
        pThis->OnCreate((LPCREATESTRUCT)lp);
        return 0;

    case WM_SIZE:
        if (wp != 1) {
            pThis->OnSize(WinUtil::Dpi(LOWORD(lp)), WinUtil::Dpi(HIWORD(lp)));
        }
        return 0;

    case WM_PAINT:
        pThis->OnPaint();
        return 0;

    case WM_NCHITTEST:
        if (DefWindowProc(hwnd, msg, wp, lp) == HTVSCROLL) {
            return HTVSCROLL;
        }
        else {
            if (WinUtil::NonClientHitTest(pThis->parent_, GET_X_LPARAM(lp), GET_Y_LPARAM(lp)) == HTCLIENT) {
                return HTCLIENT;
            }
            else {
                return HTTRANSPARENT;
            }
        }

    case WM_COMMAND:
        pThis->OnCommand(LOWORD(wp), HIWORD(wp), (HWND)lp);
        return 0;

    case WM_NOTIFY:
        return pThis->OnNotify(wp, (NMHDR*)lp);

    case WM_HSCROLL:
        pThis->OnHScroll((HWND)lp);
        return 0;

    case WM_VSCROLL:
        pThis->OnVScroll(LOWORD(wp), HIWORD(wp));
        return 0;

    case WM_MOUSEWHEEL:
        if (auto wheel = GET_WHEEL_DELTA_WPARAM(wp); wheel > 0){
            SendMessage(pThis->hwnd_, WM_VSCROLL, MAKEWPARAM(SB_LINEUP, wheel / 10), 0);
        }
        else {
            SendMessage(pThis->hwnd_, WM_VSCROLL, MAKEWPARAM(SB_LINEDOWN, abs(wheel) / 10), 0);
        }
        return 0;

    case WM_CTLCOLORBTN:
    case WM_CTLCOLORSTATIC:
        return (LRESULT)(Theme::IsEnabledAcrylic ? GetStockObject(BLACK_BRUSH) : Theme::Dark::BgBrush);
    }

    return DefWindowProc(hwnd, msg, wp, lp);
}
