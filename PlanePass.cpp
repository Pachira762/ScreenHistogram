#include "pch.h"
#include "PlanePass.h"
#include "dutil.h"

struct Vertex
{
	float	u;
	float	v;
};

void PlanePass::Init(winrt::com_ptr<ID3D11Device>& device, winrt::com_ptr<ID3D11Buffer>& histogramBuffer)
{
	D3D11_INPUT_ELEMENT_DESC elems[] = {
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};
	vs_ = CreateVertexShader(device, IDR_SHADER1, "PlaneVS", elems, _countof(elems), layout_.put());
	ps_ = CreatePixelShader(device, IDR_SHADER1, "PlanePS");

	Vertex vertexs[6] = {
			{0.f, 0.f},
			{1.f, 0.f},
			{0.f, 1.f},

			{1.f, 0.f},
			{1.f, 1.f},
			{0.f, 1.f},
	};

	vertexBuffer_ = CreateVertexBuffer(device, 6, sizeof(Vertex), vertexs);
	histogramSrv_ = CreateShaderResourceView(device, histogramBuffer, 16, 256);
}

void PlanePass::AddPass(winrt::com_ptr<ID3D11DeviceContext>& context, const PassConfig& config)
{
	ID3D11ShaderResourceView* srvs[] = { histogramSrv_.get() };

	context->VSSetShader(vs_.get(), nullptr, 0);
	context->PSSetShader(ps_.get(), nullptr, 0);
	context->PSSetShaderResources(1, _countof(srvs), srvs);

	context->IASetInputLayout(layout_.get());
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	ID3D11Buffer* vbs[] = { vertexBuffer_.get() };
	UINT strides[] = { sizeof(Vertex) };
	UINT offsets[] = { 0 };
	context->IASetVertexBuffers(0, _countof(vbs), vbs, strides, offsets);
	context->Draw(6, 0);

	srvs[0] = nullptr;
	context->PSSetShaderResources(1, _countof(srvs), srvs);
}
