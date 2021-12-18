#include "pch.h"
#include "PlanePass.h"
#include "dutil.h"

struct Vertex
{
	float	u;
	float	v;
};

void PlanePass::Init(ID3D11Device* device, PipelineState& state)
{
	D3D11_INPUT_ELEMENT_DESC elems[] = {
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};
	vs_ = CreateVertexShader(device, IDR_SHADER1, "PlaneVS", elems, _countof(elems), layout_.put());
	colorPs_ = CreatePixelShader(device, IDR_SHADER1, "ColorPlanePS");
	brightnessPs_ = CreatePixelShader(device, IDR_SHADER1, "BrightnessPlanePS");
	saturationHSVPs_ = CreatePixelShader(device, IDR_SHADER1, "SaturationHSVPlanePS");
	saturationHLSPs_ = CreatePixelShader(device, IDR_SHADER1, "SaturationHLSPlanePS");

	Vertex vertexs[6] = {
			{0.f, 0.f},
			{1.f, 0.f},
			{0.f, 1.f},

			{1.f, 0.f},
			{1.f, 1.f},
			{0.f, 1.f},
	};

	vertexBuffer_ = CreateVertexBuffer(device, 6, sizeof(Vertex), vertexs);
}

void PlanePass::AddPass(ID3D11DeviceContext* context, PipelineState& state)
{
	if (state.ViewMode() == EViewMode::Color) {
		return;
	}

	context->VSSetShader(vs_.get(), nullptr, 0);

	switch (state.ViewMode()) {
	case EViewMode::Color:
		context->PSSetShader(colorPs_.get(), nullptr, 0);
		break;

	case EViewMode::Brightness:
		context->PSSetShader(brightnessPs_.get(), nullptr, 0);
		break;

	case EViewMode::SaturationHSV:
		context->PSSetShader(saturationHSVPs_.get(), nullptr, 0);
		break;

	case EViewMode::SaturationHLS:
		context->PSSetShader(saturationHLSPs_.get(), nullptr, 0);
		break;

	default:
		return;
	}

	context->IASetInputLayout(layout_.get());
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	ID3D11Buffer* vbs[] = { vertexBuffer_.get() };
	UINT strides[] = { sizeof(Vertex) };
	UINT offsets[] = { 0 };
	context->IASetVertexBuffers(0, _countof(vbs), vbs, strides, offsets);
	context->Draw(6, 0);
}
