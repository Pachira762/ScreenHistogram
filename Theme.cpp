#include "pch.h"
#include "Theme.h"
#include <Uxtheme.h>
#include <dwmapi.h>
#include <winrt\Windows.Foundation.Metadata.h>
#include <winrt\Windows.Globalization.Fonts.h>

#pragma comment(lib, "dwmapi.lib")

static Theme* pThis = Theme::Get();

#ifdef ENABLE_ACRYLIC
enum WINDOWCOMPOSITIONATTRIB
{
    WCA_ACCENT_POLICY = 19
};

struct WINDOWCOMPOSITIONATTRIBDATA
{
    WINDOWCOMPOSITIONATTRIB Attrib;
    LPVOID pvData;
    UINT cbData;
};

enum ACCENT_STATE
{
    ACCENT_DISABLED = 0,
    ACCENT_ENABLE_GRADIENT = 1,
    ACCENT_ENABLE_TRANSPARENTGRADIENT = 2,
    ACCENT_ENABLE_BLURBEHIND = 3,
    ACCENT_ENABLE_ACRYLICBLURBEHIND = 4,
    ACCENT_ENABLE_HOSTBACKDROP = 5,
    ACCENT_INVALID_STATE = 6,
};

struct ACCENT_POLICY
{
    ACCENT_STATE    AccentState;
    DWORD           AccentFlags;
    DWORD           GradientColor;
    DWORD           AnimationId;
};
#endif

Theme::Theme()
{
    InitBrushes();
    InitFonts();
    InitCompositionAttribute();
}

Theme::~Theme()
{
    if (Dark::BgBrush) {
        DeleteObject(Dark::BgBrush);
        Dark::BgBrush = nullptr;
    }

    if (CloseHotBrush) {
        DeleteObject(CloseHotBrush);
        TextFont = nullptr;
    }

    if (CloseSelectedBrush) {
        DeleteObject(CloseSelectedBrush);
        TextFont = nullptr;
    }

    if (TextFont) {
        DeleteObject(TextFont);
        TextFont = nullptr;
    }

    if (LabelFont) {
        DeleteObject(LabelFont);
        LabelFont = nullptr;
    }

    if (IconFont) {
        DeleteObject(IconFont);
        IconFont = nullptr;
    }
}

void Theme::SetWindowTheme_DarkMode_Explorer(HWND hwnd)
{
    SetWindowTheme(hwnd, L"DarkMode_Explorer", NULL);
}

void Theme::EnableRoundCorner(HWND hwnd, bool enable)
{
    enum DWMWINDOWATTRIBUTE_Custom
    {
        DWMWA_WINDOW_CORNER_PREFERENCE = 33,
    };

    enum DWM_WINDOW_CORNER_PREFERENCE_Custom
    {
        DWMWCP_DEFAULT = 0,
        DWMWCP_DONOTROUND = 1,
        DWMWCP_ROUND = 2,
        DWMWCP_ROUNDSMALL = 3
    };

    auto attr = enable ? DWMWCP_ROUND : DWMWCP_DONOTROUND;
    DwmSetWindowAttribute(hwnd, DWMWA_WINDOW_CORNER_PREFERENCE, &attr, sizeof(attr));
}

void Theme::EnableBlurBackground(HWND hwnd, DWORD color, DWORD opacity)
{
#ifdef ENABLE_ACRYLIC
    if (!SetWindowCompositionAttribute) {
        return;
    }

    ACCENT_POLICY accent{};
    accent.AccentState = ACCENT_ENABLE_BLURBEHIND;
    accent.AccentFlags = 0; // 0x20 | 0x40 | 0x80 | 0x100;
    accent.GradientColor = color | (opacity << 24);
    accent.AnimationId = 0;

    WINDOWCOMPOSITIONATTRIBDATA data{};
    data.Attrib = WCA_ACCENT_POLICY;
    data.pvData = &accent;
    data.cbData = sizeof(accent);
    SetWindowCompositionAttribute(hwnd, &data);
#else
    DWM_BLURBEHIND bb{};
    bb.dwFlags = DWM_BB_ENABLE;
    bb.fEnable = TRUE;
    bb.hRgnBlur = NULL;

    auto hr = DwmEnableBlurBehindWindow(hwnd, &bb);
    winrt::check_hresult(hr);
#endif
}

void Theme::EnableAcrylicWindow(HWND hwnd, DWORD color, DWORD opacity)
{
#ifdef ENABLE_ACRYLIC
    if (!SetWindowCompositionAttribute) {
        return;
    }

    ACCENT_POLICY accent{};
    accent.AccentState = ACCENT_ENABLE_ACRYLICBLURBEHIND;
    accent.AccentFlags = 0; // 0x20 | 0x40 | 0x80 | 0x100;
    accent.GradientColor = color | (opacity << 24);
    accent.AnimationId = 0;

    WINDOWCOMPOSITIONATTRIBDATA data{};
    data.Attrib = WCA_ACCENT_POLICY;
    data.pvData = &accent;
    data.cbData = sizeof(accent);
    SetWindowCompositionAttribute(hwnd, &data);
#endif
}

Theme* Theme::Get()
{
    static Theme instance({});
    return &instance;
}

void Theme::InitBrushes()
{
    Dark::BgBrush = CreateSolidBrush(Dark::BgColor & 0x00FFFFFF);
   
#ifdef ENABLE_ACRYLIC
    CloseHotBrush = CreateSolidBrush(RGB(0x88,0,0));
    CloseSelectedBrush = CreateSolidBrush(RGB(0xFF, 0x99, 0x99));
#else
    CloseHotBrush = CreateSolidBrush(RGB(232, 17, 35));
    CloseSelectedBrush = CreateSolidBrush(RGB(241, 111, 122));
#endif
}

#if 0
int GetFontWidthByStretch(int height, winrt::Windows::UI::Text::FontStretch stretch)
{
    using namespace winrt::Windows::UI::Text;

    switch (stretch) {
    case FontStretch::Undefined: return height;
    case FontStretch::UltraCondensed: return 0.5 * height;
    case FontStretch::ExtraCondensed: return 0.625 * height;
    case FontStretch::Condensed: return 0.75 * height;
    case FontStretch::SemiCondensed: return 0.875 * height;
    case FontStretch::Normal: return height;
    case FontStretch::SemiExpanded: return 1.125 * height;
    case FontStretch::Expanded: return 1.25 * height;
    case FontStretch::ExtraExpanded: return 1.5 * height;
    case FontStretch::UltraExpanded: return 2 * height;
    default: return height;
    }
}

LOGFONT WinRTFont2LogFont(const winrt::Windows::Globalization::Fonts::LanguageFont& font)
{
    int defaultHeight = -14;

    LOGFONT lg{};
    lg.lfHeight = font.ScaleFactor() * defaultHeight * 0.01;
    lg.lfWidth = 0; // GetFontWidthByStretch(lg.lfHeight, font.FontStretch());
    lg.lfWeight = font.FontWeight().Weight;
    wcscpy_s(lg.lfFaceName, font.FontFamily().c_str());

    return lg;
}
#endif

void Theme::InitFonts()
{
#if 0
    LOGFONT lgfont{};
    auto fontGroup = winrt::Windows::Globalization::Fonts::LanguageFontGroup(L"ja-JP");

    auto uiTextFont = WinRTFont2LogFont(fontGroup.UITextFont());
    hTextFont_ = CreateFontIndirect(&uiTextFont);

    auto uiHeadingFont = WinRTFont2LogFont(fontGroup.UIHeadingFont());
    hLabelFont_ = CreateFontIndirect(&uiHeadingFont);
#else
    LOGFONT lgfont{};
    if (SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(lgfont), &lgfont, 0)) {
        FontName = lgfont.lfFaceName;
        FontSize = lgfont.lfHeight *= 1.2;

        auto textFont = lgfont;
        textFont.lfHeight *= 1.2;
        TextFont = CreateFontIndirect(&textFont);

        auto labelFont = lgfont;
        labelFont.lfHeight *= 1.2;
        labelFont.lfWeight = FW_MEDIUM;
        LabelFont = CreateFontIndirect(&labelFont);
    }

    LOGFONT iconFont = {};
    iconFont.lfHeight = 14;
    iconFont.lfWeight = FW_REGULAR;
    wcscpy_s(iconFont.lfFaceName, L"Segoe MDL2 Assets");
    IconFont = CreateFontIndirect(&iconFont);

#endif
}

void Theme::InitCompositionAttribute()
{
#ifdef ENABLE_ACRYLIC
    auto hUser32 = GetModuleHandle(L"User32.dll");
    if (!hUser32) {
        return;
    }

    SetWindowCompositionAttribute = reinterpret_cast<fnSetWindowCompositionAttribute>(GetProcAddress(hUser32, "SetWindowCompositionAttribute"));
    if (!SetWindowCompositionAttribute) {
        return;
    }
#endif
}
