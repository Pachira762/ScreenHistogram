#pragma once
#include "common.h"

class ColorPickPass
{
public:
	void	Init(class Renderer* renderer);
	void	AddPass(class Renderer* renderer, class PipelineState* state, ID3D11Texture2D* capture, int x, int y);

private:
	winrt::com_ptr<ID3D11Texture2D>		staging_{nullptr};
	winrt::com_ptr<IDWriteTextFormat>	textFormat_{ nullptr };

	uint32_t PickColor(ID3D11DeviceContext* context, ID3D11Texture2D* capture, int sx, int sy);
	winrt::com_ptr<IDWriteTextLayout>	GetLayoutedText(IDWriteFactory* factory, IDWriteTextFormat* textFormat, uint32_t color, EColorPickMode mode);
};
