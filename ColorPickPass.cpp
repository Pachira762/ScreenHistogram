#include "pch.h"
#include "ColorPickPass.h"
#include "Renderer.h"
#include "PipelineState.h"
#include "Theme.h"
#include "dutil.h"

using namespace std;
using namespace winrt;

void ColorPickPass::Init(class Renderer* renderer, int dpiX, int dpiY)
{
	dpiX_ = dpiX;
	dpiY_ = dpiY;

	auto device = renderer->Device();
	staging_ = CreateTexture2D(device, 1, 1, DXGI_FORMAT_B8G8R8A8_UNORM, D3D11_USAGE_STAGING, 0, D3D11_CPU_ACCESS_READ);
}

void ColorPickPass::AddPass(class Renderer* renderer, class PipelineState* state, ID3D11Texture2D* capture)
{
	if (auto mode = state->ColorPickMode(); mode == EColorPickMode::None) {
		return;
	}

	POINT cursor{};
	GetCursorPos(&cursor);
	auto color = PickColor(renderer->Context(), capture, cursor.x, cursor.y);

	auto rt = renderer->D2DRenderTarget();

	com_ptr<ID2D1SolidColorBrush> colorBrush{};
	rt->CreateSolidColorBrush(D2D1::ColorF(color, 1.f), colorBrush.put());

	auto text = GetLayoutedText(renderer->DWriteFactory(), renderer->DWriteTextFormat(), color, state->ColorPickMode());
	if (!text) {
		return;
	}

	DWRITE_TEXT_METRICS metrix{};
	text->GetMetrics(&metrix);

	auto mx = 8.f;
	auto my = 4.f;

	auto [cx, cy] = ScreenToClientDpi(renderer->GetHwnd(), cursor);
	auto [cw, ch] = GetDpiSize(state->Width(), state->Height());

	auto tw = metrix.width;
	auto th = metrix.height;
	auto w = mx + th + mx + tw + mx;
	auto h = th + 2.f * my;

	auto ax = (cx + w < cw) ? 0.f : -1.f;
	auto ay = (cy - h > 0) ? -1.f : 0.f;

	auto x = cx + ax * w;
	auto y = cy + ay * h;

	rt->FillRoundedRectangle(D2D1::RoundedRect(D2D1::RectF(x, y, x + w, y + h), 4.f, 4.f), renderer->D2DBgColorBrush());
	rt->FillEllipse(D2D1::Ellipse(D2D1::Point2F(x + mx + th / 2.f, y + my + th / 2.f), th / 2.f, th / 2.f), colorBrush.get());
	rt->DrawTextLayout(D2D1::Point2F(x + mx + th + mx, y + my), text.get(), renderer->D2DFgColorBrush());
}

void ColorPickPass::SetDpi(int dpiX, int dpiY)
{
	dpiX_ = dpiX;
	dpiY_ = dpiY;
}

uint32_t ColorPickPass::PickColor(ID3D11DeviceContext* context, ID3D11Texture2D* capture, int x, int y)
{
	D3D11_BOX box{};
	box.left = x;
	box.top = y;
	box.front = 0;
	box.right = x + 1;
	box.bottom = y + 1;
	box.back = 1;
	context->CopySubresourceRegion(staging_.get(), 0, 0, 0, 0, capture, 0, &box);

	D3D11_MAPPED_SUBRESOURCE mapped{};
	check_hresult(context->Map(staging_.get(), 0, D3D11_MAP_READ, 0, &mapped));

	uint32_t color = *reinterpret_cast<uint32_t*>(mapped.pData);

	context->Unmap(staging_.get(), 0);
	return color;
}

inline auto RGB2HSV(uint8_t r, uint8_t g, uint8_t b)
{
	auto imax = max(max(r, g), b);
	auto imin = min(min(r, g), b);

	uint16_t h = 0;
	float s = imax == 0 ? 0.f : static_cast<float>(imax - imin) / static_cast<float>(imax);
	float v = static_cast<float>(imax) / 255.f;

	if (imin == imax) {
		h = 0;
	}
	else if (imax == r) {
		auto d = 60.f * static_cast<float>(g - b) / static_cast<float>(imax - imin);
		h = static_cast<uint16_t>(d < 0.f ? d + 360.f : d);
	}
	else if (imax == g) {
		auto d = static_cast<uint16_t>(60.f * static_cast<float>(b - r) / static_cast<float>(imax - imin) + 120.f);
		h = static_cast<uint16_t>(d < 0.f ? d + 360.f : d);
	}
	else if (imax == b) {
		auto d = static_cast<uint16_t>(60.f * static_cast<float>(r - g) / static_cast<float>(imax - imin) + 240.f);
		h = static_cast<uint16_t>(d < 0.f ? d + 360.f : d);
	}

	return tuple<int, float, float>{ h, s, v };
}

winrt::com_ptr<IDWriteTextLayout> ColorPickPass::GetLayoutedText(IDWriteFactory* factory, IDWriteTextFormat* textFormat, uint32_t color, EColorPickMode mode)
{
	WCHAR text[64] = {};

	uint8_t b = (color & 0x000000ff) >> 0;
	uint8_t g = (color & 0x0000ff00) >> 8;
	uint8_t r = (color & 0x00ff0000) >> 16;
	uint8_t a = (color & 0xff000000) >> 24;

	switch (mode) {
	case EColorPickMode::RGB:
		swprintf_s(text, L"RGB: %3u %3u %3u", r, g, b);
		break;

	case EColorPickMode::HSV:
	{
		auto [h, s, v] = RGB2HSV(r, g, b);
		swprintf_s(text, L"HSV: %3uÅã %.0f%% %.0f%%", h, 100.f * s, 100.f * v);
	}
		break;

	default:
		return {};
	}

	com_ptr<IDWriteTextLayout> layout{};
	check_hresult(factory->CreateTextLayout(text, (UINT32)wcslen(text), textFormat, 10000, 10000, layout.put()));

	DWRITE_TEXT_METRICS metrix{};
	layout->GetMetrics(&metrix);

	return layout;
}

std::tuple<float, float> ColorPickPass::ScreenToClientDpi(HWND hwnd, const POINT& pos)
{
	auto p = pos;
	ScreenToClient(hwnd, &p);
	return { static_cast<float>((p.x * 96) / dpiX_), static_cast<float>((p.y * 96) / dpiY_)};
}

std::tuple<float, float> ColorPickPass::GetDpiSize(int width, int height)
{
	return { static_cast<float>((width * 96) / dpiX_), static_cast<float>((height * 96) / dpiY_)};
}
