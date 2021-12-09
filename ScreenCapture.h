#pragma once
#include "CaptureShink.h"

class ScreenCapture
{
public:
	ScreenCapture();
	~ScreenCapture();

	bool	Initialize(CaptureShink* shink, winrt::com_ptr<ID3D11Device>& device);
	void	Finalize();
	void	Capture(int sleepTime);

private:
	bool	initialized_ = false;

	CaptureShink* shink_;
	winrt::com_ptr<IDXGIOutputDuplication> dupl_{ nullptr };
};

