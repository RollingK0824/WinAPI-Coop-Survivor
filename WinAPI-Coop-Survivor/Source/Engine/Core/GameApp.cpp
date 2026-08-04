#include "Engine/Core/pch.h"
#include "Engine/Core/Define.h"
#include "GameApp.h"
#include "Engine/Core/EventBus.h"
#include "Engine/Core/ObjectPool.h"
#include "Engine/Core/EngineKernel.h"
#include "Engine/Manager/TimeManager.h"
#include "Engine/Manager/ResourceManager.h"
#include "Engine/Manager/InputManager.h"
#include "Engine/Manager/RandomManager.h"
#include "Engine/Manager/SceneManager.h"
#include "Engine/Manager/ActionManager.h"
#include "Engine/Manager/PrefabManager.h"
#include "Engine/Manager/CameraManager.h"
#include "Engine/Manager/DebugManager.h"
#include "Engine/Network/NetworkManager.h"
#include "Engine/Physics/PhysicsManager.h"
#include "Engine/Renderer/RenderSystem.h"
#include "Engine/Renderer/GraphicManager.h"
#include "Engine/Editor/EditorSystem.h"

#include <imgui.h>
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

bool GameApp::Initialize(HINSTANCE hInstance, int nCmdShow, DisplayMode mode)
{
	m_hInstance = hInstance;

	HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

	if (FAILED(hr))
	{
		MessageBoxW(nullptr, L"OS COM 라이브러리 초기화 실패", L"Fatal Error", MB_ICONERROR);
		return false;
	}

	WNDCLASSEXW wc = {};
	wc.cbSize = sizeof(WNDCLASSEXW);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.hInstance = m_hInstance;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszClassName = L"D2D_GameEngine";

	if (!RegisterClassExW(&wc))
	{
		MessageBoxW(nullptr, L"윈도우 클래스 등록 실패", L"Error", MB_ICONERROR);
		return false;
	}

	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);

	DWORD windowStyle = WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_MAXIMIZE; // 기본 윈도우 스타일
	int width = GWinSizeX;
	int height = GWinSizeY;

	switch (mode)
	{
	case DisplayMode::Windowed:
		// 기본 스타일 유지
		break;
	case DisplayMode::Borderless:
		windowStyle = WS_POPUP | WS_VISIBLE;
		width = GetSystemMetrics(SM_CXSCREEN);
		height = GetSystemMetrics(SM_CYSCREEN);
		break;
	case DisplayMode::Fullscreen:
		windowStyle = WS_POPUP | WS_VISIBLE;
		width = GetSystemMetrics(SM_CXSCREEN);
		height = GetSystemMetrics(SM_CYSCREEN);

		DEVMODE dmScreenSettings = {};
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth = (DWORD)width;
		dmScreenSettings.dmPelsHeight = (DWORD)height;
		dmScreenSettings.dmBitsPerPel = 32;
		dmScreenSettings.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL;
		if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
		{
			MessageBoxW(nullptr, L"전체 화면 모드 변경 실패", L"Error", MB_ICONERROR);
		}
		break;
	}

	m_hWnd = CreateWindowW(L"D2D_GameEngine", L"D2D Game Engine",
		windowStyle, 0, 0, width, height,
		nullptr, nullptr, m_hInstance, nullptr);

	if (!m_hWnd) return false;

	RegisterManagers();

	if (!EngineKernel::GetInstance()->Initialize())
	{
		MessageBoxW(nullptr, L"EngineKernel 초기화 실패", L"Error", MB_ICONERROR);
		return false;
	}

	m_bIsRunning = true;

	ResourceManager::GetInstance()->LoadResourcesFromJson(EngineKey::FilePath::ResourceList.data());

	SceneManager::GetInstance()->LoadSceneFromFile(EngineKey::FilePath::DefaultScene.data());

	ShowWindow(m_hWnd, SW_MAXIMIZE);
	UpdateWindow(m_hWnd);

	return true;
}

int GameApp::Run()
{
	MSG msg = {};

	while (m_bIsRunning)
	{
		if (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				m_bIsRunning = false;
			}
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
		else
		{
			// TODO: 게임 업데이트 및 렌더링 로직 추가
			EngineKernel::GetInstance()->ProcessFrame();
		}
	}

	return (int)msg.wParam;
}

void GameApp::Release()
{
	// TODO: 각종 해제 코드 추가
	ChangeDisplaySettings(nullptr, 0); // 전체 화면 모드 해제

	EngineKernel::GetInstance()->Release();

	CoUninitialize();
}

void GameApp::RegisterManagers()
{
	EngineKernel* kernel = EngineKernel::GetInstance();

	// 기초 시스템 & 입력
	kernel->RegisterManager(TimeManager::GetInstance());
	kernel->RegisterManager(InputManager::GetInstance());
	kernel->RegisterManager(RandomManager::GetInstance());
	kernel->RegisterManager(EventBus::GetInstance());
	kernel->RegisterManager(PoolManager::GetInstance());

	// 리소스 & 네트워크
	kernel->RegisterManager(ResourceManager::GetInstance());
	kernel->RegisterManager(PrefabManager::GetInstance());
	kernel->RegisterManager(NetworkManager::GetInstance());

	// 게임 로직 (Update)
	kernel->RegisterManager(ActionManager::GetInstance());
	kernel->RegisterManager(SceneManager::GetInstance());

	// 물리 연산 (Step)
	kernel->RegisterManager(PhysicsManager::GetInstance());

	// 카메라 & 매니저
	kernel->RegisterManager(CameraManager::GetInstance());

	// 렌더링
	kernel->RegisterManager(GraphicManager::GetInstance());
	kernel->RegisterManager(RenderSystem::GetInstance());
	kernel->RegisterManager(DebugManager::GetInstance());
#if WITH_EDITOR
	kernel->RegisterManager(GUISystem::GetInstance());
	kernel->RegisterManager(EditorSystem::GetInstance());
#endif
}

LRESULT CALLBACK GameApp::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (ImGui::GetCurrentContext() != nullptr)
	{
		if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
			return true;
	}

	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_SIZE:
		if (wParam != SIZE_MINIMIZED) // 최소화 상태가 아닐 때만 실행
		{
			UINT width = LOWORD(lParam);
			UINT height = HIWORD(lParam);
			GraphicManager::GetInstance()->OnResize(width, height);
		}
		return 0;
	}

	return DefWindowProcW(hWnd, message, wParam, lParam);
}

int APIENTRY WinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nCmdShow)
{

	if (!GameApp::GetInstance()->Initialize(hInstance, nCmdShow, DisplayMode::Windowed))
	{
		return 0;
	}

	int result = GameApp::GetInstance()->Run();

	GameApp::GetInstance()->Release();

	return result;
}
