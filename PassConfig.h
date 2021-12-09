#pragma once

struct ConstantVariables
{
	int		left;
	int		top;
	int		width;
	int		height;

	int		step;
	int		numCel;
	float	invNumCel;

	float	graphOpacity;
	float	graphScale;

	int		padding[3];
};

static_assert(sizeof(ConstantVariables) % 16 == 0);

struct PassConfig
{
	static constexpr int	HistogramResolution = 256;
	static constexpr int	HistogramElemSize = 4 * 4;
	static constexpr int	HistogramBuffSize = HistogramElemSize * HistogramResolution;

	static constexpr int	ThreadTilingX = 8;
	static constexpr int	ThreadTilingY = 8;

	ConstantVariables	cv;

	int		threadGroupX;
	int		threadGroupY;
	int		maxResolution;
};
