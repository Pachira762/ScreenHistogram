#pragma once
#include "resource.h"

constexpr bool UseComposition = true;
constexpr bool UseTransparentSwapChain = true;

enum class EHistogramMode
{
	RGB,
	RGBBrightness,
	Brightness,
	Saturation,
};

enum class EViewMode
{
	Color,
	Brightness,
	Saturation,
};

enum class EColorPickMode
{
	None,
	RGB,
	HSV,
};

enum class EButtonState
{
	None,
	Hover,
	Pushed,
};
