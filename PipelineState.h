#pragma once
#include "common.h"

class PipelineState
{
public:
	PipelineState();
	~PipelineState();

	void	Init(ID3D11Device* device, HWND hwnd);
	bool	Update(ID3D11Device* device, ID3D11DeviceContext* context, winrt::com_ptr<ID3D11Texture2D>& capture);

private:
	std::mutex mtx_;

	struct ConstantVariable
	{
		int		left;
		int		top;
		int		width;
		int		height;

		int		steps;
		int		numCel;
		float	invNumCel;

		float	opacity;
		float	scale;

		float	mask[3];
	};
	static_assert(sizeof(ConstantVariable) % 16 == 0);

	ConstantVariable	cv_ = {};
	EHistogramMode		histogramMode_ = EHistogramMode::RGB;
	EViewMode			viewMode_ = EViewMode::Color;
	EColorPickMode		colorPickMode_ = EColorPickMode::None;

	int		groupNumX_ = 1;
	int		groupNumY_ = 1;
	int		groupNumZ_ = 1;
	int		maxResolution_ = 1;
	HWND	hwnd_ = NULL;

	winrt::com_ptr<ID3D11Buffer>	constantBuffer_{ nullptr };
	winrt::com_ptr<ID3D11Buffer>	histogramBuffer_{ nullptr };

public:
	static constexpr int	HistogramResolution = 256;
	static constexpr int	HistogramElemSize = 4 * 4;
	static constexpr int	HistogramBufferSize = HistogramElemSize * HistogramResolution;

	static constexpr int	ThreadTilingX = 8;
	static constexpr int	ThreadTilingY = 8;

	int Width() {
		std::unique_lock<std::mutex> lock(mtx_);
		return cv_.width;
	}

	int Height() {
		std::unique_lock<std::mutex> lock(mtx_);
		return cv_.height;
	}

	int NumSteps() {
		std::unique_lock<std::mutex> lock(mtx_);
		return cv_.steps;
	}

	float Opacity() {
		std::unique_lock<std::mutex> lock(mtx_);
		return cv_.opacity;
	}

	void SetOpacity(float opacity) {
		std::unique_lock<std::mutex> lock(mtx_);
		cv_.opacity = opacity;
	}

	float Scale() {
		std::unique_lock<std::mutex> lock(mtx_);
		return cv_.scale;
	}

	void SetScale(float scale) {
		std::unique_lock<std::mutex> lock(mtx_);
		cv_.scale = scale;
	}

	EHistogramMode HistogramMode() {
		std::unique_lock<std::mutex> lock(mtx_);
		return histogramMode_;
	}

	void SetHistogramMode(EHistogramMode mode) {
		std::unique_lock<std::mutex> lock(mtx_);
		histogramMode_ = mode;
	}

	EViewMode ViewMode() {
		std::unique_lock<std::mutex> lock(mtx_);
		return viewMode_;
	}

	void SetViewMode(EViewMode mode) {
		std::unique_lock<std::mutex> lock(mtx_);
		viewMode_ = mode;
	}

	std::tuple<bool, bool, bool> ColorMask(){
		std::unique_lock<std::mutex> lock(mtx_);
		return {
			static_cast<bool>(cv_.mask[0]),
			static_cast<bool>(cv_.mask[1]),
			static_cast<bool>(cv_.mask[2])
		};
	}

	void SetColorMask(bool r, bool g, bool b)
	{
		std::unique_lock<std::mutex> lock(mtx_);
		cv_.mask[0] = static_cast<float>(r);
		cv_.mask[1] = static_cast<float>(g);
		cv_.mask[2] = static_cast<float>(b);
	}

	EColorPickMode ColorPickMode() {
		std::unique_lock<std::mutex> lock(mtx_);
		return colorPickMode_;
	}
		
	void SetColorPickMode(EColorPickMode mode){
		std::unique_lock<std::mutex> lock(mtx_);
		colorPickMode_ = mode;
	}

	std::tuple<int, int, int> NumThreadGroups() {
		std::unique_lock<std::mutex> lock(mtx_);
		return { groupNumX_, groupNumY_, groupNumZ_ };
	}

	winrt::com_ptr<ID3D11Buffer> HistogramBuffer() {
		std::unique_lock<std::mutex> lock(mtx_);
		return histogramBuffer_;
	}

	void SetMaxResolution(int resolution) {
		std::unique_lock<std::mutex> lock(mtx_);
		maxResolution_ = resolution;
	}

	auto ClientLeft() {
		std::unique_lock<std::mutex> lock(mtx_);
		return cv_.left;
	}

	auto ClientTop() {
		std::unique_lock<std::mutex> lock(mtx_);
		return cv_.top;
	}

	auto ClientRight() {
		std::unique_lock<std::mutex> lock(mtx_);
		return (cv_.left + cv_.width);
	}

	auto ClientBottom() {
		std::unique_lock<std::mutex> lock(mtx_);
		return (cv_.top + cv_.height);
	}
};

