#pragma once

class GameObject;

class IDamageable
{
public:
	IDamageable() = default;
	virtual ~IDamageable() = default;

	virtual void TakeDamage(float damage, GameObject* pAttacker = nullptr) = 0;
	virtual bool IsDead() const = 0;
};
