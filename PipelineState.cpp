#include "pch.h"
#include "PipelineState.h"
#include "dutil.h"

using namespace std;
using namespace winrt;

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

static inline int CalcStep(int width, int maxWidth)
{
	if (width < 0 || maxWidth < 0) {
		return 1;
	}

	int step = 1;

	while (width > maxWidth) {
		++step;
		width /= 2;
	}

	return step;
}

PipelineState::PipelineState()
{
	cv_.left = 0;
	cv_.left = 0;
	cv_.width = 0;
	cv_.height = 0;
	cv_.steps = 1;
	cv_.numCel = 1;
	cv_.invNumCel = 1.f;
	cv_.opacity = 1.f;
	cv_.scale = 1.f;
}

PipelineState::~PipelineState()
{
}

void PipelineState::Init(ID3D11Device* device, HWND hwnd)
{
	hwnd_ = hwnd;
	constantBuffer_ = CreateConstantBuffer(device, sizeof(ConstantVariable));
	histogramBuffer_ = CreateUnorderedAccessBuffer(device, HistogramElemSize, HistogramResolution, D3D11_BIND_SHADER_RESOURCE, D3D11_RESOURCE_MISC_BUFFER_STRUCTURED);
}

bool PipelineState::Update(ID3D11Device* device, ID3D11DeviceContext* context, winrt::com_ptr<ID3D11Texture2D>& capture)
{
	std::unique_lock<std::mutex> lock(mtx_);

	const auto [left, top, right, bottom] = GetClientArea(hwnd_);
	const int width = right - left;
	const int height = bottom - top;

	if (width <= 0 || height <= 0) {
		return false;
	}

	const int numSteps = CalcStep(max(width, height), maxResolution_);
	const int groupX = width / ThreadTilingX / numSteps;
	const int groupY = height / ThreadTilingY / numSteps;
	const int numCel = (groupX * ThreadTilingX) * (groupY * ThreadTilingY);

	cv_.left = left;
	cv_.top = top;
	cv_.width = width;
	cv_.height = height;
	cv_.steps = numSteps;
	cv_.numCel = numCel;
	cv_.invNumCel = 1.f / static_cast<float>(numCel);

	groupNumX_ = groupX;
	groupNumY_ = groupY;
	groupNumZ_ = 1;

	context->UpdateSubresource(constantBuffer_.get(), 0, nullptr, &cv_, sizeof(ConstantVariable), sizeof(ConstantVariable));

	ID3D11Buffer* cbs[] = { constantBuffer_.get() };
	context->CSSetConstantBuffers(0, _countof(cbs), cbs);
	context->VSSetConstantBuffers(0, _countof(cbs), cbs);
	context->PSSetConstantBuffers(0, _countof(cbs), cbs);

	auto srv = CreateShaderResourceView(device, capture);
	ID3D11ShaderResourceView* srvs[] = { srv.get() };
	context->CSSetShaderResources(0, _countof(srvs), srvs);
	context->VSSetShaderResources(0, _countof(srvs), srvs);
	context->PSSetShaderResources(0, _countof(srvs), srvs);

	return true;
}
