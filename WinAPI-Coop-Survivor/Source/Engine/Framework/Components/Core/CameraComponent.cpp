#include "Engine/Core/pch.h"
#include "CameraComponent.h"
#include "Engine/Core/ComponentRegister.h"
#include "Engine/Framework/GameObject.h"
#include "Engine/Framework/Components/Core/TransformComponent.h"
#include "Engine/Manager/CameraManager.h"
#include "Engine/Renderer/GraphicManager.h"

static ComponentRegistrar<CameraComponent> registrar(EngineKey::Component::Camera.data());

CameraComponent::CameraComponent(GameObject* owner, TransformComponent* transform) : ScriptComponent(owner, transform)
{
	ExposeVariable("Zoom", &m_zoom);
	ExposeVariable("Smooth Follow", &m_bSmoothFollow);
	ExposeVariable("Follow Speed", &m_followSpeed);
	ExposeVariable("Use Map Bounds", &m_bUseMapBounds);
}

CameraComponent::~CameraComponent()
{
	if (CameraManager::GetInstance()->GetMainCamera() == this)
	{
		CameraManager::GetInstance()->SetMainCamera(nullptr);
	}
}

void CameraComponent::Awake()
{
	CameraManager::GetInstance()->SetMainCamera(this);

	m_currentPos = transform.GetPosition();
}

void CameraComponent::FixedUpdate(float fixedDt)
{
	if (m_updateMode == CameraUpdateMode::FixedUpdate)
	{
		InternalUpdateCamera(fixedDt);
	}
}

void CameraComponent::Update(float dt)
{
	if (m_shakeTimer > 0.0f)
	{
		m_shakeTimer -= dt;
		float randomX = ((rand() % 200) - 100) / 100.0f * m_shakeIntensity;
		float randomY = ((rand() % 200) - 100) / 100.0f * m_shakeIntensity;
		m_shakeOffset = D2D1::Point2F(randomX, randomY);
	}
	else
	{
		m_shakeOffset = D2D1::Point2F(0.0f, 0.0f);
	}

	if (m_updateMode == CameraUpdateMode::Update)
	{
		InternalUpdateCamera(dt);
	}
}

void CameraComponent::LateUpdate(float dt)
{
	if (m_updateMode == CameraUpdateMode::LateUpdate)
	{
		InternalUpdateCamera(dt);
	}
}

void CameraComponent::InternalUpdateCamera(float dt)
{
	D2D1_POINT_2F targetPos = m_pTargetTransform ? m_pTargetTransform->GetPosition() : transform.GetPosition();

	targetPos = (Vector2)targetPos + (Vector2)m_offset;

	if (m_bSmoothFollow)
	{
		float lerpFactor = 1.0f - std::exp(-m_followSpeed * dt);
		m_currentPos.x += (targetPos.x - m_currentPos.x) * lerpFactor;
		m_currentPos.y += (targetPos.y - m_currentPos.y) * lerpFactor;
	}
	else
	{
		m_currentPos = targetPos;
	}

	if (m_bUseMapBounds)
	{
		m_currentPos.x = std::clamp(m_currentPos.x, m_mapBounds.left, m_mapBounds.right);
		m_currentPos.y = std::clamp(m_currentPos.y, m_mapBounds.top, m_mapBounds.bottom);
	}
}

D2D1_MATRIX_3X2_F CameraComponent::GetViewMatrix() const
{
	D2D1_POINT_2F finalPos = (Vector2)m_currentPos + (Vector2)m_shakeOffset;

	finalPos.x = std::floorf(finalPos.x);
	finalPos.y = std::floorf(finalPos.y);

	float screenWidth = GraphicManager::GetInstance()->GetScreenWidth();
	float screenHeight = GraphicManager::GetInstance()->GetScreenHeight();
	D2D1_POINT_2F screenCenter = D2D1::Point2F(screenWidth * 0.5f, screenHeight * 0.5f);

	D2D1_MATRIX_3X2_F matTranslation = D2D1::Matrix3x2F::Translation(-finalPos.x, -finalPos.y);
	D2D1_MATRIX_3X2_F matScale = D2D1::Matrix3x2F::Scale(m_zoom, m_zoom, D2D1::Point2F(0.0f, 0.0f));
	D2D1_MATRIX_3X2_F matCenter = D2D1::Matrix3x2F::Translation(screenCenter.x, screenCenter.y);

	return matTranslation * matScale * matCenter;
}

void CameraComponent::TriggerShake(float intensity, float duration)
{
	m_shakeIntensity = intensity;
	m_shakeTimer = duration;
}

void CameraComponent::SetTarget(GameObject* pTargetGameObject)
{
	if (pTargetGameObject)
	{
		SetTarget(&pTargetGameObject->transform);
	}
	else
	{
		m_pTargetTransform = nullptr;
	}
}

D2D1_POINT_2F CameraComponent::ScreenToWorldPoint(D2D1_POINT_2F screenPoint) const
{
	D2D1_MATRIX_3X2_F viewMat = GetViewMatrix();
	D2D1_MATRIX_3X2_F invViewMat;

	if (D2D1InvertMatrix(&invViewMat))
	{
		return D2D1::Matrix3x2F::ReinterpretBaseType(&invViewMat)->TransformPoint(screenPoint);
	}
	return screenPoint;
}

D2D1_POINT_2F CameraComponent::WorldToScreenPoint(D2D1_POINT_2F worldPoint) const
{
	D2D1_MATRIX_3X2_F viewMat = GetViewMatrix();
	D2D1_POINT_2F screenPt = D2D1::Matrix3x2F::ReinterpretBaseType(&viewMat)->TransformPoint(D2D1::Point2F(worldPoint.x, worldPoint.y));

	return screenPt;
}
