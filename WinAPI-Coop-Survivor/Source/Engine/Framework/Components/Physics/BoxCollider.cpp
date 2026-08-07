#include "Engine/Core/pch.h"
#include "BoxCollider.h"
#include "Engine/Core/ComponentRegister.h"
#include "Engine/Renderer/RenderCommand.h"
#include "Engine/Renderer/RenderSystem.h"
#include "Engine/Framework/Components/Render/SpriteRendererComponent.h"
#include "Engine/Framework/Components/UI/UIImageComponent.h"

static ComponentRegistrar<BoxCollider> registrar(EngineKey::Component::BoxCollider.data());

BoxCollider::BoxCollider(GameObject* owner, TransformComponent* transform) 
	: ColliderComponent(owner, transform)
{
    ExposeVariable("Size", &m_size);
}

void BoxCollider::Awake()
{
	ColliderComponent::Awake();
	if (m_size.x <= 0.0f || m_size.y <= 0.0f)
	{
		if (auto* spriteRenderer = gameObject.GetComponent<SpriteRendererComponent>())
		{
		m_size = spriteRenderer->GetSize();
		}
		else if (auto* uiImage = gameObject.GetComponent<UIImageComponent>())
		{
		m_size = uiImage->GetSize();
		}
		if (m_size.x <= 0.0f || m_size.y <= 0.0f)
		{
			m_size = { 100.0f, 100.0f };
		}
	}

	RebuildShape();
}

void BoxCollider::PostDeserialize(Scene* pScene)
{
	ColliderComponent::PostDeserialize(pScene);
	RebuildShape();
}

void BoxCollider::DrawDebug()
{
    if (!b2Body_IsValid(m_BodyId)) return;

    b2Vec2 pos = b2Body_GetPosition(m_BodyId);
    b2Rot rot = b2Body_GetRotation(m_BodyId);

    b2Vec2 localOffset = { PixelToMeter(m_offset.x), PixelToMeter(m_offset.y) };
    b2Vec2 rotatedOffset = b2RotateVector(rot, localOffset);

    RenderCommand cmd;
    cmd.type = RenderType::DEBUG_RECT;

    cmd.position = Vector2(MeterToPixel(pos.x + rotatedOffset.x), MeterToPixel(pos.y + rotatedOffset.y));
    cmd.rotation = RadianToDegree(b2Rot_GetAngle(rot));

    cmd.srcRect = { 0.0f, 0.0f, m_size.x, m_size.y };
    cmd.pivot = { 0.5f, 0.5f };

    cmd.shape.isFilled = false;
    cmd.color = D2D1::ColorF(D2D1::ColorF::LimeGreen); 
    cmd.zOrder = 9999; 

    RenderSystem::GetInstance()->SubmitCommand(cmd);
}