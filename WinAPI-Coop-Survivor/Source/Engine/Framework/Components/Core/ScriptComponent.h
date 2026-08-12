#pragma once
#include "Engine/Framework/Base/Component.h"
#include "Engine/Framework/Base/IUpdatable.h"
#include "Engine/Core/Define.h"

class ScriptComponent : public Component, public IUpdatable
{
public:
	ScriptComponent(GameObject* owner, TransformComponent* transform);
	virtual ~ScriptComponent()override = default;

	virtual std::string_view GetComponentType() const
	{
		return EngineKey::Component::Script;
	}

	virtual void FixedUpdate(float fixedDt)override {};
	virtual void Update(float dt)override {};
	virtual void LateUpdate(float dt)override {};
};