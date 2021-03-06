#pragma once
#include "PipelineState.h"

class PlanePass
{
public:
	void	Init(ID3D11Device* device, PipelineState& state);
	void	AddPass(ID3D11DeviceContext* context, PipelineState& state);

private:
	winrt::com_ptr<ID3D11InputLayout>	layout_{ nullptr };
	winrt::com_ptr<ID3D11VertexShader>	vs_{ nullptr };
	winrt::com_ptr<ID3D11PixelShader>	colorPs_{ nullptr };
	winrt::com_ptr<ID3D11PixelShader>	brightnessPs_{ nullptr };
	winrt::com_ptr<ID3D11PixelShader>	saturationHSVPs_{ nullptr };
	winrt::com_ptr<ID3D11PixelShader>	saturationHLSPs_{ nullptr };
	winrt::com_ptr<ID3D11PixelShader>	colorMaskPs_{ nullptr };
	winrt::com_ptr<ID3D11PixelShader>	colorMaskGrayPs_{ nullptr };
	winrt::com_ptr<ID3D11Buffer>		vertexBuffer_{ nullptr };
};
