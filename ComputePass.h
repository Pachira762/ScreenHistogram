#pragma once
#include "common.h"
#include "PipelineState.h"

class ComputePass
{
public:
	void	Init(ID3D11Device* device, PipelineState& state);
	void	AddPass(ID3D11DeviceContext* context, PipelineState& state);

private:
	winrt::com_ptr<ID3D11ComputeShader>			clearCs_{ nullptr };
	winrt::com_ptr<ID3D11ComputeShader>			colorHistogramCs_{ nullptr };
	winrt::com_ptr<ID3D11ComputeShader>			brightnessHistogramCs_{ nullptr };
	winrt::com_ptr<ID3D11ComputeShader>			saturationHistogramCs_{ nullptr };
	winrt::com_ptr<ID3D11UnorderedAccessView>	histogramUav_{ nullptr };
};
