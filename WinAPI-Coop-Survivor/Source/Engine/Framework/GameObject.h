#pragma once
#include "Engine/Framework/Base/Component.h"

class Scene;
class ColliderComponent;

class GameObject final
{
public:
	GameObject(Scene* pOwnerScene);
	~GameObject();

	GameObject* Clone(Scene* pNewOwnerScene);

	template<typename T, typename... Args>
	T* AddComponent(Args&&... args)
	{
		static_assert(std::is_base_of<Component, T>::value, "T는 Component를 상속받은 타입이어야 합니다.");

		T* comp = nullptr;

		if constexpr (std::is_same_v<T, TransformComponent>)
		{
			comp = new T(this, std::forward<Args>(args)...);
		}
		else
		{
			comp = new T(this, &transform, std::forward<Args>(args)...);
		}

		m_vComponents.push_back(comp);
		RegisterComponentToScene(comp);

		return comp;
	}

	template<typename T>
	T* GetComponent()
	{
		for (auto* comp : m_vComponents)
		{
			T* casted = dynamic_cast<T*>(comp);
			if (casted) return casted;
		}
		return nullptr;
	}

	void RemoveComponent(Component* comp);

	const std::vector<Component*>& GetComponents()const { return m_vComponents; }
	void Serialize(json& outJson)const;

	bool IsActive() const { return m_bIsActive; }
	void SetActive(bool active);
	Scene* GetOwnerScene() const { return m_pOwnerScene; }

	void OnCollision(ColliderComponent* other);

	void Destroy();
	bool IsDead() const { return m_bIsDead; }

	void SetSceneIndex(size_t index) { m_sceneIndex = index; }
	size_t GetSceneIndex() const { return m_sceneIndex; }

	void SetName(const std::string& name) { m_name = name; }
	std::string GetName() const { return m_name; }

	TransformComponent& transform;

private:
	void RegisterComponentToScene(Component* comp);

	std::string m_name;

	Scene* m_pOwnerScene = nullptr;

	TransformComponent* m_pTransform = nullptr;
	std::vector<Component*> m_vComponents;

	bool m_bIsActive = true;
	bool m_bIsDead = false;

	size_t m_sceneIndex = 0;
};