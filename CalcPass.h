#pragma once
#include "PassConfig.h"

class CalcPass
{
public:
	void	Init(winrt::com_ptr<ID3D11Device>& device, winrt::com_ptr<ID3D11Buffer>& histogramBuffer);
	void	AddPass(winrt::com_ptr<ID3D11DeviceContext>& context, const PassConfig& config);

private:
	winrt::com_ptr<ID3D11ComputeShader>			clearCs_{ nullptr };
	winrt::com_ptr<ID3D11ComputeShader>			calcCs_{ nullptr };
	winrt::com_ptr<ID3D11UnorderedAccessView>	histogramUav_{ nullptr };
};
