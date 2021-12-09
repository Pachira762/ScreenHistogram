#include "pch.h"
#include "CalcPass.h"
#include "dutil.h"

void CalcPass::Init(winrt::com_ptr<ID3D11Device>& device, winrt::com_ptr<ID3D11Buffer>& histogramBuffer)
{
	clearCs_ = CreateComputeShader(device, IDR_SHADER1, "ClearCS");
	calcCs_ = CreateComputeShader(device, IDR_SHADER1, "CalcCS");
	histogramUav_ = CreateUnorderedAccessView(device, histogramBuffer, PassConfig::HistogramResolution);
}

void CalcPass::AddPass(winrt::com_ptr<ID3D11DeviceContext>& context, const PassConfig& config)
{
	ID3D11UnorderedAccessView* uavs[] = { histogramUav_.get() };
	context->CSSetUnorderedAccessViews(1, _countof(uavs), uavs, nullptr);

	context->CSSetShader(clearCs_.get(), nullptr, 0);
	context->Dispatch(PassConfig::HistogramResolution, 1, 1);

	context->CSSetShader(calcCs_.get(), nullptr, 0);
	context->Dispatch(config.threadGroupX, config.threadGroupY, 1);

	// unbind buffer to use buffer as shader resource in other pass
	uavs[0] = nullptr;
	context->CSSetUnorderedAccessViews(1, _countof(uavs), uavs, nullptr);
}
