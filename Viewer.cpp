#include "Viewer.h"
#include <tuple>
#include "common.h"
#include "dutil.h"
#include "CalcPass.h"
#include "PlanePass.h"
#include "GraphPass.h"

using namespace std;
using namespace winrt;

static inline int CalcStep(int width, int maxWidth)
{
	int step = 1;

	while (width > maxWidth) {
		++step;
		width /= 2;
	}

	return step;
}

static inline std::tuple<int, int, int, int> GetClientArea(HWND hwnd)
{
	RECT rc{};
	GetClientRect(hwnd, &rc);

	POINT p0{ 0,0 };
	check_bool(ClientToScreen(hwnd, &p0));

	POINT p1{ rc.right, rc.bottom };
	check_bool(ClientToScreen(hwnd, &p1));

	return { p0.x,p0.y,p1.x,p1.y };
}

Viewer::Viewer():
	calcPass_(make_unique<CalcPass>()),
	//planePass_(make_unique<PlanePass>()),
	graphPass_(make_unique<GraphPass>())

{
	config_.cv.left = 0;
	config_.cv.top = 0;
	config_.cv.width = 0;
	config_.cv.height = 0;

	config_.cv.step = 1;
	config_.cv.numCel = 1;
	config_.cv.invNumCel = 1.f;
	
	config_.cv.graphOpacity = 0.5f;
	config_.cv.graphScale = 5.f;

	config_.threadGroupX = 1;
	config_.threadGroupY = 1;

	config_.maxResolution = 512;
}

Viewer::~Viewer()
{
}

bool Viewer::Init(HWND hwnd, winrt::com_ptr<ID3D11Device> device, const ViewerConfig& config)
{
	RECT rc{};
	GetClientRect(hwnd, &rc);

	config_.maxResolution = config.ResolutionLimit;
	config_.cv.graphOpacity = config.Opacity;
	config_.cv.graphScale = config.Scale;

	hwnd_ = hwnd;
	device_ = device;
	context_ = CreateD3DDeviceContext(device_);

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

		swapChain_ = CreateSwapChainForComposition(device_, rc.right - rc.left, rc.bottom - rc.top, DXGI_FORMAT_B8G8R8A8_UNORM);

		compositor_ = winrt::Windows::UI::Composition::Compositor();
		target_ = CreateDesktopWindowTarget(compositor_, hwnd);
		content_ = compositor_.CreateSpriteVisual();
		brush_ = compositor_.CreateSurfaceBrush();
		
		//brush_.Stretch(winrt::Windows::UI::Composition::CompositionStretch::Uniform);
		brush_.Surface(CreateCompositionSurfaceForSwapChain(compositor_, swapChain_.get()));

		content_.RelativeSizeAdjustment({ 1.f, 1.f });
		content_.Brush(brush_);

		target_.Root(content_);
	}
#else
	{
		swapChain_ = CreateSwapChainForHwnd(device_, hwnd, rc.right - rc.left, rc.bottom - rc.top, DXGI_FORMAT_B8G8R8A8_UNORM);
	}
#endif

	rasterizerState_ = CreateRasterizerState(device_);
	blendState_ = CreateBlendState(device_);
	rtv_ = CreateRenderTargetViewForBackbuffer(device_, swapChain_);
	viewport_ = CreateViewport(rc.right - rc.left, rc.bottom - rc.top);

	samplerState_ = CreateSamplerState(device_);
	constantBuffer_ = CreateConstantBuffer(device_, sizeof(config_.cv));
	histogramBuffer_ = CreateUnorderedAccessBuffer(device_, PassConfig::HistogramElemSize, PassConfig::HistogramResolution, D3D11_BIND_SHADER_RESOURCE, D3D11_RESOURCE_MISC_BUFFER_STRUCTURED);

	calcPass_->Init(device_, histogramBuffer_);
	graphPass_->Init(device_, histogramBuffer_);

	return true;
}

ViewerConfig Viewer::GetConfig()
{
	ViewerConfig config;
	config.ResolutionLimit = config_.maxResolution;
	config.Opacity = config_.cv.graphOpacity;
	config.Scale = config_.cv.graphScale;
	return config;
}

void Viewer::Shink(winrt::com_ptr<ID3D11Texture2D>& capture)
{
	const auto [left, top, right, bottom] = GetClientArea(hwnd_);
	if ((right - left) <= 0 || (bottom - top) <= 0) {
		return;
	}

	SettingPass(left, top, right, bottom);
	AdjustSize();

	SetDrawStates(capture);
	calcPass_->AddPass(context_, config_);
	graphPass_->AddPass(context_, config_);

	swapChain_->Present(1, 0);
}

void Viewer::SettingPass(int left, int top, int right, int bottom)
{
	const int tilingX = PassConfig::ThreadTilingX;
	const int tilingY = PassConfig::ThreadTilingY;
	const int width = (right - left);
	const int height = (bottom - top);
	const int step = CalcStep(max(width, height), config_.maxResolution);
	const int groupX = (width / tilingX / step);
	const int groupY = (height / tilingY / step);
	const int numCel = (groupX * tilingX) * (groupY * tilingY);

	config_.cv.left = left;
	config_.cv.top = top;
	config_.cv.width = width;
	config_.cv.height = height;
	config_.cv.step = step;
	config_.cv.numCel = numCel;
	config_.cv.invNumCel = 1.f / static_cast<float>(numCel);
	config_.threadGroupX = groupX;
	config_.threadGroupY = groupY;

	WriteResource(context_, constantBuffer_.get(), &config_.cv, sizeof(config_.cv));
}

void Viewer::AdjustSize()
{
	auto width = config_.cv.width;
	auto height = config_.cv.height;

	auto desc = GetDesc<DXGI_SWAP_CHAIN_DESC>(swapChain_).BufferDesc;
	if (width != desc.Width || height != desc.Height) {
		rtv_ = { nullptr };
		check_hresult(swapChain_->ResizeBuffers(2, width, height, desc.Format, 0));
		rtv_ = CreateRenderTargetViewForBackbuffer(device_, swapChain_);

		viewport_.Width = static_cast<float>(width);
		viewport_.Height = static_cast<float>(height);
		context_->RSSetViewports(1, &viewport_);
	}
}

void Viewer::SetDrawStates(winrt::com_ptr<ID3D11Texture2D>& capture)
{
	context_->RSSetState(rasterizerState_.get());
	context_->RSSetViewports(1, &viewport_);

	float factors[4] = {};
	context_->OMSetBlendState(blendState_.get(), factors, D3D11_DEFAULT_SAMPLE_MASK);

	ID3D11RenderTargetView* rtvs[] = { rtv_.get() };
	context_->OMSetRenderTargets(_countof(rtvs), rtvs, nullptr);

	static const float ClearColors[4] = { 0,0,0,0 };
	context_->ClearRenderTargetView(rtv_.get(), ClearColors);

	auto captureView = CreateShaderResourceView(device_, capture);

	ID3D11ShaderResourceView* srvs[] = { captureView.get() };
	context_->CSSetShaderResources(0, _countof(srvs), srvs);
	context_->VSSetShaderResources(0, _countof(srvs), srvs);
	context_->PSSetShaderResources(0, _countof(srvs), srvs);

	ID3D11Buffer* cbs[] = { constantBuffer_.get() };
	context_->CSSetConstantBuffers(0, _countof(cbs), cbs);
	context_->VSSetConstantBuffers(0, _countof(cbs), cbs);
	context_->PSSetConstantBuffers(0, _countof(cbs), cbs);

	ID3D11SamplerState* samplers[] = { samplerState_.get() };
	context_->CSSetSamplers(0, _countof(samplers), samplers);
	context_->VSSetSamplers(0, _countof(samplers), samplers);
	context_->PSSetSamplers(0, _countof(samplers), samplers);
}
