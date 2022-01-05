#include "pch.h"
#include "GuiBuilder.h"
#include "Theme.h"
#include "WinUtil.h"
#include "GuiLayout.h"

using namespace std;

GuiBuilderImpl::GuiBuilderImpl(HWND hwnd, int marginX, int marginY, int width, std::unique_ptr<GuiLayout>& layout) :
    hwnd_(hwnd),
    hdc_(GetDC(hwnd)),
    layout_(layout)
{
    x_ = marginX;
    y_ = marginY;
    width_ = width;
}

GuiBuilderImpl::~GuiBuilderImpl()
{
    ReleaseDC(hwnd_, hdc_);
}

void GuiBuilderImpl::Build()
{
    if (OnBuild) {
        OnBuild(this, width_, layout_->Height());
    }
}

void GuiBuilderImpl::AddBlank(int height)
{
    auto space = make_unique<GuiSpace>(x_, y_, width_, height);
    y_ += space->GetSize().cy;
    layout_->AddElement(move(space));

}

void GuiBuilderImpl::AddText(LPCTSTR text)
{
    auto tex = make_unique<GuiText>(text, x_, y_, width_, hdc_, false);
    y_ += tex->GetSize().cy;
    layout_->AddElement(move(tex));

    if (OnTextAdded) {
        OnTextAdded(text);
    }
}

void GuiBuilderImpl::AddLabel(LPCTSTR text)
{
    auto tex = make_unique<GuiText>(text, x_, y_, width_, hdc_, true);
    y_ += tex->GetSize().cy;
    layout_->AddElement(move(tex));

    if (OnTextAdded) {
        OnTextAdded(text);
    }
}

void GuiBuilderImpl::AddRadioButtons(const std::vector<LPCTSTR>& options, int initial, const RadioButtonCallback& callback)
{
    auto radios = make_unique<GuiRadioButtons>(hwnd_, x_, y_, width_, options, initial);
    y_ += radios->GetSize().cy;

    auto handles = radios->GetHandles();
    layout_->AddElement(move(radios));

    if (OnRadioGroupAdded) {
        OnRadioGroupAdded(handles, callback);
    }
}

void GuiBuilderImpl::AddCheckboxes(const std::vector<LPCTSTR>& options, int initial, const CheckboxCallback& callback)
{
    auto checks = make_unique<GuiCheckboxes>(hwnd_, x_, y_, width_, options);
    y_ += checks->GetSize().cy;

    auto handles = checks->GetHandles();

    layout_->AddElement(move(checks));

    if (OnCheckboxGroupAddes) {
        OnCheckboxGroupAddes(handles, callback);
    }
}

void GuiBuilderImpl::AddSlider(int min, int max, int initial, const SliderCallback& callback)
{
    auto slider = make_unique<GuiSlider>(hwnd_, x_, y_, width_, min, max, initial);
    y_ += slider->GetSize().cy;
    auto handle = slider->GetHandle();
    layout_->AddElement(move(slider));

    if (OnSliderAdded) {
        OnSliderAdded(handle, callback);
    }
}

HWND GuiBuilderImpl::CreateScrollBar(HWND hwnd, int x, int y, int width, int height, int page, int content, bool darkmode)
{
    bool show = page < content;

    HWND scroll = CreateWindowEx(0, L"SCROLLBAR", NULL, WS_CHILD | SBS_VERT | (show ? WS_VISIBLE : 0),
        x, y, width, height, hwnd, NULL, GetModuleHandle(NULL), NULL);
    WINRT_VERIFY(scroll);

    SCROLLINFO info{};
    info.cbSize = sizeof(info);
    info.fMask = SIF_RANGE | SIF_PAGE;
    info.nMin = 0;
    info.nMax = content;
    info.nPage = page;
    SetScrollInfo(scroll, SB_CTL, &info, TRUE);

    if (darkmode) {
        SetWindowTheme(scroll, L"DarkMode_Explorer", NULL);
    }

    return scroll;
}
