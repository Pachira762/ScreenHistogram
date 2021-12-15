#pragma once

using RadioButtonCallback = std::function<void(int id)>;
using SliderCallback = std::function<void(int value)>;

class IGuiBuilder
{
public:
	virtual ~IGuiBuilder() {}

	virtual void	Build() = 0;
	virtual void	AddBlank(int height) = 0;
	virtual void	AddText(LPCTSTR text) = 0;
	virtual void	AddLabel(LPCTSTR label) = 0;
	virtual void	AddRadioButtons(const std::vector<LPCTSTR>& options, int initial, const RadioButtonCallback& callback) = 0;
	virtual void	AddSlider(int min, int max, int initial, const SliderCallback& callback) = 0;

protected:
	IGuiBuilder() {}
};

class GuiBuilderImpl : public IGuiBuilder
{
public:
	GuiBuilderImpl(HWND hwnd, const RECT& margin, int dpi);
	~GuiBuilderImpl();

	virtual void	Build()override;
	virtual void	AddBlank(int height)override;
	virtual void	AddText(LPCTSTR text)override;
	virtual void	AddLabel(LPCTSTR label)override;
	virtual void	AddRadioButtons(const std::vector<LPCTSTR>& options, int initial, const RadioButtonCallback& callback)override;
	virtual void	AddSlider(int min, int max, int initial, const SliderCallback& callback)override;

public:
	std::function<void(LPCTSTR text, const RECT& layout)>	OnTextAdded = {};
	std::function<void(LPCTSTR text, const RECT& layout)>	OnLabelAdded = {};
	std::function<void(const std::vector<HWND>& handles, const RadioButtonCallback& callback)>	OnRadioGroupAdded = {};
	std::function<void(HWND handle, const SliderCallback& callback)> OnSliderAdded = {};
	std::function<void(GuiBuilderImpl* builder, int contentWidth, int contentHeight)>	OnBuild = {};

	HWND	CreateCloseButton(HWND hwnd);
	HWND	CreateScrollBar(HWND hwnd, int x, int y, int width, int height, int page, int content, bool darkmode);

private:
	HINSTANCE	instance_ = GetModuleHandle(NULL);
	HWND		hwnd_ = NULL;
	RECT		tree_ = {};
	HDC			hdc_ = {};
	int			dpi_ = 96;
};
