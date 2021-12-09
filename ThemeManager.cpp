#include "pch.h"
#include "ThemeManager.h"
#include <Uxtheme.h>

ThemeManager::ThemeManager()
{
    InitFonts();
}

ThemeManager::~ThemeManager()
{
    if (hGuiFont_) {
        DeleteObject(hGuiFont_);
    }
}

void ThemeManager::EnableAcrylicWindow(HWND hwnd)
{
    // windows Defender gets angry 
    /*
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

    using fnSetWindowCompositionAttribute = BOOL(WINAPI*)(HWND, const WINDOWCOMPOSITIONATTRIBDATA*);

    auto hUser32 = GetModuleHandle(L"User32.dll");
    if (!hUser32) {
        DLOG(L"Failed to GetModuleHandle User32.dll\n");
        return;
    }

    auto SetWindowCompositionAttribute = reinterpret_cast<fnSetWindowCompositionAttribute>(GetProcAddress(hUser32, "SetWindowCompositionAttribute"));
    if (!SetWindowCompositionAttribute) {
        DLOG(L"Failed Get SetWindowCompositionAttribute Addree\n");
        return;
    }

    ACCENT_POLICY accent{};
    accent.AccentState = ACCENT_ENABLE_BLURBEHIND;
    accent.AccentFlags = 0; // 0x20 | 0x40 | 0x80 | 0x100;
    accent.GradientColor = 0x00'00'00'00;
    accent.AnimationId = 0;

    WINDOWCOMPOSITIONATTRIBDATA data{};
    data.Attrib = WCA_ACCENT_POLICY;
    data.pvData = &accent;
    data.cbData = sizeof(accent);
    SetWindowCompositionAttribute(hwnd, &data);
    */
}

HFONT ThemeManager::GetGuiFont()
{
    return hGuiFont_;
}

void ThemeManager::SetGuiFont(HWND hwnd)
{
    SendMessage(hwnd, WM_SETFONT, (WPARAM)hGuiFont_, TRUE);
}

ThemeManager* ThemeManager::GetInstance()
{
    static ThemeManager instance({});
    return &instance;
}

void ThemeManager::InitFonts()
{
    LOGFONT lgGuiFont{};
    auto ret = SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), (PVOID)&lgGuiFont, 0);

    if (ret) {
        hGuiFont_ = CreateFontIndirect(&lgGuiFont);
    }
}