#include "Engine/Core/pch.h"
#include "EngineKernel.h"
#include "Engine/Manager/TimeManager.h"
#include "Engine/Framework/base/ISystem.h"
#include "Engine/Framework/base/IUpdatable.h"
#include "Engine/Framework/base/IRenderable.h"

bool EngineKernel::Initialize()
{
	for (const auto& it : m_vAllSystems)
	{
		if (!it->Initialize())
		{
			return false;
		}
	}
	return true;
}

void EngineKernel::Release()
{
	for (auto it = m_vAllSystems.rbegin(); it != m_vAllSystems.rend(); ++it)
	{
		(*it)->Release();
	}

	m_vAllSystems.clear();
	m_vUpdatableSystems.clear();
	m_vRenderableSystems.clear();

	// 임시 vector를 만들어 swap 함수 종료 시 완전 삭제
	std::vector<ISystem*>().swap(m_vAllSystems);
	std::vector<IUpdatable*>().swap(m_vUpdatableSystems);
	std::vector<IRenderable*>().swap(m_vRenderableSystems);
}

void EngineKernel::ProcessFrame()
{
	TimeManager* pTime = TimeManager::GetInstance();

	pTime->Update(0.0f);

	float dt = pTime->GetDeltaTime();
	float fixedDt = pTime->GetFixedDeltaTime();

	if (m_PlayState == EnginePlayState::Play)
	{
		while (pTime->AccumulateTime())
		{
			for (auto* sys : m_vUpdatableSystems) sys->FixedUpdate(fixedDt);

			pTime->ConsumeFixedTick();
		}
	}

	for (auto* sys : m_vUpdatableSystems)sys->Update(dt);
	for (auto* sys : m_vUpdatableSystems)sys->LateUpdate(dt);


	for (auto* sys : m_vRenderableSystems)sys->PreRender();
	for (auto* sys : m_vRenderableSystems)sys->Render();
	for (auto* sys : m_vRenderableSystems)sys->PostRender();

	for (auto* sys : m_vAllSystems)sys->PostFrame();
}

void EngineKernel::RegisterManager(ISystem* manager)
{
	if (manager == nullptr)return;
	m_vAllSystems.push_back(manager);

	if (auto* updatable = dynamic_cast<IUpdatable*>(manager))
	{
		m_vUpdatableSystems.push_back(updatable);
	}

	if (auto* renderable = dynamic_cast<IRenderable*>(manager))
	{
		m_vRenderableSystems.push_back(renderable);
	}
}