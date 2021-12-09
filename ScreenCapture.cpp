#include "pch.h"
#include "ScreenCapture.h"
#include "dutil.h"

using namespace winrt;

ScreenCapture::ScreenCapture()
{
}

ScreenCapture::~ScreenCapture()
{
}

bool ScreenCapture::Initialize(CaptureShink* shink, winrt::com_ptr<ID3D11Device>& device)
{
	shink_ = shink;

	auto dxgiDevice = device.as<IDXGIDevice>();

	com_ptr<IDXGIAdapter> adapter{ nullptr };
	check_hresult(dxgiDevice->GetAdapter(adapter.put()));

	com_ptr<IDXGIOutput> output{ nullptr };
	check_hresult(adapter->EnumOutputs(0, output.put()));

	auto output1 = output.as<IDXGIOutput1>();
	check_hresult(output1->DuplicateOutput(device.get(), dupl_.put()));

	initialized_ = true;

	return true;
}

void ScreenCapture::Finalize()
{
	initialized_ = false;
}

void ScreenCapture::Capture(int sleepTime)
{
	if (!initialized_) {
		Sleep(sleepTime); // avoid busy wait
		return;
	}

	DXGI_OUTDUPL_FRAME_INFO info{};
	com_ptr<IDXGIResource> resource{ nullptr };
	HRESULT hr = dupl_->AcquireNextFrame(500, &info, resource.put());

	if (FAILED(hr)) {
		Sleep(sleepTime);
		return;
	}

	if (info.AccumulatedFrames == 0) {
		if (info.LastMouseUpdateTime.QuadPart == 0) {
			Sleep(sleepTime);
		}
		else {
			Sleep(sleepTime / 2);
		}
	}
	else {
		shink_->Shink(resource.as<ID3D11Texture2D>());

		if (info.AccumulatedFrames == 1) {
			Sleep(0);
		}
		else{
			// drop some frame
		}
	}

	dupl_->ReleaseFrame();
}

