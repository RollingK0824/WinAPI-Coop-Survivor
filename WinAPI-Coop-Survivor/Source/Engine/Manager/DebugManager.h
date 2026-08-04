#pragma once
#include "Engine/Core/Singleton.h"
#include "Engine/Framework/Base/ISystem.h"
#include "Engine/Framework/Base/IUpdatable.h"

class GameObject;
class Scene;
class DebugHUDComponent;

class DebugManager : public Singleton<DebugManager>, public ISystem, public IUpdatable
{
    friend class Singleton<DebugManager>;
public:
    virtual bool Initialize() override;
    virtual void Release() override {}

    virtual void Update(float dt) override;

    GameObject* CreateDebugUIOverlay(Scene* pScene);

    void RegisterDebugHUD(DebugHUDComponent* pComp, GameObject* pRootObj);
    void UnRegisterDebugHUD(DebugHUDComponent* pComp);

    void SetVisible(bool visible);
    bool IsVisible() const { return m_bVisible; }

private:
    DebugManager() = default;
    virtual ~DebugManager() = default;

private:
    bool m_bVisible = false;
    GameObject* m_pDebugUIRoot = nullptr;
    DebugHUDComponent* m_pDebugHUDComp = nullptr;
    const std::string m_actionName = "ToggleDebugOverlay";
};