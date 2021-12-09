#include "pch.h"
#include "GraphPass.h"
#include "dutil.h"

using namespace std;

struct Vertex
{
	float		u;
	float		v;
	int32_t		index;
};

static constexpr int NumVertexs = 6 * PassConfig::HistogramResolution;

void GraphPass::Init(winrt::com_ptr<ID3D11Device>& device, winrt::com_ptr<ID3D11Buffer>& histogramBuffer)
{
	D3D11_INPUT_ELEMENT_DESC elems[] = {
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"HISTOINDEX", 0, DXGI_FORMAT_R32_UINT, 0, 4 * 2, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	vs_ = CreateVertexShader(device, IDR_SHADER1, "GraphVS", elems, _countof(elems), layout_.put());
	ps_ = CreatePixelShader(device, IDR_SHADER1, "GraphPS");

	constexpr float Delta0 = 0.f;
	constexpr float DeltaU = 1.f / static_cast<float>(PassConfig::HistogramResolution);

	Vertex vertexs[NumVertexs];
	for (int i = 0; i < PassConfig::HistogramResolution; ++i) {
		float u = static_cast<float>(i) * DeltaU;

		vertexs[6 * i + 0] = { u + Delta0, 0.f, i };
		vertexs[6 * i + 1] = { u + DeltaU, 0.f, i + 1 };
		vertexs[6 * i + 2] = { u + Delta0, 1.f, i };
		vertexs[6 * i + 3] = { u + DeltaU, 0.f, i + 1 };
		vertexs[6 * i + 4] = { u + DeltaU, 1.f, i + 1 };
		vertexs[6 * i + 5] = { u + Delta0, 1.f, i };
	}

	vertexBuffer_ = CreateVertexBuffer(device, NumVertexs, sizeof(Vertex), vertexs);
	histogramSrv_ = CreateShaderResourceView(device, histogramBuffer, 4 * 4, 256);
}

void GraphPass::AddPass(winrt::com_ptr<ID3D11DeviceContext>& context, const PassConfig& config)
{;
	ID3D11ShaderResourceView* srvs[] = { histogramSrv_.get() };

	context->VSSetShader(vs_.get(), nullptr, 0);
	context->VSSetShaderResources(1, _countof(srvs), srvs);
	context->PSSetShader(ps_.get(), nullptr, 0);

	context->IASetInputLayout(layout_.get());
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	ID3D11Buffer* vbs[] = { vertexBuffer_.get() };
	UINT strides[] = { sizeof(Vertex) };
	UINT offsets[] = { 0 };
	context->IASetVertexBuffers(0, _countof(vbs), vbs, strides, offsets);
	context->DrawInstanced(NumVertexs, 3, 0, 0);

	srvs[0] = nullptr;
	context->VSSetShaderResources(1, _countof(srvs), srvs);
}
