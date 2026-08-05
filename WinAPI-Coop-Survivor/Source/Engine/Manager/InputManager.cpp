#include "Engine/Core/pch.h"
#include "InputManager.h"
#include "Engine/Core/GameApp.h"
#include "Engine/Renderer/GraphicManager.h"
#include "Engine/Manager/CameraManager.h"
#include "Engine/Editor/EditorSystem.h"
#include "Engine/Editor/Panels/ViewportPanel.h"
#include <imgui.h>

bool InputManager::Initialize()
{
	m_vKeyStates.resize(256, KeyState::NONE);
	m_vPrevStates.resize(256, false);

	return true;
}

void InputManager::Release()
{
	m_vKeyStates.clear();
	m_vPrevStates.clear();

	std::vector<KeyState>().swap(m_vKeyStates);
	std::vector<bool>().swap(m_vPrevStates);
}

void InputManager::Update(float dt)
{
	HWND hWnd = GameApp::GetInstance()->GetWindowHandle();
	bool isForeground = (GetForegroundWindow() == hWnd);
	ImGuiIO* io = (ImGui::GetCurrentContext() != nullptr) ? &ImGui::GetIO() : nullptr;

	bool isTypingText = (io != nullptr) && io->WantTextInput;
	for (int vkCode = 0; vkCode < 256; ++vkCode)
	{
		bool isCurrentPressed = false;
		bool isFunctionKey = (vkCode >= VK_F1 && vkCode <= VK_F12);
		if (isForeground && (!isTypingText || isFunctionKey))
		{
			isCurrentPressed = (GetAsyncKeyState(vkCode) & 0x8000) != 0;
		}
		bool isPrevPressed = m_vPrevStates[vkCode];

		if (isCurrentPressed)
		{
			if (!isPrevPressed)
				m_vKeyStates[vkCode] = KeyState::DOWN;
			else
				m_vKeyStates[vkCode] = KeyState::PRESS;
		}
		else
		{
			if (isPrevPressed)
				m_vKeyStates[vkCode] = KeyState::UP;
			else
				m_vKeyStates[vkCode] = KeyState::NONE;
		}

		m_vPrevStates[vkCode] = isCurrentPressed;
	}

	UpdateMousePosition();
}

void InputManager::UpdateMousePosition()
{
	Vector2 currentMousePos = { 0.0f, 0.0f };

#if WITH_EDITOR
	ImGuiIO* io = (ImGui::GetCurrentContext() != nullptr) ? &ImGui::GetIO() : nullptr;
	if (io != nullptr)
	{
		ViewportPanel* pVP = EditorSystem::GetInstance()->GetViewportPanel();
		if (pVP != nullptr)
		{
			ImVec2 vMin = pVP->GetViewportMin();
			ImVec2 vMax = pVP->GetViewportMax();
			float vpW = vMax.x - vMin.x;
			float vpH = vMax.y - vMin.y;

			if (vpW > 0.0f && vpH > 0.0f)
			{
				float u = (io->MousePos.x - vMin.x) / vpW;
				float v = (io->MousePos.y - vMin.y) / vpH;

				float renderW = GraphicManager::GetInstance()->GetScreenWidth();
				float renderH = GraphicManager::GetInstance()->GetScreenHeight();

				currentMousePos = Vector2(u * renderW, v * renderH);
			}
		}
	}
	else
#endif
	{
		HWND hWnd = GameApp::GetInstance()->GetWindowHandle();
		POINT pt;
		GetCursorPos(&pt);
		ScreenToClient(hWnd, &pt);

		RECT clientRect;
		GetClientRect(hWnd, &clientRect);
		float clientW = static_cast<float>(clientRect.right - clientRect.left);
		float clientH = static_cast<float>(clientRect.bottom - clientRect.top);

		float renderW = GraphicManager::GetInstance()->GetScreenWidth();
		float renderH = GraphicManager::GetInstance()->GetScreenHeight();

		if (clientW > 0.0f && clientH > 0.0f && renderW > 0.0f && renderH > 0.0f)
		{
			currentMousePos = Vector2((static_cast<float>(pt.x) / clientW) * renderW, (static_cast<float>(pt.y) / clientH) * renderH);
		}
		else
		{
			currentMousePos = Vector2(static_cast<float>(pt.x), static_cast<float>(pt.y));
		}
	}

	m_mousePos = currentMousePos;

	D2D1_POINT_2F worldPt = CameraManager::GetInstance()->ScreenToWorld(D2D1::Point2F(m_mousePos.x, m_mousePos.y));
	m_worldMousePos = Vector2(worldPt.x, worldPt.y);
}