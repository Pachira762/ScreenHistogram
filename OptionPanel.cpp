#include "pch.h"
#include "OptionPanel.h"
#include "WindowCommon.h"
#include "Theme.h"

using namespace std;

static OptionPanel* pThis = nullptr;

static inline std::tuple<int, int, int, int> CalcFramePosSize(HWND parent)
{
    auto [x, y] = ClientToScreen(parent, 0, 0);
    auto [cx, cy] = GetClientSize(parent);
    return { x, y, PanelWidth, cy };
}

static inline std::tuple<int, int, int, int> CalcContentPosSize(HWND frame)
{
    auto [cx, cy] = GetClientSize(frame);
    auto th = TitleHeight;
    auto sw = 0;
    return { 0, 0, cx, cy};
}

static inline std::tuple<int, int, int, int> CalcScrollbarPositionSize(HWND hwnd)
{
    auto [cx, cy] = GetClientSize(hwnd);
    return { cx - VScrollWidth - 2, 0, VScrollWidth, cy };
}

OptionPanel::OptionPanel()
{
    pThis = this;
}

void OptionPanel::Show()
{
    ShowWindow(frame_, SW_SHOW);
}

void OptionPanel::Hide()
{
    ShowWindow(frame_, SW_HIDE);
}

void OptionPanel::Move(int, int)
{
    auto [x, y, w, h] = CalcFramePosSize(parent_);
    SetWindowPos(frame_, NULL, x, y, w, h, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
}

void OptionPanel::Size(int , int )
{
    auto [x, y, w, h] = CalcFramePosSize(parent_);
    SetWindowPos(frame_, NULL, x, y, w, h, SWP_NOZORDER | SWP_NOACTIVATE);
}

std::shared_ptr<IGuiBuilder> OptionPanel::Create(HWND parent)
{
    instance_ = GetModuleHandle(NULL);
    parent_ = parent;
    
    WNDCLASSEX wcex = {};
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = OptionPanel::FrameWndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = instance_;
    wcex.hIcon = NULL;
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = Theme::Dark::BgBrush;
    wcex.lpszClassName = L"ScreenHistogramOptionPanelFrameWnd";
    WINRT_VERIFY(RegisterClassEx(&wcex));

    auto [x, y, w, h] = CalcFramePosSize(parent_);
    frame_ = CreateWindowEx(0, L"ScreenHistogramOptionPanelFrameWnd", NULL, WS_VISIBLE | WS_POPUP | WS_VSCROLL,
        x, y, w, h, parent, NULL, instance_, NULL);
    WINRT_VERIFY(frame_);

    auto builder = make_shared<GuiBuilderImpl>((HWND)frame_, RECT{12, 12, VScrollWidth + 4, 12});

    builder->OnTextAdded = [this](LPCTSTR text, const RECT& layout) {
        texts_.push_back({layout, text});
    };

    builder->OnLabelAdded = [this](LPCTSTR text, const RECT& layout) {
        labels_.push_back({ layout, text });
    };

    builder->OnRadioGroupAdded = [this](const std::vector<HWND>& handles, const RadioButtonCallback& callback) {
        radioGroups_.push_back(RadioButtonGroup{ handles, callback });
        customDrawRadioButtons_.insert(handles.begin(), handles.end());
    };

    builder->OnSliderAdded = [this](HWND handle, const SliderCallback& callback) {
        sliders_.insert({ handle, callback });
    };

    builder->OnBuild = [this](GuiBuilderImpl* builder, int contentWidth, int contentHeight) {
        UpdateScroll(frame_, GetClientSize(frame_).cy, contentHeight);
    };

	return builder;
}

void OptionPanel::CheckRadioButtons(HWND radio, int index)
{
    for (const auto&[radios, callback] : radioGroups_) {
        if (index < radios.size() && radio == radios.at(index) && callback) {
            callback(index);
            return;
        }
    }
}

void OptionPanel::OnCreate(LPCREATESTRUCT cs)
{
    Theme::SetWindowTheme_DarkMode_Explorer(frame_);
    Theme::EnableRoundCorner(frame_);
   
    if (Theme::IsEnabledAcrylic) {
        Theme::EnableAcrylicWindow(frame_);
    }

    SetWindowDisplayAffinity(frame_, WDA_EXCLUDEFROMCAPTURE);
}

void OptionPanel::OnSize(int cx, int cy)
{
    if (auto dy = UpdateScroll(frame_, cy); dy != 0) {
        ScrollWindow(frame_, 0, dy, nullptr, nullptr);
        UpdateWindow(frame_);
    }
}

void OptionPanel::OnPaint()
{
    int y = GetScrollPos(frame_);

    PAINTSTRUCT ps{};
    HDC hdc = BeginPaint(frame_, &ps);

    if (Theme::IsEnabledAcrylic) {
        FillRect(hdc, &ps.rcPaint, (HBRUSH)GetStockObject(BLACK_BRUSH));
    }

    SetBkMode(hdc, TRANSPARENT);

    SelectObject(hdc, Theme::TextFont);
    SetTextColor(hdc, Theme::Dark::TextColor);
    for (auto& [layout, text] : texts_) {
        auto rc = layout;
        rc.top -= y;
        rc.bottom -= y;
        DrawText(hdc, text.c_str(), -1, &rc, 0);
    }

    SelectObject(hdc, Theme::LabelFont);
    SetTextColor(hdc, Theme::Dark::AccentTextColor);
    for (auto& [layout, text] : labels_) {
        auto rc = layout;
        rc.top -= y;
        rc.bottom -= y;
        DrawText(hdc, text.c_str(), text.length(), &rc, DT_SINGLELINE | DT_VCENTER);
    }

    EndPaint(frame_, &ps);
}

void OptionPanel::OnHScroll(HWND scroll)
{
    if (auto itr = sliders_.find(scroll); itr != sliders_.end()) {
        itr->second((int)SendMessage(scroll, TBM_GETPOS, 0, 0));
    }
}

void OptionPanel::OnCommand(int id, int code, HWND handle)
{
    switch (code) {
    case BN_CLICKED:
        CheckRadioButtons(handle, id);
        break;
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
    const auto hwnd = nmc->hdr.hwndFrom;

    if (customDrawRadioButtons_.find(hwnd) != customDrawRadioButtons_.end()) {
        return DrawRadioButton(nmc);
    }
    else {
        return CDRF_DODEFAULT;
    }
}

LRESULT CALLBACK OptionPanel::FrameWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_CREATE:
        pThis->frame_ = hwnd;
        pThis->OnCreate((LPCREATESTRUCT)lp);
        break;

    case WM_SIZE:
        if (wp != 1) {
            pThis->OnSize(LOWORD(lp), HIWORD(lp));
        }
        break;

    case WM_PAINT:
        pThis->OnPaint();
        break;

    case WM_NCHITTEST:
        if (DefWindowProc(hwnd, msg, wp, lp) == HTVSCROLL) {
            return HTVSCROLL;
        }
        else {
            if (NonClientHitTest(pThis->parent_, GET_X_LPARAM(lp), GET_Y_LPARAM(lp)) == HTCLIENT) {
                return HTCLIENT;
            }
            else {
                return HTTRANSPARENT;
            }
        }

    case WM_COMMAND:
        pThis->OnCommand(LOWORD(wp), HIWORD(wp), (HWND)lp);
        break;

    case WM_NOTIFY:
        return pThis->OnNotify(wp, (NMHDR*)lp);

    case WM_HSCROLL:
        pThis->OnHScroll((HWND)lp);
        break;

    case WM_VSCROLL:
        if (auto dy = VScrollProc(hwnd, wp); dy != 0) {
            ScrollWindow(hwnd, 0, dy, nullptr, nullptr);
            UpdateWindow(hwnd);
        }
        break;

    case WM_MOUSEWHEEL:
        if (auto wheel = GET_WHEEL_DELTA_WPARAM(wp); wheel > 0){
            SendMessage(pThis->frame_, WM_VSCROLL, MAKEWPARAM(SB_LINEUP, wheel / 10), 0);
        }
        else {
            SendMessage(pThis->frame_, WM_VSCROLL, MAKEWPARAM(SB_LINEDOWN, abs(wheel) / 10), 0);
        }
        return 0;

    case WM_CTLCOLORBTN:
    case WM_CTLCOLORSTATIC:
        return (LRESULT)(Theme::IsEnabledAcrylic ? GetStockObject(BLACK_BRUSH) : Theme::Dark::BgBrush);

    default:
        return DefWindowProc(hwnd, msg, wp, lp);
    }

    return 0;
}
