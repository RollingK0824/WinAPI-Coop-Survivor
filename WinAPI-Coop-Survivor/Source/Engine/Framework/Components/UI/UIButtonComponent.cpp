// Source/Engine/Framework/Components/UI/UIButtonComponent.cpp
#include "Engine/Core/pch.h"
#include "UIButtonComponent.h"
#include "Engine/Core/ComponentRegister.h"
#include "Engine/Framework/Components/Core/TransformComponent.h"
#include "Engine/Framework/Components/UI/UIImageComponent.h"
#include "Engine/Framework/Components/UI/UIPanelComponent.h"
#include "Engine/Manager/InputManager.h"
#include "Engine/Framework/GameObject.h"

static ComponentRegistrar<UIButtonComponent> registrar(EngineKey::Component::UIButtonComponent.data());

UIButtonComponent::UIButtonComponent(GameObject* owner, TransformComponent* transform)
	: ScriptComponent(owner, transform)
{
	ExposeVariable("IsHovered", &m_bIsHovered);
}

void UIButtonComponent::Awake()
{
	ScriptComponent::Awake();
	m_pImgView = gameObject.GetComponent<UIImageComponent>();
	m_pPanelView = gameObject.GetComponent<UIPanelComponent>();
}

void UIButtonComponent::Update(float dt)
{
	ScriptComponent::Update(dt);

	if (CheckMouseOver())
	{
		m_bIsHovered = true;
		if (InputManager::GetInstance()->GetKeyDown(VK_LBUTTON))
		{
			if (m_onClick)
			{
				m_onClick();
			}
		}
	}
	else
	{
		m_bIsHovered = false;
	}
}

bool UIButtonComponent::CheckMouseOver()
{
	Vector2 mousePos = InputManager::GetInstance()->GetMousePosition();

	if (m_pImgView == nullptr && m_pPanelView == nullptr)
	{
		m_pImgView = gameObject.GetComponent<UIImageComponent>();
		m_pPanelView = gameObject.GetComponent<UIPanelComponent>();
	}

	Vector2 pos = transform.GetPosition();
	Vector2 scale = transform.GetScale();
	Vector2 size = { 100.0f, 30.0f };
	D2D1_POINT_2F pivot = { 0.5f, 0.5f };

	if (m_pImgView != nullptr)
	{
		const RenderCommand& cmd = m_pImgView->GetRenderCommand();
		pivot = cmd.pivot;
		float srcW = cmd.srcRect.right - cmd.srcRect.left;
		float srcH = cmd.srcRect.bottom - cmd.srcRect.top;
		if (srcW > 0.0f && srcH > 0.0f)
		{
			size = { srcW, srcH };
		}
		else
		{
			size = m_pImgView->GetSize();
		}
	}
	else if (m_pPanelView != nullptr)
	{
		pivot = m_pPanelView->GetRenderCommand().pivot;
		size = m_pPanelView->GetSize();
	}

	float finalW = size.x * scale.x;
	float finalH = size.y * scale.y;

	float left = pos.x - finalW * pivot.x;
	float right = pos.x + finalW * (1.0f - pivot.x);
	float top = pos.y - finalH * pivot.y;
	float bottom = pos.y + finalH * (1.0f - pivot.y);

	return (mousePos.x >= left && mousePos.x <= right &&
			mousePos.y >= top && mousePos.y <= bottom);
}