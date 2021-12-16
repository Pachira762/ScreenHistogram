#pragma once

class GuiElement
{
public:
	virtual ~GuiElement() {}
	virtual void	Layout(int x, int y, int width, int height, HDC hdc) {}
	virtual void	Draw(HDC hdc, int scroll) {};

protected:
	RECT	rect_ = {};

public:
	virtual SIZE	GetSize() const {
		return { rect_.right - rect_.left, rect_.bottom - rect_.top };
	}
};

//
// Space
//
class GuiSpace : public GuiElement
{
public:
	GuiSpace(int x, int y, int width, int height);
	virtual void	Layout(int x, int y, int width, int height, HDC hdc)override;
};

//
// Text
//
class GuiText : public GuiElement
{
public:
	GuiText(LPCTSTR text, int x, int y, int width, HDC hdc, bool label = false);
	virtual void	Layout(int x, int y, int width, int height, HDC hdc)override;
	virtual void	Draw(HDC hdc, int scroll)override;

private:
	std::wstring	text_;
	bool			isLabel_;
};

//
// Radio Buttons
//
class GuiRadioButtons : public GuiElement
{
public:
	GuiRadioButtons(HWND parent, int x, int y, int width, std::vector<LPCTSTR> options, int selected = -1);
	virtual void	Layout(int x, int y, int width, int height, HDC hdc)override;

private:
	std::vector<HWND>	radios_ = {};

public:
	const std::vector<HWND>&	GetHandles() const {
		return radios_;
	}
};

//
// Slider
//
class GuiSlider : public GuiElement
{
public:
	GuiSlider(HWND parent, int x, int y, int width, int min = 0, int max = 100, int value = 50);
	virtual void	Layout(int x, int y, int width, int height, HDC hdc)override;

private:
	HWND	slider_ = NULL;

public:
	HWND GetHandle() const {
		return slider_;
	}
};

//
// Layout
//
class GuiLayout
{
public:
	GuiLayout(int marginX, int marginY);
	void	AddElement(std::unique_ptr<GuiElement>&& elem);
	void	Layout(HDC hdc, int width);
	void	Draw(HDC hdc, int scroll);

private:
	std::vector<std::unique_ptr<GuiElement>>	elements_;
	int		marginX_;
	int		marginY_;

public:
	int Height() const {
		int h = marginY_;
		for (auto& elem : elements_) {
			h += elem->GetSize().cy;
		}
		return (h + marginY_);
	}
};
