#pragma once
#include "Engine/Core/Singleton.h"

class ISystem;
class IUpdatable;
class IRenderable;

enum class EnginePlayState
{
	Edit,
	Play,
	Puase,
};

class EngineKernel : public Singleton<EngineKernel>
{
	friend class Singleton<EngineKernel>;

public:
	bool Initialize();
	void ProcessFrame();
	void Release();

	void RegisterManager(ISystem* manager);

	void SetPlayState(EnginePlayState state) { m_PlayState = state; }
	EnginePlayState GetPlayState() const { return m_PlayState; }

private:
	EngineKernel() = default;
	virtual ~EngineKernel() = default;

private:
	std::vector<ISystem*> m_vAllSystems;
	std::vector<IUpdatable*> m_vUpdatableSystems;
	std::vector<IRenderable*> m_vRenderableSystems;

private:
#if WITH_EDITOR
	EnginePlayState m_PlayState = EnginePlayState::Edit;
#else
	EnginePlayState m_PlayState = EnginePlayState::Play;
#endif
};

