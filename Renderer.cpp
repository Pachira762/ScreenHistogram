#include "pch.h"
#include "Renderer.h"
#include "dutil.h"
#include "Theme.h"

using namespace std;
using namespace winrt;

void Renderer::Init(HWND hwnd, int dpiX, int dpiY)
{
	RECT rc{};
	GetClientRect(hwnd, &rc);

	int width = rc.right - rc.left;
	int height = rc.bottom - rc.top;

	dpiX_ = dpiX;
	dpiY_ = dpiY;

	hwnd_ = hwnd;
	device_ = CreateD3DDevice(D3D_DRIVER_TYPE_HARDWARE);
	context_ = CreateD3DDeviceContext(device_.get());

#ifdef USE_DIRECTCOMPOSITION
	{
		swapChain_ = CreateSwapChainForComposition(device_, rc.right - rc.left, rc.bottom - rc.top, DXGI_FORMAT_B8G8R8A8_UNORM);
		composition_ = CreateCompositionDevice(device_);
		target_ = CreateCompositionTargetForHwnd(composition_, hwnd_);
		visual_ = CreateCompositionVisual(composition_);

		check_hresult(visual_->SetContent(swapChain_.get()));
		check_hresult(target_->SetRoot(visual_.get()));
		check_hresult(composition_->Commit());
	}
#elif defined USE_WINRTCOMPOSITION
	{
		dispatcherQueueController_ = CreateDispatcherQueueController();
		dispatcherQueue_ = dispatcherQueueController_.DispatcherQueue();

		swapChain_ = CreateSwapChainForComposition(device_.as<IDXGIDevice>(), width, height, DXGI_FORMAT_B8G8R8A8_UNORM).as<IDXGISwapChain4>();

		compositor_ = winrt::Windows::UI::Composition::Compositor();
		compositionTarget_ = CreateDesktopWindowTarget(compositor_, hwnd);
		compositionContent_ = compositor_.CreateSpriteVisual();
		compositionBrush_ = compositor_.CreateSurfaceBrush();

		auto surface = CreateCompositionSurfaceForSwapChain(compositor_, swapChain_.get());
		compositionBrush_.Surface(surface);
		compositionContent_.RelativeSizeAdjustment({ 1.f, 1.f });
		compositionContent_.Brush(compositionBrush_);
		compositionTarget_.Root(compositionContent_);
	}
#else
	{
		swapChain_ = CreateSwapChainForHwnd(device_, hwnd, rc.right - rc.left, rc.bottom - rc.top, DXGI_FORMAT_B8G8R8A8_UNORM);
	}
#endif

	// Direct2D
	{
		d2dFactory_ = CreateD2DFactory();
		d2dDevice_ = CreateD2DDevice(d2dFactory_.get(), device_.as<IDXGIDevice>().get());

		dwFactory_ = CreateDWriteFactory();
		dwTextFormat_ = CreateDWriteTextFormat(dwFactory_.get(), Theme::MonoFontName.c_str(), 16.f);

		InitRenderTarget();
	}

	rasterizerState_ = CreateRasterizerState(device_.get());
	blendState_ = CreateBlendState(device_.get());
	rtv_ = CreateRenderTargetView(device_.get(), swapChain_.get());
	viewport_ = CreateViewport(width, height);
}

void Renderer::SetDpi(int dpiX, int dpiY)
{
	dpiX_ = dpiX;
	dpiY_ = dpiY;

	d2dFgBrush_ = nullptr;
	d2dBgBrush_ = nullptr;
	d2dRenderTarget_ = nullptr;
	InitRenderTarget();
}

void Renderer::BeginDraw(int width, int height)
{
	AdjustRenderTarget(width, height);

	context_->RSSetState(rasterizerState_.get());
	context_->RSSetViewports(1, &viewport_);

	float factors[4] = {};
	context_->OMSetBlendState(blendState_.get(), factors, D3D11_DEFAULT_SAMPLE_MASK);

	ID3D11RenderTargetView* rtvs[] = { rtv_.get() };
	context_->OMSetRenderTargets(_countof(rtvs), rtvs, nullptr);

	static const float ClearColors[4] = { 0,0,0,0.f };
	context_->ClearRenderTargetView(rtv_.get(), ClearColors);

	BeginD2();
}

void Renderer::EndDraw()
{
	EndD2();
	swapChain_->Present(1, 0);
}

void Renderer::BeginD2()
{
	d2dRenderTarget_->BeginDraw();
}

void Renderer::EndD2()
{
	d2dRenderTarget_->EndDraw();
}

void Renderer::AdjustRenderTarget(int width, int height)
{
	auto desc = GetDesc<DXGI_SWAP_CHAIN_DESC>(swapChain_).BufferDesc;

	if (width != desc.Width || height != desc.Height) {
		rtv_ = { nullptr };
		d2dFgBrush_ = nullptr;
		d2dBgBrush_ = nullptr;
		d2dRenderTarget_ = nullptr;

		check_hresult(swapChain_->ResizeBuffers(2, width, height, desc.Format, 0));
		com_ptr<IDXGISurface> backbuffer{};
		check_hresult(swapChain_->GetBuffer(0, guid_of<IDXGISurface>(), backbuffer.put_void()));

		rtv_ = CreateRenderTargetView(device_.get(), backbuffer.as<ID3D11Resource>().get());

		viewport_.Width = static_cast<float>(width);
		viewport_.Height = static_cast<float>(height);

		InitRenderTarget();
	}
}

void Renderer::InitRenderTarget()
{
	d2dRenderTarget_ = CreateD2DRenderTarget(d2dFactory_.get(), swapChain_.get(), static_cast<float>(dpiX_), static_cast<float>(dpiY_));
	d2dFgBrush_ = CreateD2DColorBrush(d2dRenderTarget_.get(), Theme::Dark::AccentTextColor);
	d2dBgBrush_ = CreateD2DColorBrush(d2dRenderTarget_.get(), Theme::Dark::BgColor);
}
