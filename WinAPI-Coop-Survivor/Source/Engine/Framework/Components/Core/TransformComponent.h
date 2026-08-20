#pragma once
#include "Engine/Framework/Base/Component.h"

class TransformComponent : public Component
{
public:
	CLONEABLE_COMPONENT(TransformComponent)

	TransformComponent(GameObject* owner);
	virtual ~TransformComponent() override = default;

	virtual std::string_view GetComponentType() const override
	{
		return EngineKey::Component::Trnasform;
	}

	void SetSiblingIndex(int index);
	int GetSiblingIndex() const;
	void SetAsFirstSibling();
	void SetAsLastSibling();

	// ---- Local Transform ----
	const Vector2& GetLocalPosition() const { return m_localPosition; }
	float          GetLocalRotation() const  { return m_localRotation; }
	const Vector2& GetLocalScale()    const  { return m_localScale; }

	void SetLocalPosition(float x, float y)  { m_localPosition.x = x; m_localPosition.y = y; SetDirty(); }
	void SetLocalPosition(Vector2 pos)        { m_localPosition = pos; SetDirty(); }
	void SetLocalRotation(float angle)        { m_localRotation = angle; SetDirty(); }
	void SetLocalScale(float sx, float sy)    { m_localScale.x = sx; m_localScale.y = sy; SetDirty(); }
	void SetLocalScale(Vector2 scale)         { m_localScale = scale; SetDirty(); }

	const Vector2& GetPosition() const { return m_localPosition; }
	void SetPosition(float x, float y) { SetLocalPosition(x, y); }
	void SetPosition(Vector2 pos)       { SetLocalPosition(pos); }

	const Rotation& GetRotation() const { m_rotCompat.angle = m_localRotation; return m_rotCompat; }
	void SetRotation(float angle)       { SetLocalRotation(angle); }
	void SetRotation(Rotation rot)      { SetLocalRotation(rot.angle); }

	const Vector2& GetScale() const { return m_localScale; }
	void SetScale(float sx, float sy) { SetLocalScale(sx, sy); }
	void SetScale(Vector2 scale)      { SetLocalScale(scale); }

	// ---- World Transform ----
	Vector2 GetWorldPosition();
	float   GetWorldRotation();
	Vector2 GetWorldScale();

	void AttachToParent(TransformComponent* pParent);
	void DetachFromParent();
	TransformComponent* GetParentTransform() const { return m_pParent; }

	void SetDirty();

	Vector2 WorldToLocal(const Vector2& worldPos);

	D2D1_MATRIX_3X2_F GetWorldMatrixRaw();

private:
	void UpdateWorldMatrix();

	static D2D1_MATRIX_3X2_F MakeLocalMatrix(Vector2 pos, float rotDeg, Vector2 scale);
	static D2D1_MATRIX_3X2_F InvertMatrix(const D2D1_MATRIX_3X2_F& m);

	TransformComponent* m_pParent = nullptr;
	std::vector<TransformComponent*> m_vChildren;

	// Local
	Vector2 m_localPosition = { 0.0f, 0.0f };
	float   m_localRotation = 0.0f;
	Vector2 m_localScale    = { 1.0f, 1.0f };

	mutable Rotation m_rotCompat = { 0.0f };

	bool   m_bIsDirty    = true;
	D2D1_MATRIX_3X2_F m_worldMatrix;
	Vector2 m_worldPosition = { 0.0f, 0.0f };
	float   m_worldRotation = 0.0f;
	Vector2 m_worldScale    = { 1.0f, 1.0f };
};
