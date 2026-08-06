#include "Engine/Core/pch.h"
#include "CircleCollider.h"
#include "Engine/Core/ComponentRegister.h"
#include "Engine/Framework/GameObject.h"
#include "Engine/Renderer/RenderCommand.h"
#include "Engine/Renderer/RenderSystem.h"
#include "Engine/Framework/Components/Render/SpriteRendererComponent.h"
#include "Engine/Framework/Components/UI/UIImageComponent.h"

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
		if (auto* spriteRenderer = gameObject.GetComponent<SpriteRendererComponent>())
		{
			Vector2 sz = spriteRenderer->GetSize();
			m_Radius = max(sz.x, sz.y) * 0.5f;
		}
		else if (auto* uiImage = gameObject.GetComponent<UIImageComponent>())
		{
			Vector2 sz = uiImage->GetSize();
			m_Radius = max(sz.x, sz.y) * 0.5f;
		}
		if (m_Radius <= 0.0f)
		{
			m_Radius = 50.0f;
		}
	}

	RebuildShape();
}

void CircleCollider::PostDeserialize(Scene* pScene)
{
	ColliderComponent::PostDeserialize(pScene);
	RebuildShape();
}

void CircleCollider::DrawDebug()
{
	if (!b2Body_IsValid(m_BodyId)) return;

	b2Vec2 pos = b2Body_GetPosition(m_BodyId);
	b2Rot rot = b2Body_GetRotation(m_BodyId);

	b2Vec2 localOffset = { PixelToMeter(m_offset.x), PixelToMeter(m_offset.y) };
	b2Vec2 rotatedOffset = b2RotateVector(rot, localOffset);

	RenderCommand cmd;
	cmd.type = RenderType::DEBUG_CIRCLE;
	cmd.position = Vector2(MeterToPixel(pos.x + rotatedOffset.x), MeterToPixel(pos.y + rotatedOffset.y));
	cmd.srcRect = { m_Radius, 0.0f, 0.0f, 0.0f };
	cmd.pivot = { 0.5f, 0.5f };
	cmd.shape.isFilled = false;
	cmd.color = D2D1::ColorF(D2D1::ColorF::LimeGreen);
	cmd.zOrder = 9999;

	RenderSystem::GetInstance()->SubmitCommand(cmd);
}