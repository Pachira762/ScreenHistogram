#pragma once
#include "common.h"

class AppWindowListener
{
public:
	virtual ~AppWindowListener() {}

	virtual	void	OnCreate(HWND hwnd) = 0;
	virtual void	OnClose() = 0;
	virtual void	OnDestory() = 0;
	virtual void	OnSizing() = 0;
	virtual void	OnMoving() = 0;
	virtual void	OnMinimized() = 0;

	virtual void	SetHistogramMode(EHistogramMode mode) = 0;
	virtual void	SetViewMode(EViewMode mode) = 0;
	virtual void	SetScale(float scale) = 0;
	virtual void	SetOpacity(float opacity) = 0;
	virtual void	SetColorPickMode(EColorPickMode mode) = 0;

	virtual void	SetCloseButtonState(int cx, int cy, int cw, int ch, EButtonState state) = 0;
};

class AppWindow
{
public:
	AppWindow(AppWindowListener* listener);
	~AppWindow();

	HWND	Create(int x, int y, int width, int height, EHistogramMode histogramMode, EViewMode viewMode, float scale, float opacity);
	void	Show();

private:
	AppWindowListener* listener_;
	std::unique_ptr<class OptionPanel>	panel_;

	HWND	hwnd_{};
	HWND	close_ = {};
	DWORD	style_;
	DWORD	styleEx_;
	bool	transparency_ = false;
	

	void	SetTransparency(bool transparency);

	void	OnCreate(LPCREATESTRUCT cs);
	void	OnMove(int x, int y);
	void	OnSize(int cx, int cy);
	LRESULT	OnNcHitTest(int x, int y);
	void	OnCheckTransparency();

	static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

public:
	HWND	GetHandle() const
	{
		return hwnd_;
	}

	RECT	GetRect() const
	{
		RECT rc{};
		GetWindowRect(hwnd_, &rc);
		return rc;
	}
};

