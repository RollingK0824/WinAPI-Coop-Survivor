#pragma once
#include "Engine/Framework/Components/Core/ScriptComponent.h"

enum class CameraUpdateMode
{
	FixedUpdate,
	Update,
	LateUpdate
};

class CameraComponent : public ScriptComponent
{
public:
	CLONEABLE_COMPONENT(CameraComponent)

		CameraComponent(GameObject* owner, TransformComponent* transform);
	virtual ~CameraComponent()override;

	virtual void Awake() override;
	virtual void FixedUpdate(float fixedDt)override;
	virtual void Update(float dt)override;
	virtual void LateUpdate(float dt)override;

	void SetUpdateMode(CameraUpdateMode mode) { m_updateMode = mode; }
	CameraUpdateMode GetUpdateM() const { return m_updateMode; }

	void SetTarget(TransformComponent* pTarget) { m_pTargetTransform = pTarget; }
	void SetTarget(GameObject* pTargetGameObject);
	TransformComponent* GetTarget() const { return m_pTargetTransform; }

	D2D1_MATRIX_3X2_F GetViewMatrix() const;
	D2D1_POINT_2F ScreenToWorldPoint(D2D1_POINT_2F screenPoint) const;
	D2D1_POINT_2F WorldToScreenPoint(D2D1_POINT_2F worldPoint)const;

	void SetZoom(float zoom) { m_zoom = zoom; }
	float GetZoom() const { return m_zoom; }
	void SetOffset(Vector2 offset) { m_offset = offset; }
	void SetSmoothFollow(bool enable, float speed = 5.0f)
	{
		m_bSmoothFollow = enable;
		m_followSpeed = speed;
	}
	void TriggerShake(float intensity, float duration);
	void SetMapBounds(D2D1_RECT_F bounds, bool useBounds = true)
	{
		m_mapBounds = bounds;
		m_bUseMapBounds = useBounds;
	}

	virtual std::string_view GetComponentType() const
	{
		return EngineKey::Component::Camera;
	}

private:
	void InternalUpdateCamera(float dt);

private:
	TransformComponent* m_pTargetTransform = nullptr;

	CameraUpdateMode m_updateMode = CameraUpdateMode::FixedUpdate;

	float m_zoom = 1.0f;
	D2D1_POINT_2F m_currentPos = { 0.0f,0.0f };
	D2D1_POINT_2F m_offset = { 0.0f,0.0f };

	bool m_bSmoothFollow = true;
	float m_followSpeed = 5.0f;

	float m_shakeIntensity = 0.0f;
	float m_shakeTimer = 0.0f;
	D2D1_POINT_2F m_shakeOffset{ 0.0f,0.0f };

	bool m_bUseMapBounds = false;
	D2D1_RECT_F m_mapBounds = { 0.0f, 0.0f, 0.0f, 0.0f };
};