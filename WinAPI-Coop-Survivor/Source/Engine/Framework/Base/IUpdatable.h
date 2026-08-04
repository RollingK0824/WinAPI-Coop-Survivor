#pragma once

class IUpdatable
{
public:
	IUpdatable() = default;
	virtual ~IUpdatable() = default;

	virtual void FixedUpdate(float fixedDt) {}
	virtual void Update(float dt) {}
	virtual void LateUpdate(float dt) {}
};