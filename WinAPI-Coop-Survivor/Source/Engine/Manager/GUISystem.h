#pragma once
#include "Engine/Core/Singleton.h"
#include "Engine/Framework/Base/ISystem.h"
#include "Engine/Framework/Base/IUpdatable.h"
#include "Engine/Framework/Base/IRenderable.h"

class IGUIPanel
{
public:
	virtual ~IGUIPanel() = default;
	virtual void OnDrawGUI() = 0;
};

class GUISystem : public Singleton<GUISystem>, public ISystem, public IUpdatable, public IRenderable
{
	friend class Singleton<GUISystem>;
public:
	virtual bool Initialize() override;
	virtual void Release() override;

	virtual void Update(float dt) override;
	virtual void Render() override;

	void RegisterPanel(IGUIPanel* pPanel);
	void UnRegisterPanel(IGUIPanel* pPanel);
private:
	GUISystem() = default;
	virtual ~GUISystem() = default;

private:
	std::vector<IGUIPanel*> m_vPanels;
	bool m_bInitialized = false;
	
};