// Source/Engine/Framework/Components/UI/UIButtonComponent.cpp
#include "Engine/Core/pch.h"
#include "UIButtonComponent.h"
#include "UIImageComponent.h"
#include "Engine/Core/ComponentRegister.h"
#include "Engine/Manager/InputManager.h"
#include "Engine/Framework/GameObject.h"
#include "Engine/Framework/Components/Core/TransformComponent.h"

static ComponentRegistrar<UIButtonComponent> registrar(EngineKey::Component::UIButtonComponent.data());

UIButtonComponent::UIButtonComponent(GameObject* owner, TransformComponent* transform)
	: ScriptComponent(owner, transform)
{
	ExposeVariable("Is Hovered", &m_bIsHovered);
}

void UIButtonComponent::Awake()
{
	ScriptComponent::Awake();

	// 동일한 GameObject에 붙어있는 UIImageComponent 자동 참조
	if (m_pImgView == nullptr)
	{
		m_pImgView = gameObject.GetComponent<UIImageComponent>();
	}
}

void UIButtonComponent::Update(float dt)
{
	m_bIsHovered = CheckMouseOver();

	if (m_pImgView != nullptr)
	{
		if (m_bIsHovered)
		{
			m_pImgView->SetOpacity(0.8f);
		}
		else
		{
			m_pImgView->SetOpacity(1.0f);
		}
	}

	if (m_bIsHovered && InputManager::GetInstance()->GetKeyDown(VK_LBUTTON))
	{
		if (m_onClick != nullptr)
		{
			m_onClick();
		}
	}
}

bool UIButtonComponent::CheckMouseOver()
{
	if (m_pImgView == nullptr) return false;

	Vector2 pos = transform.GetPosition();
	Vector2 size = m_pImgView->GetSize();

	POINT mousePos;
	GetCursorPos(&mousePos);
	ScreenToClient(GetActiveWindow(), &mousePos);

	float mouseX = static_cast<float>(mousePos.x);
	float mouseY = static_cast<float>(mousePos.y);

	return (mouseX >= pos.x && mouseX <= pos.x + size.x &&
		mouseY >= pos.y && mouseY <= pos.y + size.y);
}

void UIButtonComponent::Serialize(json& outJson) const
{
	ScriptComponent::Serialize(outJson);
}

void UIButtonComponent::Deserialize(const json& inJson)
{
	ScriptComponent::Deserialize(inJson);
}