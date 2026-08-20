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
	ExposeTexture("Normal Sprite", &m_normalSpriteKey);
	ExposeTexture("Hover Sprite", &m_hoverSpriteKey);
	ExposeTexture("Pressed Sprite", &m_pressedSpriteKey);
	ExposeTexture("Disabled Sprite", &m_disabledSpriteKey);
	ExposeVariable("Normal Color", &m_normalColor);
	ExposeVariable("Hover Color", &m_hoverColor);
	ExposeVariable("Pressed Color", &m_pressedColor);
	ExposeVariable("Disabled Color", &m_disabledColor);
}

void UIButtonComponent::Awake()
{
	ScriptComponent::Awake();
	auto it = std::find(s_allButtons.begin(), s_allButtons.end(), this);
	if (it == s_allButtons.end())
	{
		s_allButtons.push_back(this);
	}

	m_pImgView = gameObject.GetComponent<UIImageComponent>();
	m_pPanelView = gameObject.GetComponent<UIPanelComponent>();

	if (m_normalSpriteKey.empty())
	{
		if (m_pImgView != nullptr)
		{
			m_normalSpriteKey = m_pImgView->GetSpriteKey();
		}
		else if (m_pPanelView != nullptr)
		{
			m_normalSpriteKey = m_pPanelView->GetSpriteKey();
		}
	}
}

void UIButtonComponent::OnDestroy()
{
	auto it = std::find(s_allButtons.begin(), s_allButtons.end(), this);
	if (it != s_allButtons.end())
	{
		s_allButtons.erase(it);
	}
	ScriptComponent::OnDestroy();
}

int UIButtonComponent::GetEffectiveZOrder() const
{
	if (m_pImgView != nullptr) return m_pImgView->GetRenderCommand().zOrder;
	if (m_pPanelView != nullptr) return m_pPanelView->GetRenderCommand().zOrder;

	GameObject* parent = gameObject.GetParent();
	while (parent != nullptr)
	{
		auto* renderComp = parent->GetComponent<RenderComponent>();
		if (renderComp != nullptr) return renderComp->GetRenderCommand().zOrder;
		parent = parent->GetParent();
	}

	return 0;
}

bool UIButtonComponent::IsBlockedByHigherPanel(int myZOrder) const
{
	for (auto* panel : UIPanelComponent::GetAllPanels())
	{
		if (panel == nullptr) continue;
		if (!panel->IsEnabled() || !panel->gameObject.IsActiveInHierarchy()) continue;

		if (this->gameObject.IsDescendantOf(&panel->gameObject) || &this->gameObject == &panel->gameObject)
		{
			continue;
		}

		int panelZ = panel->GetRenderCommand().zOrder;
		if (panelZ > myZOrder)
		{
			if (panel->CheckMouseOver())
			{
				return true;
			}
		}
	}
	return false;
}

UIButtonComponent* UIButtonComponent::GetTopmostHoveredButton()
{
	UIButtonComponent* bestBtn = nullptr;
	int bestZ = INT_MIN;
	size_t bestHierarchy = 0;

	for (auto* btn : s_allButtons)
	{
		if (btn == nullptr) continue;
		if (!btn->IsEnabled() || !btn->gameObject.IsActiveInHierarchy()) continue;

		if (btn->CheckMouseOver())
		{
			int z = btn->GetEffectiveZOrder();

			if (btn->IsBlockedByHigherPanel(z))
			{
				continue;
			}

			size_t hierarchy = btn->gameObject.GetHierarchyIndex();

			if (bestBtn == nullptr || z > bestZ || (z == bestZ && hierarchy > bestHierarchy))
			{
				bestBtn = btn;
				bestZ = z;
				bestHierarchy = hierarchy;
			}
		}
	}
	return bestBtn;
}

void UIButtonComponent::PostDeserialize(Scene* pScene)
{
	ScriptComponent::PostDeserialize(pScene);
	m_pImgView = gameObject.GetComponent<UIImageComponent>();
	m_pPanelView = gameObject.GetComponent<UIPanelComponent>();

	if (m_normalSpriteKey.empty())
	{
		if (m_pImgView != nullptr)
		{
			m_normalSpriteKey = m_pImgView->GetSpriteKey();
		}
		else if (m_pPanelView != nullptr)
		{
			m_normalSpriteKey = m_pPanelView->GetSpriteKey();
		}
	}
}

void UIButtonComponent::Update(float dt)
{
	ScriptComponent::Update(dt);

	if (!IsEnabled() || !gameObject.IsActiveInHierarchy())
	{
		m_bIsPressed = false;
		m_bIsHovered = false;
		ApplyVisualState(ButtonVisualState::Disabled);
		return;
	}

	UIButtonComponent* topmost = GetTopmostHoveredButton();
	bool isTopmostHovered = (topmost == this);
	m_bIsHovered = isTopmostHovered;

	InputManager* input = InputManager::GetInstance();

	if (input->GetKeyDown(VK_LBUTTON))
	{
		if (isTopmostHovered)
		{
			m_bIsPressed = true;
		}
	}

	if (input->GetKeyUp(VK_LBUTTON))
	{
		if (m_bIsPressed)
		{
			m_bIsPressed = false;
			if (isTopmostHovered)
			{
				if (m_onClick)
				{
					m_onClick();
				}
			}
		}
	}

	if (!input->GetKeyPress(VK_LBUTTON) && !input->GetKeyDown(VK_LBUTTON))
	{
		m_bIsPressed = false;
	}

	if (m_bIsPressed && isTopmostHovered)
	{
		ApplyVisualState(ButtonVisualState::Pressed);
	}
	else if (isTopmostHovered)
	{
		ApplyVisualState(ButtonVisualState::Hover);
	}
	else
	{
		ApplyVisualState(ButtonVisualState::Normal);
	}
}

void UIButtonComponent::ApplyVisualState(ButtonVisualState state)
{
	if (m_visualState == state && m_bVisualInitialized) return;
	m_visualState = state;
	m_bVisualInitialized = true;

	if (m_pImgView == nullptr && m_pPanelView == nullptr)
	{
		m_pImgView = gameObject.GetComponent<UIImageComponent>();
		m_pPanelView = gameObject.GetComponent<UIPanelComponent>();
	}

	std::wstring targetSprite = m_normalSpriteKey;
	D2D1_COLOR_F targetColor = m_normalColor;

	switch (state)
	{
	case ButtonVisualState::Normal:
		targetSprite = m_normalSpriteKey;
		targetColor = m_normalColor;
		break;
	case ButtonVisualState::Hover:
		targetSprite = !m_hoverSpriteKey.empty() ? m_hoverSpriteKey : m_normalSpriteKey;
		targetColor = m_hoverColor;
		break;
	case ButtonVisualState::Pressed:
		targetSprite = !m_pressedSpriteKey.empty() ? m_pressedSpriteKey : m_normalSpriteKey;
		targetColor = m_pressedColor;
		break;
	case ButtonVisualState::Disabled:
		targetSprite = !m_disabledSpriteKey.empty() ? m_disabledSpriteKey : m_normalSpriteKey;
		targetColor = m_disabledColor;
		break;
	}

	if (m_pImgView != nullptr)
	{
		if (!targetSprite.empty())
		{
			m_pImgView->SetSpriteKey(targetSprite);
		}
		m_pImgView->SetColor(targetColor);
	}
	else if (m_pPanelView != nullptr)
	{
		if (!targetSprite.empty())
		{
			m_pPanelView->SetSpriteKey(targetSprite);
		}
		m_pPanelView->SetColor(targetColor);
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

	Vector2 pos = transform.GetWorldPosition();
	Vector2 scale = transform.GetWorldScale();
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