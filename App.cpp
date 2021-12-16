#include "App.h"
#include "common.h"
#include "WinUtil.h"
#include "Theme.h"
#include "AppConfig.h"
#include "AppWindow.h"
#include "Renderer.h"
#include "ScreenCapture.h"
#include "PipelineState.h"
#include "ComputePass.h"
#include "PlanePass.h"
#include "GraphPass.h"
#include "ColorPickPass.h"

using namespace ::std;
using namespace ::winrt;

App::App():
	config_(make_unique<AppConfig>(L"ScreenHistogram.ini")),
	window_(make_unique<AppWindow>(this)),
	renderer_(make_unique<Renderer>()),
	capture_(make_unique<ScreenCapture>()),
	state_(make_unique<PipelineState>()),
	computePass_(make_unique<ComputePass>()),
	planePass_(make_unique<PlanePass>()),
	graphPass_(make_unique<GraphPass>()),
	colorPickPass_(make_unique<ColorPickPass>())
{
}

App::~App()
{
}

bool App::Initialize()
{
	try {
		auto x = config_->WindowPosX.Load(100);
		auto y = config_->WindowPosY.Load(100);
		auto width = config_->WindowWidth.Load(800);
		auto height = config_->WindowHeight.Load(600);
		auto fps = config_->FrameRate.Load(60);

		auto histogramMode = clamp(config_->HistogramMode.Load(EHistogramMode::RGB), EHistogramMode::RGB, EHistogramMode::Saturation);
		auto viewMode = clamp(config_->ViewMode.Load(EViewMode::Color), EViewMode::Color, EViewMode::Saturation);
		auto colorPickMode = clamp(config_->ColorPickMode.Load(EColorPickMode::None), EColorPickMode::None, EColorPickMode::HSV);
		auto maxResolution = config_->MaxResolution.Load(512);
		auto opacity = config_->Opacity.Load(0.5f);
		auto scale = config_->Scale.Load(5.f);

		HWND hwnd = window_->Create(x, y, width, height, histogramMode, viewMode, scale, opacity);
		WINRT_VERIFY(hwnd);

		int dpi = WinUtil::GetDpi();
		dpiChanged_ = false;
		dpiX_ = dpi;
		dpiY_ = dpi;

		renderer_->Init(hwnd, dpi, dpi);
		capture_->Init(renderer_->Device());
		state_->Init(renderer_->Device(), hwnd);
		computePass_->Init(renderer_->Device(), *state_.get());
		planePass_->Init(renderer_->Device(), *state_.get());
		graphPass_->Init(renderer_->Device(), *state_.get());
		colorPickPass_->Init(renderer_.get(), dpi, dpi);

		state_->SetOpacity(opacity);
		state_->SetScale(scale);
		state_->SetHistogramMode(histogramMode);
		state_->SetViewMode(viewMode);
		state_->SetMaxResolution(maxResolution);

		bContinue_ = true;
		bCapture_ = true;
		thread_ = thread(&App::CaptureProcess, this, window_->GetHandle());

		window_->Show();
		return true;
	}
	catch (...) {
		return false;
	}
}

void App::Finalize()
{
	if (thread_.joinable()) {
		bContinue_ = false;
		thread_.join();
	}
}

void App::CaptureProcess(HWND hwnd)
{
	auto sleepTime = chrono::milliseconds(2000 / config_->FrameRate);
	bool dpiChangedEver = false;

	while (bContinue_) {
		if (!bCapture_) {
			this_thread::sleep_for(chrono::milliseconds(100));
			continue;
		}

		if (dpiChanged_) {
			dpiChanged_ = false;
			renderer_->SetDpi(dpiX_, dpiY_);
			colorPickPass_->SetDpi(dpiX_, dpiY_);
			dpiChangedEver = true;
		}

		auto cap = capture_->Capture(500);
		if (!cap.succeeded) {
			this_thread::sleep_for(sleepTime);
			continue;
		}

		if (!cap.screenUpdated) {
			capture_->ReleaseCapture();
			this_thread::sleep_for(sleepTime);
			continue;
		}

		if (!state_->Update(renderer_->Device(), renderer_->Context(), cap.texture)) {
			capture_->ReleaseCapture();
			this_thread::sleep_for(sleepTime);
			continue;
		}

		renderer_->BeginDraw(state_->Width(), state_->Height());

		computePass_->AddPass(renderer_->Context(), *state_.get());
		planePass_->AddPass(renderer_->Context(), *state_.get());
		graphPass_->AddPass(renderer_->Context(), *state_.get());
		colorPickPass_->AddPass(renderer_.get(), state_.get(), cap.texture.get());
		DrawCloseButton();

		renderer_->EndDraw();
		capture_->ReleaseCapture();
		Sleep(0);
	}
}

void App::DrawCloseButton()
{
	if (close_.state == EButtonState::None) {
		return;
	}

	auto rt = renderer_->D2DRenderTarget();

	com_ptr<ID2D1SolidColorBrush> brush{};
	rt->CreateSolidColorBrush(D2D1::ColorF(close_.state == EButtonState::Hover ? 0xc42b1c : 0xB22A1C, 1.f), brush.put());

	float x = (float)close_.x;
	float y = (float)close_.y;
	float w = (float)close_.w;
	float h = (float)close_.h;
	rt->FillRectangle(D2D1::RectF(x, y, x + w, y + h), brush.get());

	brush = {};
	rt->CreateSolidColorBrush(D2D1::ColorF(0xBBBBBB, 1.f), brush.put());

	float s = 0.333f;
	float t = 1.f - s;
	rt->DrawLine(D2D1::Point2F(x + s * w, y + s * h), D2D1::Point2F(x + t * w, y + t * h), brush.get(), 1.5f);
	rt->DrawLine(D2D1::Point2F(x + s * w, y + t * h), D2D1::Point2F(x + t * w, y + s * h), brush.get(), 1.5f);
}

void App::OnCreate(HWND hwnd)
{
}

void App::OnClose()
{
	Finalize();

	RECT rect = {};
	GetWindowRect(window_->GetHandle(), &rect);

	config_->WindowPosX.Save(rect.left);
	config_->WindowPosY.Save(rect.top);
	config_->WindowWidth.Save(rect.right - rect.left);
	config_->WindowHeight.Save(rect.bottom - rect.top);
	config_->FrameRate.Save();

	config_->HistogramMode.Save(state_->HistogramMode());
	config_->ViewMode.Save(state_->ViewMode());
	config_->ColorPickMode.Save(state_->ColorPickMode());
	config_->Opacity.Save(state_->Opacity());
	config_->Scale.Save(state_->Scale());
	config_->MaxResolution.Save();
}

void App::OnDestory()
{
	Finalize();
}

void App::OnSizing()
{
	bCapture_ = true;
}

void App::OnMoving()
{
}

void App::OnMinimized()
{
	bCapture_ = false;
}

void App::OnDpiChanged(int dpiX, int dpiY)
{
	WinUtil::SetDpi(dpiX);
	Theme::SetDpi(dpiX);

	dpiChanged_ = true;
	dpiX_ = dpiX;
	dpiY_ = dpiY;
}

void App::SetHistogramMode(EHistogramMode mode)
{
	state_->SetHistogramMode(mode);
}

void App::SetViewMode(EViewMode mode)
{
	state_->SetViewMode(mode);
}

void App::SetScale(float scale)
{
	state_->SetScale(scale);
}

void App::SetOpacity(float opacity)
{
	state_->SetOpacity(opacity);
}

void App::SetColorPickMode(EColorPickMode mode)
{
	state_->SetColorPickMode(mode);
}

void App::SetCloseButtonState(int cx, int cy, int cw, int ch, EButtonState state)
{
	close_.x = cx;
	close_.y = cy;
	close_.w = cw;
	close_.h = ch;
	close_.state = state;
}
