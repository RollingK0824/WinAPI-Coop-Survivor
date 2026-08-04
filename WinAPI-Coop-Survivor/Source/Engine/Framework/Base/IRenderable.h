#pragma once

class IRenderable
{
public:
	IRenderable() = default;
	virtual ~IRenderable() = default;

	virtual void PreRender() {}
	virtual void Render() {}
	virtual void PostRender() {}
};