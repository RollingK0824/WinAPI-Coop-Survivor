#pragma once
#include "Engine/Framework/Base/ISystem.h"
#include "Engine/Framework/Base/IUpdatable.h"
#include "Engine/Framework/Base/IRenderable.h"

class GameObject;
class Component;
class ScriptComponent;
class RenderComponent;
class ColliderComponent;

class Scene : public ISystem, public IUpdatable, public IRenderable
{
public:
	Scene() = default;
	virtual ~Scene();

	virtual bool Initialize() override;
	virtual void Release() override;

	virtual void FixedUpdate(float fixedDt) override;
	virtual void Update(float dt) override;
	virtual void LateUpdate(float dt) override;

	virtual void Render() override;

	virtual void PostFrame() override { PostFrameCleanUp(); }

	bool Save(const std::string& filePath);
	bool Load(const std::string& filePath);

	GameObject* CreateGameObject();
	GameObject* CreateGameObject(const std::string& objName);

	void OnComponentAdded(Component* pComponent);

	void UnregisterScriptComponent(ScriptComponent* pComp);
	void UnregisterRenderComponent(RenderComponent* pComp);

	void DestroyObjects(GameObject* pObj);
	void PostFrameCleanUp();

	const std::vector<GameObject*>& GetGameObjects() const { return m_vGameObjects; }

	void SetSceneName(const std::string& name) { m_SceneName = name; }
	const std::string& GetSceneName() const { return m_SceneName; }

private:
	std::vector<GameObject*> m_vGameObjects;
	std::vector<ScriptComponent*> m_vUpdatableComponents;
	std::vector<RenderComponent*> m_vRenderComponents;

	std::vector<GameObject*> m_vCreationQueue;
	std::vector<Component*> m_vComponentCreationQueue;
	
	std::vector<GameObject*> m_vDestroyQueue;

	std::string m_SceneName = "";
};
