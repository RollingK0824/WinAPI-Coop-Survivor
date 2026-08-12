#pragma once

class ISystem
{
public:
	ISystem() = default;
	virtual ~ISystem() = default;

	virtual bool Initialize() = 0;
	virtual void Release() = 0;

	virtual void PostFrame() {}
};
