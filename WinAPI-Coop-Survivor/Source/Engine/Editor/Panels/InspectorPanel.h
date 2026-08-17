#pragma once
#include "Engine/Manager/GUISystem.h"

class GameObject;

class InspectorPanel : public IGUIPanel
{
public:
	InspectorPanel() = default;
	virtual ~InspectorPanel() = default;

	void Initialize();
	void Release();

	virtual void OnDrawGUI() override;

private:
	void DrawHeader(GameObject* pObj);
	void DrawTransform(GameObject* pObj);
	void DrawComponents(GameObject* pObj);
	void DrawAddComponentButton(GameObject* pObj);
	void DrawScriptableObjectData();
};