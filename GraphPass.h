#pragma once
#include "PipelineState.h"

class GraphPass
{
public:
	void	Init(ID3D11Device* device, PipelineState& state);
	void	AddPass(ID3D11DeviceContext* context, PipelineState& state);

private:
	winrt::com_ptr<ID3D11InputLayout>			layout_{ nullptr };
	winrt::com_ptr<ID3D11VertexShader>			colorVs_{ nullptr };
	winrt::com_ptr<ID3D11VertexShader>			monoVs_{ nullptr };
	winrt::com_ptr<ID3D11PixelShader>			fillPs_{ nullptr };
	winrt::com_ptr<ID3D11PixelShader>			linePs_{ nullptr };
	winrt::com_ptr<ID3D11Buffer>				fillVertexBuffer_{ nullptr };
	winrt::com_ptr<ID3D11Buffer>				lineVertexBuffer_{ nullptr };
	winrt::com_ptr<ID3D11ShaderResourceView>	histogramSrv_{ nullptr };

	void	InitFillPass(ID3D11Device* device);
	void	InitLinePass(ID3D11Device* device);

	void	AddColorPass(ID3D11DeviceContext* context, int numCh);
	void	AddMonoPass(ID3D11DeviceContext* context);
	void	AddLinePass(ID3D11DeviceContext* context);
};
