#pragma once
#include "common.h"

class ColorPickPass
{
public:
	void	Init(class Renderer* renderer, int dpiX, int dpiY);
	void	AddPass(class Renderer* renderer, class PipelineState* state, ID3D11Texture2D* capture);
	void	SetDpi(int dpiX, int dpiY);

private:
	int		dpiX_ = 96;
	int		dpiY_ = 96;

	winrt::com_ptr<ID3D11Texture2D>		staging_{nullptr};
	winrt::com_ptr<IDWriteTextFormat>	textFormat_{ nullptr };

	uint32_t PickColor(ID3D11DeviceContext* context, ID3D11Texture2D* capture, int sx, int sy);
	winrt::com_ptr<IDWriteTextLayout>	GetLayoutedText(IDWriteFactory* factory, IDWriteTextFormat* textFormat, uint32_t color, EColorPickMode mode);

	std::tuple<float, float>	ScreenToClientDpi(HWND hwnd, const POINT& pos);
	std::tuple<float, float>	GetDpiSize(int width, int height);
};
