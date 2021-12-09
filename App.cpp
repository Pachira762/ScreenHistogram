#include "App.h"
#include <chrono>
#include "common.h"
#include "dutil.h"
#include "Viewer.h"
#include "ScreenCapture.h"
#include "AppWindow.h"
#include "AppConfig.h"

using namespace ::std;
using namespace ::winrt;

static constexpr auto ConfigFileName = L"ScreenHistogram.ini";

App::App():
	window_(make_unique<AppWindow>(this)),
	capture_(make_unique<ScreenCapture>()),
	viewer_(make_unique<Viewer>())
{
}

App::~App()
{
}

bool App::Initialize()
{
	try {
		AppConfig appConfig(ConfigFileName);
		auto x = appConfig.WindowPosX.Load(100);
		auto y = appConfig.WindowPosY.Load(100);
		auto width = appConfig.WindowWidth.Load(800);
		auto height = appConfig.WindowHeight.Load(600);
		frameRate_ = max(1, appConfig.FrameRate.Load(60));

		static ViewerConfig viewerConfig{};
		viewerConfig.ResolutionLimit = appConfig.MaxResolution.Load(512);
		viewerConfig.Opacity = appConfig.Opacity.Load(1.f);
		viewerConfig.Scale = appConfig.Scale.Load(5.f);

		HWND hwnd = window_->Create(x, y, width, height);
		if (!hwnd) {
			DLOG(L"Failed create app window\n");
			return false;
		}

		auto device = CreateD3DDevice(D3D_DRIVER_TYPE_HARDWARE);

		if (!viewer_->Init(hwnd, device, viewerConfig)) {
			DLOG(L"Failed initialize Histogram\n");
			return false;
		}

		if (!capture_->Initialize(viewer_.get(), device)) {
			DLOG(L"Failed initialize Capture\n");
			return false;
		}

		bContinue_ = true;
		bCapture_ = true;
		thread_ = thread(&App::CaptureProcess, this, window_->GetHandle());

		window_->Show();
		return true;
	}
	catch (...) {
		DLOG(L"Failed App::Initialize\n");
		return false;
	}
}

void App::Finalize()
{
	if (thread_.joinable()) {
		bContinue_ = false;
		thread_.join();
	}

	capture_->Finalize();
}

void App::CaptureProcess(HWND hwnd)
{
	int sleepTime = 2000 / frameRate_;

	while (bContinue_) {
		if (!bCapture_) {
			this_thread::sleep_for(chrono::milliseconds(100));
			continue;
		}

		capture_->Capture(sleepTime);
		//PostMessage(hwnd, WM_APP_CHECKTRANSPARENCY, 0, 0);
	}
}

void App::OnCreate(HWND hwnd)
{
}

void App::OnClose()
{
	Finalize();

	auto rect = window_->GetRect();

	AppConfig appConfig(ConfigFileName);
	appConfig.WindowPosX.Save(rect.left);
	appConfig.WindowPosY.Save(rect.top);
	appConfig.WindowWidth.Save(rect.right - rect.left);
	appConfig.WindowHeight.Save(rect.bottom - rect.top);
	appConfig.FrameRate.Save(frameRate_);

	auto viewerConfig = viewer_->GetConfig();
	appConfig.MaxResolution.Save(viewerConfig.ResolutionLimit);
	appConfig.Opacity.Save(viewerConfig.Opacity);
	appConfig.Scale.Save(viewerConfig.Scale);
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
