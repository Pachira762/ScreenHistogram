#pragma once
#include "PassConfig.h"

class GraphPass
{
public:
	void	Init(winrt::com_ptr<ID3D11Device>& device, winrt::com_ptr<ID3D11Buffer>& histogramBuffer);
	void	AddPass(winrt::com_ptr<ID3D11DeviceContext>& context, const PassConfig& config);

private:
	winrt::com_ptr<ID3D11InputLayout>			layout_{ nullptr };
	winrt::com_ptr<ID3D11VertexShader>			vs_{ nullptr };
	winrt::com_ptr<ID3D11PixelShader>			ps_{ nullptr };
	winrt::com_ptr<ID3D11Buffer>				vertexBuffer_{ nullptr };
	winrt::com_ptr<ID3D11ShaderResourceView>	histogramSrv_{ nullptr };
};

