#include "WindowCommon.h"
#include "Theme.h"

int GetScrollPos(HWND scroll, int bar)
{
    SCROLLINFO si{};
    si.cbSize = sizeof(si);
    si.fMask = SIF_POS;
    GetScrollInfo(scroll, bar, &si);
    return si.nPos;
}

int VScrollProc(HWND scroll, WPARAM wp, int bar)
{
    SCROLLINFO si{};
    si.cbSize = sizeof(si);
    si.fMask = SIF_ALL;
    GetScrollInfo(scroll, bar, &si);

    int pos = si.nPos;
    auto line = GET_WHEEL_DELTA_WPARAM(wp) == 0 ? 1 : HIWORD(wp);

    switch (LOWORD(wp)) {
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
        return 0;
    }

    si.fMask = SIF_POS;
    SetScrollInfo(scroll, bar, &si, TRUE);
    GetScrollInfo(scroll, bar, &si);

    return (pos - si.nPos);
}

int UpdateScroll(HWND scroll, int page, int range, int bar)
{
    SCROLLINFO si{};
    si.cbSize = sizeof(si);
    si.fMask = SIF_POS | SIF_RANGE;
    GetScrollInfo(scroll, bar, &si);

    int pos= GetScrollPos(scroll);

    si.fMask = (SIF_PAGE | (range > 0 ? SIF_RANGE : 0));
    si.nMin = 0;
    si.nMax = range;
    si.nPage = page;
    SetScrollInfo(scroll, bar, &si, TRUE);

    si.fMask = SIF_POS;
    GetScrollInfo(scroll, bar, &si);

    return (pos - si.nPos);
}

LRESULT	DrawRadioButton(NMCUSTOMDRAW* nmc, int dpi)
{
    static TCHAR buff[256];

    if (nmc->dwDrawStage == CDDS_PREERASE) {
        return CDRF_NOTIFYPOSTERASE;
    }
    else if (nmc->dwDrawStage != CDDS_PREPAINT) {
        return CDRF_DODEFAULT;
    }

    auto hdc = nmc->hdc;
    SetBkMode(hdc, TRANSPARENT);

    SetTextColor(hdc, Theme::Dark::TextColor);
    SelectObject(hdc, Theme::TextFont);

    nmc->rc.left += DPISCALE(17, dpi);
    GetWindowText(nmc->hdr.hwndFrom, buff, _countof(buff));
    DrawText(hdc, buff, -1, &nmc->rc, DT_VCENTER | DT_SINGLELINE);

    return CDRF_SKIPDEFAULT;
}

