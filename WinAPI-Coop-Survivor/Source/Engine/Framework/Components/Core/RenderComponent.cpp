#include "Engine/Core/pch.h"
#include "RenderComponent.h"
#include "Engine/Core/ComponentRegister.h"
#include "Engine/Manager/ResourceManager.h"
#include "Engine/Renderer/RenderCommand.h"
#include "Engine/Framework/Scene.h"
#include "Engine/Framework/GameObject.h"

static ComponentRegistrar<RenderComponent> registrar(EngineKey::Component::Render.data());

RenderComponent::RenderComponent(GameObject* owner, TransformComponent* transform)
	: Component(owner, transform) 
{
	ExposeVariable("RenderType", reinterpret_cast<int*>(&m_RenderCommand.type));
	ExposeVariable("ZOrder", &m_RenderCommand.zOrder);
	ExposeVariable("Opacity", &m_RenderCommand.bitmap.opacity);
	ExposeVariable("Color", &m_RenderCommand.color);
}
