#include "pch.h"
#include "ScreenCapture.h"
#include "dutil.h"
#include <sstream>

using namespace std;
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
	check_hresult(output1->DuplicateOutput(dxgiDevice.get(), dupl_.put()));

#if 0
	{
		DXGI_ADAPTER_DESC desc{};
		adapter->GetDesc(&desc);

		info += L"Adapter\n";
		info += desc.Description;
		info += L"\n";
	}

	{
		info += L"\nOutput\n";
		DXGI_OUTPUT_DESC desc{};
		output->GetDesc(&desc);

		MONITORINFO monitor{};
		GetMonitorInfo(desc.Monitor, &monitor);

		info += desc.DeviceName;
		info += L"\n";
	}

	{
		DXGI_OUTDUPL_FRAME_INFO frame{};
		com_ptr<IDXGIResource> resource{};
		HRESULT hr = dupl_->AcquireNextFrame(500, &frame, resource.put());
		if (SUCCEEDED(hr)) {
			D3D11_TEXTURE2D_DESC desc{};
			resource.as<ID3D11Texture2D>()->GetDesc(&desc);

			resource = nullptr;
			dupl_->ReleaseFrame();

			info += L"\nFrame\n";
			info += L"Width ";
			info += to_wstring(desc.Width) + L"\n";
			info += L"Height ";
			info += to_wstring(desc.Height) + L"\n";
			info += L"Format ";
			info += to_wstring(desc.Format) + L"\n";
		}
	}
#endif

	initialized_ = true;
}

static inline auto ToHex(HRESULT hr)
{
	wostringstream oss{};
	oss << std::hex << hr;
	return oss.str();
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
	ret.protectedContent = info.ProtectedContentMaskedOut;
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