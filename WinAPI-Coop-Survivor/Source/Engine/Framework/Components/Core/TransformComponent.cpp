#include "Engine/Core/pch.h"
#include "TransformComponent.h"
#include "Engine/Framework/GameObject.h"

TransformComponent::TransformComponent(GameObject* owner)
	: Component(owner)
{
	m_worldMatrix = D2D1::Matrix3x2F::Identity();

	ExposeVariable("Position", &m_localPosition);
	ExposeVariable("Rotation", &m_localRotation);
	ExposeVariable("Scale",    &m_localScale);
}

void TransformComponent::SetSiblingIndex(int index)
{
	gameObject.SetSiblingIndex(index);
}

int TransformComponent::GetSiblingIndex() const
{
	return gameObject.GetSiblingIndex();
}

void TransformComponent::SetAsFirstSibling()
{
	gameObject.SetAsFirstSibling();
}

void TransformComponent::SetAsLastSibling()
{
	gameObject.SetAsLastSibling();
}

void TransformComponent::SetDirty()
{
	if (m_bIsDirty) return;
	m_bIsDirty = true;
	for (TransformComponent* child : m_vChildren)
	{
		if (child) child->SetDirty();
	}
}

void TransformComponent::AttachToParent(TransformComponent* pNewParent)
{
	DetachFromParent();
	m_pParent = pNewParent;
	if (m_pParent)
	{
		m_pParent->m_vChildren.push_back(this);
	}
	SetDirty();
}

void TransformComponent::DetachFromParent()
{
	if (m_pParent == nullptr) return;
	auto& siblings = m_pParent->m_vChildren;
	siblings.erase(std::remove(siblings.begin(), siblings.end(), this), siblings.end());
	m_pParent = nullptr;
	SetDirty();
}

D2D1_MATRIX_3X2_F TransformComponent::MakeLocalMatrix(Vector2 pos, float rotDeg, Vector2 scale)
{
	float rad = rotDeg * (PI / 180.0f);
	D2D1_MATRIX_3X2_F S = D2D1::Matrix3x2F::Scale(scale.x, scale.y);
	D2D1_MATRIX_3X2_F R = D2D1::Matrix3x2F::Rotation(rotDeg);
	D2D1_MATRIX_3X2_F T = D2D1::Matrix3x2F::Translation(pos.x, pos.y);
	return S * R * T;
}

D2D1_MATRIX_3X2_F TransformComponent::InvertMatrix(const D2D1_MATRIX_3X2_F& m)
{
	float det = m._11 * m._22 - m._12 * m._21;
	if (std::abs(det) < 1e-8f) return D2D1::Matrix3x2F::Identity();

	float invDet = 1.0f / det;
	D2D1_MATRIX_3X2_F inv;
	inv._11 =  m._22 * invDet;
	inv._12 = -m._12 * invDet;
	inv._21 = -m._21 * invDet;
	inv._22 =  m._11 * invDet;
	inv._31 = -(m._31 * inv._11 + m._32 * inv._21);
	inv._32 = -(m._31 * inv._12 + m._32 * inv._22);
	return inv;
}

void TransformComponent::UpdateWorldMatrix()
{
	if (!m_bIsDirty) return;

	D2D1_MATRIX_3X2_F localMat = MakeLocalMatrix(m_localPosition, m_localRotation, m_localScale);

	if (m_pParent != nullptr)
	{
		D2D1_MATRIX_3X2_F parentWorld = m_pParent->GetWorldMatrixRaw();
		m_worldMatrix = localMat * parentWorld;
	}
	else
	{
		m_worldMatrix = localMat;
	}

	m_worldPosition = Vector2(m_worldMatrix._31, m_worldMatrix._32);

	float scaleX = std::sqrt(m_worldMatrix._11 * m_worldMatrix._11 + m_worldMatrix._12 * m_worldMatrix._12);
	float scaleY = std::sqrt(m_worldMatrix._21 * m_worldMatrix._21 + m_worldMatrix._22 * m_worldMatrix._22);
	m_worldScale  = Vector2(scaleX, scaleY > 1e-8f ? scaleY : 1.0f);

	if (scaleX > 1e-8f)
	{
		m_worldRotation = std::atan2(m_worldMatrix._12, m_worldMatrix._11) * (180.0f / PI);
	}
	else
	{
		m_worldRotation = 0.0f;
	}

	m_bIsDirty = false;
}

D2D1_MATRIX_3X2_F TransformComponent::GetWorldMatrixRaw()
{
	UpdateWorldMatrix();
	return m_worldMatrix;
}

Vector2 TransformComponent::GetWorldPosition()
{
	UpdateWorldMatrix();
	return m_worldPosition;
}

float TransformComponent::GetWorldRotation()
{
	UpdateWorldMatrix();
	return m_worldRotation;
}

Vector2 TransformComponent::GetWorldScale()
{
	UpdateWorldMatrix();
	return m_worldScale;
}

Vector2 TransformComponent::WorldToLocal(const Vector2& worldPos)
{
	if (m_pParent == nullptr) return worldPos;

	D2D1_MATRIX_3X2_F parentWorld = m_pParent->GetWorldMatrixRaw();
	D2D1_MATRIX_3X2_F inv = InvertMatrix(parentWorld);

	float lx = worldPos.x * inv._11 + worldPos.y * inv._21 + inv._31;
	float ly = worldPos.x * inv._12 + worldPos.y * inv._22 + inv._32;
	return Vector2(lx, ly);
}