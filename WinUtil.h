#pragma once

class WinUtil
{
public:
	static void		SetDpi(int Dpi);

	static void		SetDpi(HWND hwnd = NULL);

	static HWND		Create(LPCTSTR className, LPCTSTR windowName, DWORD style, DWORD styleEx, int x, int y, int width, int height, HWND parent = NULL, HMENU menu = NULL);

	// x, y is pixel unit
	static LRESULT	NonClientHitTest(HWND hwnd, int x, int y);

private:
	static inline int	dpi_ = 96;
	static inline RECT	frame_ = {};
	static inline RECT	frame_pix_ = {};
	static inline int	caption_ = {};
	static inline int	caption_pix_ = {};
	static inline SIZE	vscroll_{};
	static inline SIZE	hscroll_{};

public:

	static int Dpi(int x)
	{
		return ((x * 96) / dpi_);
	}

	static POINT Dpi(const POINT& point)
	{
		return { Dpi(point.x), Dpi(point.y) };
	}

	static SIZE	Dpi(const SIZE& size)
	{
		return { Dpi(size.cx), Dpi(size.cy) };
	}

	static RECT	Dpi(const RECT& rect)
	{
		return { Dpi(rect.left), Dpi(rect.top), Dpi(rect.right), Dpi(rect.bottom) };
	}

	static int Pix(int dpi)
	{
		return ((dpi * dpi_) / 96);
	}

	static POINT Pix(const POINT& point)
	{
		return { Pix(point.x), Pix(point.y) };
	}

	static SIZE	Pix(const SIZE& size)
	{
		return { Pix(size.cx), Pix(size.cy) };
	}

	static RECT	Pix(const RECT& rect)
	{
		return { Pix(rect.left), Pix(rect.top), Pix(rect.right), Pix(rect.bottom) };
	}

	static int GetDpi()
	{
		return dpi_;
	}

	static POINT GetCursorScreenPos()
	{
		POINT cur{};
		::GetCursorPos(&cur);
		return Dpi(cur);
	}

	// x, y : pixel unit
	static POINT GetCursorClientPos(HWND hwnd, int x, int y) 
	{
		POINT pos{};
		::ClientToScreen(hwnd, &pos);

		return Dpi(POINT{ x - pos.x, y - pos.y });
	}

	static POINT GetCursorClientPos(HWND hwnd)
	{
		POINT cur{};
		::GetCursorPos(&cur);

		POINT pos{};
		::ClientToScreen(hwnd, &pos);

		return Dpi(POINT{ cur.x - pos.x, cur.y - pos.y });
	}

	static POINT GetWindowPos(HWND hwnd)
	{
		RECT rc = {};
		GetWindowRect(hwnd, &rc);
		return Dpi(POINT{ rc.left, rc.top });
	}

	static SIZE	GetWindowSize(HWND hwnd)
	{
		RECT rc = {};
		GetWindowRect(hwnd, &rc);
		return Dpi(SIZE{ rc.right - rc.left, rc.bottom - rc.top });
	}
	
	static POINT GetClientPos(HWND hwnd)
	{
		POINT p{ 0, 0 };
		ClientToScreen(hwnd, &p);
		return Dpi(p);
	}

	static SIZE GetClientSize(HWND hwnd)
	{
		RECT rc = {};
		GetClientRect(hwnd, &rc);
		return Dpi(SIZE{ rc.right, rc.bottom });
	}

	static RECT GetAdjustedRect(DWORD style, DWORD styleEx = 0, BOOL menu = FALSE)
	{
		RECT rc{};
		::AdjustWindowRectExForDpi(&rc, style, menu, style, dpi_);
		return rc;
	}

	static const RECT& GetFrameSize()
	{
		return frame_;
	}

	static const RECT& GetFrameSizePix()
	{
		return frame_;
	}

	static int GetCaptionSize()
	{
		return caption_;
	}

	static int GetVScrollWidth()
	{
		return vscroll_.cx;
	}

	static void SetWindowPos(HWND hwnd, int x, int y, UINT exFlag = 0)
	{
		::SetWindowPos(hwnd, NULL, Pix(x), Pix(y), 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | exFlag);
	}

	static void SetWindowSize(HWND hwnd, int width, int height, UINT exFlag = 0)
	{
		::SetWindowPos(hwnd, NULL, 0, 0, Pix(width), Pix(height), SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | exFlag);
	}

	static void SetWindowPosSize(HWND hwnd, int x, int y, int width, int height, UINT exFlag = 0)
	{
		::SetWindowPos(hwnd, NULL, Pix(x), Pix(y), Pix(width), Pix(height), SWP_NOZORDER | SWP_NOACTIVATE | exFlag);
	}
};

#define FrameSize WinUtil::GetFrameSize()

#define CaptionSize WinUtil::GetCaptionSize()

#define VScrollWidth WinUtil::GetVScrollWidth()
