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

static constexpr int NumFillVertexs = 6 * PipelineState::HistogramResolution;
static constexpr int NumLineVertexs = 2 * PipelineState::HistogramResolution;
static constexpr float Delta0 = 0.f;
static constexpr float DeltaU = 1.f / static_cast<float>(PipelineState::HistogramResolution);

void GraphPass::Init(ID3D11Device* device, PipelineState& state)
{
	D3D11_INPUT_ELEMENT_DESC elems[] = {
		   {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		   {"HISTOINDEX", 0, DXGI_FORMAT_R32_UINT, 0, 4 * 2, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	colorVs_ = CreateVertexShader(device, IDR_SHADER1, "ColorGraphVS", elems, _countof(elems), layout_.put());
	monoVs_ = CreateVertexShader(device, IDR_SHADER1, "MonoGraphVS");

	InitFillPass(device);
	InitLinePass(device);
	histogramSrv_ = CreateShaderResourceView(device, state.HistogramBuffer(), 4 * 4, 256);
}

void GraphPass::AddPass(ID3D11DeviceContext* context, PipelineState& state)
{;
	ID3D11ShaderResourceView* srvs[] = { histogramSrv_.get() };
	context->VSSetShaderResources(1, _countof(srvs), srvs);

	switch (state.HistogramMode()) {
	case EHistogramMode::RGB:
		AddColorPass(context, 3);
		break;

	case EHistogramMode::RGBBrightness:
		AddColorPass(context, 4);
		AddLinePass(context);
		break;

	case EHistogramMode::Brightness:
	//case EHistogramMode::Saturation:
		AddMonoPass(context);
		AddLinePass(context);
		break;
	}

	srvs[0] = nullptr;
	context->VSSetShaderResources(1, _countof(srvs), srvs);
}

void GraphPass::InitFillPass(ID3D11Device* device)
{
	fillPs_ = CreatePixelShader(device, IDR_SHADER1, "FillGraphPS");

	vector<Vertex> vertexs(NumFillVertexs);
	for (int i = 0; i < PipelineState::HistogramResolution; ++i) {
		float u = static_cast<float>(i) * DeltaU;

		vertexs[6 * i + 0] = { u + Delta0, 0.f, i };
		vertexs[6 * i + 1] = { u + DeltaU, 0.f, i + 1 };
		vertexs[6 * i + 2] = { u + Delta0, 1.f, i };
		vertexs[6 * i + 3] = { u + DeltaU, 0.f, i + 1 };
		vertexs[6 * i + 4] = { u + DeltaU, 1.f, i + 1 };
		vertexs[6 * i + 5] = { u + Delta0, 1.f, i };
	}

	fillVertexBuffer_ = CreateVertexBuffer(device, NumFillVertexs, sizeof(Vertex), vertexs.data());
}

void GraphPass::InitLinePass(ID3D11Device* device)
{
	linePs_ = CreatePixelShader(device, IDR_SHADER1, "LineGraphPS");

	vector<Vertex> vertexs(NumLineVertexs);
	for (int i = 0; i < PipelineState::HistogramResolution; ++i) {
		float u = static_cast<float>(i) * DeltaU;

		vertexs[2 * i + 0] = { u + Delta0, 1.f, i };
		vertexs[2 * i + 1] = { u + DeltaU, 1.f, i + 1 };
	}

	lineVertexBuffer_ = CreateVertexBuffer(device, NumLineVertexs, sizeof(Vertex), vertexs.data());
}

void GraphPass::AddColorPass(ID3D11DeviceContext* context, int numCh)
{
	context->VSSetShader(colorVs_.get(), nullptr, 0);
	context->PSSetShader(fillPs_.get(), nullptr, 0);

	context->IASetInputLayout(layout_.get());
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	ID3D11Buffer* vbs[] = { fillVertexBuffer_.get() };
	UINT strides[] = { sizeof(Vertex) };
	UINT offsets[] = { 0 };
	context->IASetVertexBuffers(0, _countof(vbs), vbs, strides, offsets);
	context->DrawInstanced(NumFillVertexs, numCh, 0, 0);
}

void GraphPass::AddMonoPass(ID3D11DeviceContext* context)
{
	context->VSSetShader(monoVs_.get(), nullptr, 0);
	context->PSSetShader(fillPs_.get(), nullptr, 0);

	context->IASetInputLayout(layout_.get());
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	ID3D11Buffer* vbs[] = { fillVertexBuffer_.get() };
	UINT strides[] = { sizeof(Vertex) };
	UINT offsets[] = { 0 };
	context->IASetVertexBuffers(0, _countof(vbs), vbs, strides, offsets);
	context->Draw(NumFillVertexs, 0);
}

void GraphPass::AddLinePass(ID3D11DeviceContext* context)
{
	context->VSSetShader(monoVs_.get(), nullptr, 0);
	context->PSSetShader(linePs_.get(), nullptr, 0);

	context->IASetInputLayout(layout_.get());
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	ID3D11Buffer* vbs[] = { lineVertexBuffer_.get() };
	UINT strides[] = { sizeof(Vertex) };
	UINT offsets[] = { 0 };
	context->IASetVertexBuffers(0, _countof(vbs), vbs, strides, offsets);
	context->Draw(NumLineVertexs, 0);
}
