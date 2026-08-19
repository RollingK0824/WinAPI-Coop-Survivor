#pragma once
#include "Engine/Manager/GUISystem.h"

class GameObject;
class ScriptableObject;

class InspectorPanel : public IGUIPanel
{
public:
	InspectorPanel() = default;
	virtual ~InspectorPanel() = default;

	void Initialize();
	void Release();

	virtual void OnDrawGUI() override;

	bool IsLocked() const { return m_bIsLocked; }
	void SetLocked(bool locked) { m_bIsLocked = locked; }

private:
	void DrawHeader(GameObject* pObj);
	void DrawTransform(GameObject* pObj);
	void DrawComponents(GameObject* pObj);
	void DrawAddComponentButton(GameObject* pObj);
	void DrawScriptableObjectData();

private:
	bool m_bIsLocked = false;
	GameObject* m_pLockedObject = nullptr;
	ScriptableObject* m_pLockedSO = nullptr;
};