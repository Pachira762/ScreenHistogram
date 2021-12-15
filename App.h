#pragma once
#include "pch.h"
#include "common.h"
#include "AppWindow.h"

class App : public AppWindowListener
{
public:
	App();
	~App();
	bool	Initialize();
	void	Finalize();

private:
	std::unique_ptr<class AppConfig>		config_;
	std::unique_ptr<class AppWindow>		window_;
	std::unique_ptr<class Renderer>			renderer_;
	std::unique_ptr<class ScreenCapture>	capture_;
	std::unique_ptr<class PipelineState>	state_;
	std::unique_ptr<class ComputePass>		computePass_;
	std::unique_ptr<class PlanePass>		planePass_;
	std::unique_ptr<class GraphPass>		graphPass_;
	std::unique_ptr<class ColorPickPass>	colorPickPass_;

	std::atomic_bool	bContinue_ = false;
	std::atomic_bool	bCapture_ = false;
	std::thread			thread_;

	std::atomic_bool	dpiChanged_ = false;
	std::atomic_int		dpiX_ = 96;
	std::atomic_int		dpiY_ = 96;

	struct CloseButton
	{
		std::atomic_int		x;
		std::atomic_int		y;
		std::atomic_int		w;
		std::atomic_int		h;
		std::atomic<EButtonState>	state;
	};
	CloseButton		close_;

	void	CaptureProcess(HWND hwnd);
	void	DrawCloseButton();

	virtual	void	OnCreate(HWND hwnd)override;
	virtual void	OnClose()override;
	virtual void	OnDestory()override;
	virtual void	OnSizing()override;
	virtual void	OnMoving()override;
	virtual void	OnMinimized()override;
	virtual void	OnDpiChanged(int dpiX, int dpiY)override;

	virtual void	SetHistogramMode(EHistogramMode mode)override;
	virtual void	SetViewMode(EViewMode mode)override;
	virtual void	SetScale(float scale)override;
	virtual void	SetOpacity(float opacity)override;
	virtual void	SetColorPickMode(EColorPickMode mode)override;

	virtual void	SetCloseButtonState(int cx, int cy, int cw, int ch, EButtonState state)override;
};
