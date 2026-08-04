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
    void DrawHeader(GameObject* pObj);             // 이름, Active 체크박스
    void DrawTransform(GameObject* pObj);          // TransformComponent 전용 UI
    void DrawComponents(GameObject* pObj);         // 일반 컴포넌트 목록 및 우클릭 삭제
    void DrawAddComponentButton(GameObject* pObj); // Add Component 버튼 및 팝업
};