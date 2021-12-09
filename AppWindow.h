#pragma once

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
};

class AppWindow
{
public:
	AppWindow(AppWindowListener* listener);
	~AppWindow();

	HWND	Create(int x, int y, int width, int height);
	void	Show();

private:
	AppWindowListener* listener_;

	HWND	hwnd_{};
	DWORD	style_;
	DWORD	styleEx_;
	bool	tranparency_ = false;
	
	void	SetTransparency(bool transparency);
	LRESULT	HitTest(int x, int y);

	void	OnCreate(LPCREATESTRUCT cs);
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

