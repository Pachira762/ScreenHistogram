#pragma once

class ScreenCapture
{
public:
	struct Result
	{
		bool	succeeded;
		bool	mouseUpdated;
		bool	screenUpdated;
		UINT	accumulatedFrames;
		winrt::com_ptr<ID3D11Texture2D> texture;
	};

	ScreenCapture();
	~ScreenCapture();

	void	Init(ID3D11Device* device);
	Result	Capture(int timeout);
	void	ReleaseCapture();

private:
	bool	initialized_ = false;
	bool	captured_ = false;

	winrt::com_ptr<IDXGIOutputDuplication> dupl_{ nullptr };
};

