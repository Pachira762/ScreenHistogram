#pragma once
#include "CaptureShink.h"
#include "PassConfig.h"

struct ViewerConfig
{
	int		ResolutionLimit;
	float	Opacity;
	float	Scale;
};

class Viewer : public CaptureShink
{
public:
	Viewer();
	virtual ~Viewer();

	bool	Init(HWND hwnd, winrt::com_ptr<ID3D11Device> device, const ViewerConfig& config);

	ViewerConfig	GetConfig();

	virtual void	Shink(winrt::com_ptr<ID3D11Texture2D>& capture)override;

private:
	bool	initialized_ = false;
	HWND	hwnd_ = NULL;
	
	// devices
	winrt::com_ptr<ID3D11Device>			device_{ nullptr };
	winrt::com_ptr<ID3D11DeviceContext>		context_{ nullptr };
	winrt::com_ptr<IDXGISwapChain1>			swapChain_{ nullptr };

#ifdef USE_DIRECTCOMPOSITION
	winrt::com_ptr<IDCompositionDevice>		composition_{};
	winrt::com_ptr<IDCompositionTarget>		target_{};
	winrt::com_ptr<IDCompositionVisual>		visual_{};
#else defined USE_WINRTCOMPOSITION
	winrt::Windows::System::DispatcherQueueController				dispatcherQueueController_{ nullptr };
	winrt::Windows::System::DispatcherQueue							dispatcherQueue_{ nullptr };
	winrt::Windows::UI::Composition::Compositor						compositor_{ nullptr };
	winrt::Windows::UI::Composition::Desktop::DesktopWindowTarget	target_{ nullptr };
	winrt::Windows::UI::Composition::SpriteVisual					content_{ nullptr };
	winrt::Windows::UI::Composition::CompositionSurfaceBrush		brush_{ nullptr };
#endif

	// common states
	winrt::com_ptr<ID3D11RasterizerState>	rasterizerState_{ nullptr };
	winrt::com_ptr<ID3D11BlendState>		blendState_{ nullptr };
	winrt::com_ptr<ID3D11RenderTargetView>	rtv_{ nullptr };
	D3D11_VIEWPORT	viewport_{};

	// common resource
	winrt::com_ptr<ID3D11SamplerState>		samplerState_ = { nullptr };
	winrt::com_ptr<ID3D11Buffer>			constantBuffer_{ nullptr };
	winrt::com_ptr<ID3D11Buffer>			histogramBuffer_{ nullptr };

	// passes
	PassConfig								config_;
	std::unique_ptr<class CalcPass>			calcPass_;
	//std::unique_ptr<class PlanePass>		planePass_;
	std::unique_ptr<class GraphPass>		graphPass_;

	void	SettingPass(int left, int top, int right, int bottom);
	void	AdjustSize();
	void	SetDrawStates(winrt::com_ptr<ID3D11Texture2D>& capture);
};

