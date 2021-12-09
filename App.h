#pragma once
#include "pch.h"
#include "AppWindow.h"

class App : public AppWindowListener
{
public:
	App();
	~App();
	bool	Initialize();
	void	Finalize();

private:
	std::unique_ptr<class AppWindow>		window_;
	std::unique_ptr<class ScreenCapture>	capture_;
	std::unique_ptr<class Viewer>			viewer_;

	std::atomic_bool	bContinue_ = false;
	std::atomic_bool	bCapture_ = false;
	std::thread			thread_;
	int	frameRate_ = 0;

	void	CaptureProcess(HWND hwnd);

	virtual	void	OnCreate(HWND hwnd)override;
	virtual void	OnClose()override;
	virtual void	OnDestory()override;
	virtual void	OnSizing()override;
	virtual void	OnMoving()override;
	virtual void	OnMinimized()override;
};
