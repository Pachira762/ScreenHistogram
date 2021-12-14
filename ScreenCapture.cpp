#include "pch.h"
#include "ScreenCapture.h"
#include "dutil.h"

using namespace winrt;

ScreenCapture::ScreenCapture()
{
}

ScreenCapture::~ScreenCapture()
{
	if (captured_) {
		dupl_->ReleaseFrame();
		captured_ = false;
	}
}

void ScreenCapture::Init(ID3D11Device* device)
{
	com_ptr<IDXGIDevice> dxgiDevice{ nullptr };
	check_hresult(device->QueryInterface<IDXGIDevice>(dxgiDevice.put()));

	com_ptr<IDXGIAdapter> adapter{ nullptr };
	check_hresult(dxgiDevice->GetAdapter(adapter.put()));

	com_ptr<IDXGIOutput> output{ nullptr };
	check_hresult(adapter->EnumOutputs(0, output.put()));

	auto output1 = output.as<IDXGIOutput1>();
	check_hresult(output1->DuplicateOutput(device, dupl_.put()));

	initialized_ = true;
}

ScreenCapture::Result ScreenCapture::Capture(int timeout)
{
	if (!initialized_) {
		return {false};
	}

	if (captured_) {
		dupl_->ReleaseFrame();
		captured_ = false;
	}

	DXGI_OUTDUPL_FRAME_INFO info{};
	com_ptr<IDXGIResource> resource{ nullptr };
	HRESULT hr = dupl_->AcquireNextFrame(500, &info, resource.put());
	if (FAILED(hr)) {
		return {false};
	}

	captured_ = true;

	Result ret;
	ret.succeeded = true;
	ret.mouseUpdated = info.LastMouseUpdateTime.QuadPart != 0;
	ret.screenUpdated = info.LastPresentTime.QuadPart != 0;
	ret.accumulatedFrames = info.AccumulatedFrames;
	ret.texture = resource.as<ID3D11Texture2D>();

	return ret;
}

void ScreenCapture::ReleaseCapture()
{
	if (dupl_ && captured_) {
		dupl_->ReleaseFrame();
		captured_ = false;
	}
}