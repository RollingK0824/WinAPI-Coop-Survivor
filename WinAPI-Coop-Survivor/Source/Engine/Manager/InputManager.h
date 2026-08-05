#pragma once
#include "Engine/Core/Singleton.h"
#include "Engine/Framework/Base/ISystem.h"
#include "Engine/Framework/Base/IUpdatable.h"

enum class KeyState { NONE, DOWN, PRESS, UP };

class InputManager : public Singleton<InputManager>, public ISystem, public IUpdatable
{
	friend class Singleton<InputManager>;

public:
	virtual bool Initialize() override;
	virtual void Release() override;
	virtual void Update(float dt) override;

	bool GetKeyDown(int vkCode) const { return m_vKeyStates[vkCode] == KeyState::DOWN; }
	bool GetKeyPress(int vkCode) const { return m_vKeyStates[vkCode] == KeyState::PRESS; }
	bool GetKeyUp(int vkCode) const { return m_vKeyStates[vkCode] == KeyState::UP; }

	Vector2 GetMousePosition() const { return m_mousePos; }
	Vector2 GetWorldMousePosition() const { return m_worldMousePos; }

private:
	InputManager() = default;
	virtual ~InputManager() = default;

	void UpdateMousePosition();

	std::vector<KeyState> m_vKeyStates;
	std::vector<bool> m_vPrevStates;

	Vector2 m_mousePos = { 0.0f, 0.0f };
	Vector2 m_worldMousePos = { 0.0f, 0.0f };
};