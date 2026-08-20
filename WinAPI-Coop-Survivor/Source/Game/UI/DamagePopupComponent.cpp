#include "Engine/Core/pch.h"
#include "DamagePopupComponent.h"
#include "Engine/Core/ComponentRegister.h"
#include "Engine/Core/ObjectPool.h"
#include "Engine/Framework/GameObject.h"
#include "Engine/Framework/Components/Core/TransformComponent.h"
#include "Engine/Framework/Components/UI/UITextComponent.h"
#include "Engine/Manager/RandomManager.h"

static ComponentRegistrar<DamagePopupComponent> registrar(EngineKey::CustomComponent::DamagePopup.data());
static ComponentRegistrar<DamagePopupComponent> registrarAlias("DamagePopupComponent");

DamagePopupComponent::DamagePopupComponent(GameObject* owner, TransformComponent* transform)
	: ScriptComponent(owner, transform)
{
}

void DamagePopupComponent::Awake()
{
	m_pText = gameObject.GetComponent<UITextComponent>();
	if (!m_pText.IsValid())
	{
		m_pText = gameObject.AddComponent<UITextComponent>();
	}
}

void DamagePopupComponent::Init(int damage, const Vector2& spawnPos, bool isCritical, const std::string& poolKey)
{
	m_poolKey = poolKey;
	m_lifeTimer = 0.0f;
	m_duration = isCritical ? 0.85f : 0.65f;

	// Random jitter for spawn position around monster head/body
	float offsetRange = 10.0f;
	float randX = RandomManager::GetInstance()->GetLocalRandomFloat(-offsetRange, offsetRange);
	float randY = RandomManager::GetInstance()->GetLocalRandomFloat(-offsetRange, offsetRange);
	Vector2 actualSpawnPos = spawnPos + Vector2(randX, randY - 15.0f);

	transform.SetPosition(actualSpawnPos.x, actualSpawnPos.y);

	// Velocity: initial upward burst & gentle horizontal drift
	float vx = RandomManager::GetInstance()->GetLocalRandomFloat(-20.0f, 20.0f);
	float vy = isCritical ? -75.0f : -55.0f;
	m_velocity = { vx, vy };

	m_swayFreq = RandomManager::GetInstance()->GetLocalRandomFloat(8.0f, 14.0f);
	m_swayAmp = RandomManager::GetInstance()->GetLocalRandomFloat(10.0f, 20.0f);

	if (isCritical)
	{
		m_baseColor = D2D1::ColorF(1.0f, 0.45f, 0.1f, 1.0f); // Bright Orange-Red for crit
	}
	else
	{
		m_baseColor = D2D1::ColorF(1.0f, 1.0f, 1.0f, 1.0f); // Clean White for normal hit
	}

	if (!m_pText.IsValid())
	{
		m_pText = gameObject.GetComponent<UITextComponent>();
		if (!m_pText.IsValid())
		{
			m_pText = gameObject.AddComponent<UITextComponent>();
		}
	}

	if (m_pText.IsValid())
	{
		m_pText->SetIsUI(false); // World space rendering
		m_pText->SetText(std::to_wstring(damage));
		m_pText->SetFontSize(isCritical ? 17.0f : 13.0f);
		m_pText->SetColor(m_baseColor);
		m_pText->SetAlignment(ETextAlignment::Center);
		m_pText->SetParagraphAlignment(EParagraphAlignment::Center);
		m_pText->SetSize(80.0f, 30.0f);
		m_pText->SetPivot(0.5f, 0.5f);
		m_pText->SetZOrder(9500);
	}
}

void DamagePopupComponent::Update(float dt)
{
	if (!gameObject.IsActive()) return;

	m_lifeTimer += dt;
	if (m_lifeTimer >= m_duration)
	{
		PoolManager::GetInstance()->Despawn<GameObject>(m_poolKey, &gameObject);
		return;
	}

	float t = m_lifeTimer / m_duration; // 0.0 -> 1.0

	// Movement: Upward float with horizontal sway
	float sway = sinf(m_lifeTimer * m_swayFreq) * m_swayAmp * dt;
	Vector2 currentPos = transform.GetPosition();
	currentPos.x += (m_velocity.x * dt) + sway;
	currentPos.y += m_velocity.y * dt;
	transform.SetPosition(currentPos.x, currentPos.y);

	// Decelerate upward velocity slightly
	m_velocity.y *= (1.0f - dt * 0.8f);

	// Alpha fade out (starts solid, fades in last 50%)
	float alpha = 1.0f;
	if (t > 0.4f)
	{
		alpha = (std::max)(0.0f, 1.0f - (t - 0.4f) / 0.6f);
	}

	if (m_pText.IsValid())
	{
		D2D1::ColorF c(m_baseColor.r, m_baseColor.g, m_baseColor.b, alpha);
		m_pText->SetColor(c);
	}
}
