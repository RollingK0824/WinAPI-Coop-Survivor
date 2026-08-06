#pragma once
#include "Engine/Core/Singleton.h"
#include "Engine/Framework/Base/ISystem.h"
#include "Engine/Framework/Base/IUpdatable.h"

class GameObject;
class ScriptableObject;
class MainMenuBarPanel;
class HierarchyPanel;
class InspectorPanel;
class ContentBrowserPanel;
class ViewportPanel;

class EditorSystem : public Singleton<EditorSystem>, public ISystem, public IUpdatable
{
	friend class Singleton<EditorSystem>;
public:
	virtual bool Initialize() override;
	virtual void Release() override;
	virtual void Update(float dt) override;

	void SetSelectedObject(GameObject* pObj)
	{
		m_pSelectedObject = pObj;
		if (pObj) m_pSelectedSO = nullptr;
	}
	GameObject* GetSelectedObject() const { return m_pSelectedObject; }

	void SetSelectedScriptableObject(ScriptableObject* pSO)
	{
		m_pSelectedSO = pSO;
		if (pSO) m_pSelectedObject = nullptr;
	}
	ScriptableObject* GetSelectedScriptableObject() const { return m_pSelectedSO; }

	ViewportPanel* GetViewportPanel() const { return m_pViewportPanel.get(); }

private:
	EditorSystem();
	virtual ~EditorSystem();

private:
	GameObject* m_pSelectedObject = nullptr;
	ScriptableObject* m_pSelectedSO = nullptr;
	// 분리된 패널 모듈 인스턴스 소유
	std::unique_ptr<MainMenuBarPanel> m_pMainMenuBarPanel;
	std::unique_ptr<HierarchyPanel> m_pHierarchyPanel;
	std::unique_ptr<InspectorPanel> m_pInspectorPanel;
	std::unique_ptr<ContentBrowserPanel> m_pContentBrowserPanel;
	std::unique_ptr<ViewportPanel> m_pViewportPanel;
};