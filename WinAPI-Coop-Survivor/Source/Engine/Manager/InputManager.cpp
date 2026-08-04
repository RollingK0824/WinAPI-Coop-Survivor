#include "Engine/Core/pch.h"
#include "InputManager.h"
#include "Engine/Core/GameApp.h"

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
	bool isImGuiUsingKeyboard = (io != nullptr) && io->WantCaptureKeyboard;

	for (int vkCode = 0; vkCode < 256; ++vkCode)
	{
		bool isCurrentPressed = false;

		bool isFunctionKey = (vkCode >= VK_F1 && vkCode <= VK_F12);

		if (isForeground && (!isImGuiUsingKeyboard || isFunctionKey))
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
}