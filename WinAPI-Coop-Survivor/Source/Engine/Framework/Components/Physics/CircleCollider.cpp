#include "Engine/Core/pch.h"
#include "CircleCollider.h"
#include "Engine/Core/ComponentRegister.h"
#include "Engine/Framework/GameObject.h"
#include "Engine/Framework/Components/Render/SpriteRendererComponent.h"
#include "Engine/Renderer/RenderCommand.h"
#include "Engine/Renderer/RenderSystem.h"

static ComponentRegistrar<CircleCollider> registrar(EngineKey::Component::CircleCollider.data());

CircleCollider::CircleCollider(GameObject* owner, TransformComponent* transform)
	: ColliderComponent(owner, transform)
{
	ExposeVariable("Radius", &m_Radius);
}

void CircleCollider::Awake()
{
	ColliderComponent::Awake();

	if (m_Radius <= 0.0f)
	{
		if (auto* spriteComp = gameObject.GetComponent<SpriteRendererComponent>())
		{
			Vector2 size = spriteComp->GetSpriteSize();
			float maxDim = (std::max)(size.x, size.y);
			if (maxDim > 0.0f)
			{
				SetRadius(maxDim * 0.5f);
			}
		}
	}
}

void CircleCollider::PostDeserialize(Scene* pScene)
{
	ColliderComponent::PostDeserialize(pScene);

	if (m_Radius <= 0.0f)
	{
		if (auto* spriteComp = gameObject.GetComponent<SpriteRendererComponent>())
		{
			Vector2 size = spriteComp->GetSpriteSize();
			float maxDim = (std::max)(size.x, size.y);
			if (maxDim > 0.0f)
			{
				SetRadius(maxDim * 0.5f);
			}
		}
	}
}

void CircleCollider::DrawDebug()
{
	if (!b2Body_IsValid(m_BodyId)) return;

	b2Vec2 pos = b2Body_GetPosition(m_BodyId);

	RenderCommand cmd;
	cmd.type = RenderType::DEBUG_CIRCLE;
	cmd.position = Vector2(MeterToPixel(pos.x), MeterToPixel(pos.y));
	cmd.srcRect = { m_Radius, 0.0f, 0.0f, 0.0f };
	cmd.pivot = { 0.5f, 0.5f };
	cmd.shape.isFilled = false;
	cmd.color = D2D1::ColorF(D2D1::ColorF::LimeGreen);
	cmd.zOrder = 9999;

	RenderSystem::GetInstance()->SubmitCommand(cmd);
}