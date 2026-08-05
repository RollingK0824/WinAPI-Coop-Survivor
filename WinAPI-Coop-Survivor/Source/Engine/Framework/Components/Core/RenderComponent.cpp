#include "Engine/Core/pch.h"
#include "RenderComponent.h"
#include "Engine/Core/ComponentRegister.h"
#include "Engine/Manager/ResourceManager.h"
#include "Engine/Renderer/RenderCommand.h"
#include "Engine/Framework/Scene.h"
#include "Engine/Framework/GameObject.h"

static ComponentRegistrar<RenderComponent> registrar(EngineKey::Component::Render.data());

RenderComponent::RenderComponent(GameObject* owner, TransformComponent* transform) : Component(owner, transform) 
{
	ExposeVariable("Z-Order", &m_RenderCommand.zOrder);
	ExposeVariable("Opacity", &m_RenderCommand.bitmap.opacity);
	ExposeVariable("Color", &m_RenderCommand.color);
}

void RenderComponent::Serialize(json& outJson) const
{
	Component::Serialize(outJson);

	outJson[EngineKey::Property::RenderType.data()] = static_cast<int>(m_RenderCommand.type);
	outJson[EngineKey::Property::ZOrder.data()] = m_RenderCommand.zOrder;
	outJson["Opacity"] = m_RenderCommand.bitmap.opacity;
	outJson["Color"] = {
		{"r", m_RenderCommand.color.r},
		{"g", m_RenderCommand.color.g},
		{"b", m_RenderCommand.color.b},
		{"a", m_RenderCommand.color.a}
	};
}

void RenderComponent::Deserialize(const json& inJson)
{
	Component::Deserialize(inJson);
	if (inJson.contains(EngineKey::Property::RenderType.data()))
	{
		m_RenderCommand.type = static_cast<RenderType>(inJson[EngineKey::Property::RenderType.data()].get<int>());
	}
	if (inJson.contains(EngineKey::Property::ZOrder.data()))
	{
		m_RenderCommand.zOrder = inJson[EngineKey::Property::ZOrder.data()].get<int>();
	}
	if (inJson.contains("Opacity"))
	{
		m_RenderCommand.bitmap.opacity = inJson["Opacity"].get<float>();
	}
	if (inJson.contains("Color"))
	{
		const auto& col = inJson["Color"];
		m_RenderCommand.color = D2D1::ColorF(
			col["r"].get<float>(),
			col["g"].get<float>(),
			col["b"].get<float>(),
			col["a"].get<float>()
		);
	}
}
