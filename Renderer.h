#pragma once

class Renderer
{
public:
	void	Init(HWND hwnd, int dpiX, int dpiY);
	void	SetDpi(int dpiX, int dpiY);
	void	BeginDraw(int width, int height);
	void	EndDraw();

private:
	HWND	hwnd_ = NULL;
	int		dpiX_ = 96;
	int		dpiY_ = 96;

	winrt::com_ptr<ID3D11Device5>			device_{ nullptr };
	winrt::com_ptr<ID3D11DeviceContext4>	context_{ nullptr };
	winrt::com_ptr<IDXGISwapChain4>			swapChain_{ nullptr };

	// Direct2D / DirectWrite
	winrt::com_ptr<ID2D1Factory6>			d2dFactory_{ nullptr };
	winrt::com_ptr<ID2D1Device4>			d2dDevice_{ nullptr };
	winrt::com_ptr<ID2D1RenderTarget>		d2dRenderTarget_{nullptr};
	winrt::com_ptr<ID2D1SolidColorBrush>	d2dFgBrush_{ nullptr };
	winrt::com_ptr<ID2D1SolidColorBrush>	d2dBgBrush_{ nullptr };
	winrt::com_ptr<IDWriteFactory7>			dwFactory_{ nullptr };
	winrt::com_ptr<IDWriteTextFormat>		dwTextFormat_{ nullptr };

#ifdef USE_DIRECTCOMPOSITION
	winrt::com_ptr<IDCompositionDevice>		composition_{};
	winrt::com_ptr<IDCompositionTarget>		target_{};
	winrt::com_ptr<IDCompositionVisual>		visual_{};
#else defined USE_WINRTCOMPOSITION
	winrt::Windows::System::DispatcherQueueController				dispatcherQueueController_{ nullptr };
	winrt::Windows::System::DispatcherQueue							dispatcherQueue_{ nullptr };
	winrt::Windows::UI::Composition::Compositor						compositor_{ nullptr };
	winrt::Windows::UI::Composition::Desktop::DesktopWindowTarget	compositionTarget_{ nullptr };
	winrt::Windows::UI::Composition::SpriteVisual					compositionContent_{ nullptr };
	winrt::Windows::UI::Composition::CompositionSurfaceBrush		compositionBrush_{ nullptr };
#endif

	winrt::com_ptr<ID3D11RasterizerState>	rasterizerState_{ nullptr };
	winrt::com_ptr<ID3D11BlendState>		blendState_{ nullptr };
	winrt::com_ptr<ID3D11RenderTargetView>	rtv_{ nullptr };
	D3D11_VIEWPORT	viewport_{};

	void	BeginD2();
	void	EndD2();

	void	AdjustRenderTarget(int width, int height);
	void	InitRenderTarget();

public:
	auto GetHwnd() const {
		return hwnd_;
	}

	auto Device() const {
		return device_.get();
	}

	auto Context() const {
		return context_.get();
	}

	auto D2DRenderTarget() const {
		return d2dRenderTarget_.get();
	}

	auto D2DFgColorBrush() const {
		return d2dFgBrush_.get();
	}

	auto D2DBgColorBrush() const {
		return d2dBgBrush_.get();
	}

	auto DWriteTextFormat() const {
		return dwTextFormat_.get();
	}

	auto DWriteFactory() const {
		return dwFactory_.get();
	}
};
