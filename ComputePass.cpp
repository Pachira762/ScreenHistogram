#include "pch.h"
#include "ComputePass.h"
#include "dutil.h"

void ComputePass::Init(ID3D11Device* device, PipelineState& state)
{
	clearCs_ = CreateComputeShader(device, IDR_SHADER1, "ClearCS");
	colorHistogramCs_ = CreateComputeShader(device, IDR_SHADER1, "ColorHistogramCS");
	brightnessHistogramCs_ = CreateComputeShader(device, IDR_SHADER1, "BrightnessHistogramCS");
	//saturationHistogramCs_ = CreateComputeShader(device, IDR_SHADER1, "SaturationHistogramCS");
	histogramUav_ = CreateUnorderedAccessView(device, state.HistogramBuffer(), PipelineState::HistogramResolution);
}

void ComputePass::AddPass(ID3D11DeviceContext* context, PipelineState& state)
{
	ID3D11UnorderedAccessView* uavs[] = { histogramUav_.get() };
	context->CSSetUnorderedAccessViews(1, _countof(uavs), uavs, nullptr);

	context->CSSetShader(clearCs_.get(), nullptr, 0);
	context->Dispatch(PipelineState::HistogramResolution, 1, 1);

	switch (state.HistogramMode()) {
	case EHistogramMode::RGB:
	case EHistogramMode::RGBBrightness:
		context->CSSetShader(colorHistogramCs_.get(), nullptr, 0);
		break;
	
	case EHistogramMode::Brightness:
		context->CSSetShader(brightnessHistogramCs_.get(), nullptr, 0);
		break;

	//case EHistogramMode::Saturation:
	//	context->CSSetShader(saturationHistogramCs_.get(), nullptr, 0);
	//	break;
		
	default:
		return;
	}

	auto [groupX, groupY, groupZ] = state.NumThreadGroups();
	context->Dispatch(groupX, groupY, groupZ);

	// unbind buffer to use buffer as shader resource in other pass
	uavs[0] = nullptr;
	context->CSSetUnorderedAccessViews(1, _countof(uavs), uavs, nullptr);
}
