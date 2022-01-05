#pragma once
#include "resource.h"

constexpr bool UseComposition = true;
constexpr bool UseTransparentSwapChain = true;

enum class EHistogramMode
{
	RGB,
	RGBBrightness,
	Brightness,
	NumMode
};

enum class EViewMode
{
	Color,
	Brightness,
	SaturationHSV,
	SaturationHLS,
	ColorMask,
	NumMode
};

enum class EColorPickMode
{
	None,
	RGB,
	HSV,
	HLS,
	NumMode
};

