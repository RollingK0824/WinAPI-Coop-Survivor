#include "Engine/Core/pch.h"
#include "UITextComponent.h"
#include "Engine/Core/ComponentRegister.h"
#include "Engine/Renderer/RenderSystem.h"
#include "Engine/Renderer/RenderCommand.h"
#include "Engine/Framework/GameObject.h"
#include "Engine/Framework/Components/Core/TransformComponent.h"

static ComponentRegistrar<UITextComponent> registrar(EngineKey::Component::UITextComponent.data());

UITextComponent::UITextComponent(GameObject* owner, TransformComponent* transform) : Component(owner,transform)
{
}

void UITextComponent::RenderUI()
{
	if (!m_bIsEnabled || !gameObject.IsActive() || m_text.empty()) return;
	RenderCommand cmd;
	cmd.type = RenderType::Debug_TEXT;
	cmd.isUI = true; // Screen Space 고정
	cmd.position = transform.GetPosition() + m_position;
	cmd.text.fontSize = m_fontSize;
	cmd.color = m_color;
	cmd.text.pText = m_text;
	cmd.zOrder = 99999; // 최상단 Layer
	RenderSystem::GetInstance()->SubmitCommand(cmd);
}
