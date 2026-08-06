#include "Engine/Core/pch.h"
#include "BoxCollider.h"
#include "Engine/Core/ComponentRegister.h"
#include "Engine/Renderer/RenderCommand.h"
#include "Engine/Renderer/RenderSystem.h"

#include "Engine/Framework/GameObject.h"
#include "Engine/Framework/Components/Render/SpriteRendererComponent.h"

static ComponentRegistrar<BoxCollider> registrar(EngineKey::Component::BoxCollider.data());

BoxCollider::BoxCollider(GameObject* owner, TransformComponent* transform) 
	: ColliderComponent(owner, transform)
{
    ExposeVariable("HalfWidth", &m_HalfWidth);
    ExposeVariable("HalfHeight", &m_HalfHeight);
}

void BoxCollider::Awake()
{
	ColliderComponent::Awake();

	if (m_HalfWidth <= 0.0f || m_HalfHeight <= 0.0f)
	{
		if (auto* spriteComp = gameObject.GetComponent<SpriteRendererComponent>())
		{
			Vector2 size = spriteComp->GetSpriteSize();
			if (size.x > 0.0f && size.y > 0.0f)
			{
				SetSize(size.x, size.y);
			}
		}
	}
}

void BoxCollider::PostDeserialize(Scene* pScene)
{
	ColliderComponent::PostDeserialize(pScene);

	if (m_HalfWidth <= 0.0f || m_HalfHeight <= 0.0f)
	{
		if (auto* spriteComp = gameObject.GetComponent<SpriteRendererComponent>())
		{
			Vector2 size = spriteComp->GetSpriteSize();
			if (size.x > 0.0f && size.y > 0.0f)
			{
				SetSize(size.x, size.y);
			}
		}
	}
}

void BoxCollider::DrawDebug()
{
    if (!b2Body_IsValid(m_BodyId)) return;

    b2Vec2 pos = b2Body_GetPosition(m_BodyId);
    b2Rot rot = b2Body_GetRotation(m_BodyId);

    RenderCommand cmd;
    cmd.type = RenderType::DEBUG_RECT;

    cmd.position = Vector2(MeterToPixel(pos.x), MeterToPixel(pos.y));
    cmd.rotation = RadianToDegree(b2Rot_GetAngle(rot));

    cmd.srcRect = { 0.0f, 0.0f, m_HalfWidth * 2.0f, m_HalfHeight * 2.0f };
    cmd.pivot = { 0.5f, 0.5f };

    cmd.shape.isFilled = false;
    cmd.color = D2D1::ColorF(D2D1::ColorF::LimeGreen); 
    cmd.zOrder = 9999; 

    RenderSystem::GetInstance()->SubmitCommand(cmd);
}