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
	void Deserialize(const json& inJson);
	void PostDeserialize(Scene* pScene);

	void SetSiblingIndex(int index);
	int GetSiblingIndex() const;
	void SetAsFirstSibling();
	void SetAsLastSibling();

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

	void SetInstanceID(uint64 id);
	uint64 GetInstanceID() const { return m_instanceID; }
private:
	void RegisterComponentToScene(Component* comp);

	static inline uint64 s_nextInstanceID = 1;
	uint64 m_instanceID = 0;
	std::string m_name;

	Scene* m_pOwnerScene = nullptr;

	TransformComponent* m_pTransform = nullptr;
	std::vector<Component*> m_vComponents;

	bool m_bIsActive = true;
	bool m_bIsDead = false;

	size_t m_sceneIndex = 0;
};

template<typename T>
inline void Component::ExposeComponent(const std::string& name, T** componentPtr)
{
	ExposedProperty prop;
	prop.name = name;
	prop.type = PropType::ObjectRef;
	prop.data = reinterpret_cast<void*>(componentPtr);
	prop.getInstanceID = [](void* ptr) -> uint64 {
		T* pTarget = *reinterpret_cast<T**>(ptr);
		if (!pTarget) return 0;
		return pTarget->GetGameObjectInternal().GetInstanceID();
	};
	prop.resolver = [](GameObject* obj, void* ptr) {
		if constexpr (std::is_same_v<T, TransformComponent>) {
			*reinterpret_cast<TransformComponent**>(ptr) = (obj != nullptr ? &obj->transform : nullptr);
		} else {
			*reinterpret_cast<T**>(ptr) = (obj != nullptr ? obj->GetComponent<T>() : nullptr);
		}
	};
	prop.getTargetGameObject = [](void* ptr) -> GameObject* {
		T* pTarget = *reinterpret_cast<T**>(ptr);
		if (!pTarget) return nullptr;
		return &pTarget->GetGameObjectInternal();
	};
	m_vProperties.push_back(prop);
}

inline void Component::ExposeGameObject(const std::string& name, GameObject** gameObjectPtr)
{
	ExposedProperty prop;
	prop.name = name;
	prop.type = PropType::ObjectRef;
	prop.data = reinterpret_cast<void*>(gameObjectPtr);
	prop.getInstanceID = [](void* ptr) -> uint64 {
		GameObject* pTarget = *reinterpret_cast<GameObject**>(ptr);
		return pTarget != nullptr ? pTarget->GetInstanceID() : 0;
	};
	prop.resolver = [](GameObject* obj, void* ptr) {
		*reinterpret_cast<GameObject**>(ptr) = obj;
	};
	prop.getTargetGameObject = [](void* ptr) -> GameObject* {
		return *reinterpret_cast<GameObject**>(ptr);
	};
	m_vProperties.push_back(prop);
}