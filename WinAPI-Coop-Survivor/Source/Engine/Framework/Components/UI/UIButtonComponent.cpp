#include "Engine/Core/pch.h"
#include "UIButtonComponent.h"
#include "Engine/Core/ComponentRegister.h"
#include "Engine/Manager/InputManager.h"
#include "Engine/Framework/GameObject.h"
#include "Engine/Framework/Components/UI/UIImageComponent.h"
#include "Engine/Framework/Components/Core/TransformComponent.h"

static ComponentRegistrar<UIButtonComponent> registrar(EngineKey::Component::UIButtonComponent.data());

UIButtonComponent::UIButtonComponent(GameObject* owner, TransformComponent* transform)
	: ScriptComponent(owner, transform)
{
}

void UIButtonComponent::Awake()
{
	m_pImgView = gameObject.GetComponent<UIImageComponent>();
}

void UIButtonComponent::Update(float dt)
{
	if (!m_bIsEnabled || !gameObject.IsActive()) return;

	m_bIsHovered = CheckMouseOver();

	if (m_bIsHovered && InputManager::GetInstance()->GetKeyDown(VK_LBUTTON))
	{
		if (m_onClick)
		{
			m_onClick();
		}
	}
}

bool UIButtonComponent::CheckMouseOver()
{
	POINT mousePos;
	GetCursorPos(&mousePos);
	HWND hWnd = GetActiveWindow();
	if (hWnd) ScreenToClient(hWnd, &mousePos);

	Vector2 uiPos = m_pImgView ? m_pImgView->GetPosition() : transform.GetPosition();
	Vector2 uiSize = m_pImgView ? m_pImgView->GetSize() : Vector2(100.0f, 30.0f);

	return (mousePos.x >= uiPos.x && mousePos.x <= uiPos.x + uiSize.x &&
		mousePos.y >= uiPos.y && mousePos.y <= uiPos.y + uiSize.y);
}