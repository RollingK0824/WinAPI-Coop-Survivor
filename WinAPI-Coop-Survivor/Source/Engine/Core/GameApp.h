#pragma once
#include "Engine/Core/Singleton.h"
#include "Engine/Core/Define.h"

struct ID2D1Bitmap;

class GameApp : public Singleton<GameApp>
{
	friend class Singleton<GameApp>;

public:
	// 윈도우 생성 및 초기화
	bool Initialize(HINSTANCE hInstance, int nCmdShow, DisplayMode mode = DisplayMode::Windowed);

	// 게임 메인 루프
	int Run();

	// 엔진 종료 및 메모리 해제
	void Release();

	void RegisterManagers();

	HWND GetWindowHandle() const { return m_hWnd; }
	HINSTANCE GetInstanceHandle() const { return m_hInstance; }

private:
	GameApp() = default;
	virtual ~GameApp() = default;

private:
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	HWND m_hWnd = nullptr;
	HINSTANCE m_hInstance = nullptr;
	bool m_bIsRunning = true;
};