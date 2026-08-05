#pragma once
#include "Engine/Core/Define.h"

class GameObject;
class ColliderComponent;
class TransformComponent;
class Scene;

enum class PropType
{
	Int,
	Float,
	Bool,
	String,
	WString,
	Vector2,
	Color,
	Rect,
	StringVector,
	Texture,
	ObjectRef
};

struct ExposedProperty
{
	std::string name;
	PropType type;
	void* data = nullptr;
	std::function<uint64(void*)> getInstanceID = nullptr;
	std::function<void(GameObject*, void*)> resolver = nullptr;
	std::function<GameObject* (void*)> getTargetGameObject = nullptr;
};

#define CLONEABLE_COMPONENT(Type) \
	virtual Component* Clone() override { return new Type(*this); }

class Component
{
public:
	Component(GameObject* owner, TransformComponent* transform)
		: m_pOwnerGameObject(owner)
		, m_pTransform(transform)
		, m_bIsEnabled(true)
	{
	}

	Component(GameObject* owner)
		: m_pOwnerGameObject(owner)
		, m_pTransform(reinterpret_cast<TransformComponent*>(this))
		, m_bIsEnabled(true)
	{
	}

	Component(const Component& other)
		: m_pOwnerGameObject(nullptr)
		, m_pTransform(nullptr)
		, m_bIsEnabled(other.m_bIsEnabled)
	{
		intptr_t offset = reinterpret_cast<intptr_t>(this) - reinterpret_cast<intptr_t>(&other);

		m_vProperties = other.m_vProperties;
		for (auto& prop : m_vProperties)
		{
			if (prop.data != nullptr)
			{
				prop.data = reinterpret_cast<void*>(reinterpret_cast<intptr_t>(prop.data) + offset);
			}
		}
	}

	virtual ~Component() = default;
	virtual Component* Clone() = 0;

public:
	__declspec(property(get = GetGameObjectInternal)) GameObject& gameObject;
	__declspec(property(get = GetTransformInternal)) TransformComponent& transform;

public:
	void BindToNewObject(GameObject* newOwner, TransformComponent* newTransform)
	{
		m_pOwnerGameObject = newOwner;
		m_pTransform = newTransform;
	}

	void SetDestroyCallback(std::function<void(Component*)> callback)
	{
		m_OnDestroyCallback = callback;
	}

	virtual void OnDestroy()
	{
		if (m_OnDestroyCallback)
		{
			m_OnDestroyCallback(this);
			m_OnDestroyCallback = nullptr;
		}
	}

	virtual void Awake() {};
	virtual void Start() {};

	virtual void OnEnable() {};
	virtual void OnDisable() {};

	void SetEnable(bool isEnabled)
	{
		if (m_bIsEnabled == isEnabled) return;
		m_bIsEnabled = isEnabled;
		if (m_bIsEnabled)
		{
			OnEnable();
		}
		else
		{
			OnDisable();
		}
	}
	void SetEnabled(bool isEnabled) { SetEnable(isEnabled); }

	bool IsEnabled() const { return m_bIsEnabled; }

	virtual void OnCollision(ColliderComponent* pOtherCollider) {};

	void SetSceneVectorIndex(size_t index) { m_sceneVectorIndex = index; }
	size_t GetSceneVectorIndex() const { return m_sceneVectorIndex; }

	virtual std::string_view GetComponentType() const = 0;

	virtual void Serialize(json& outJson) const;
	virtual void Deserialize(const json& inJson);
	virtual void PostDeserialize(Scene* pScene);

public:

	void ExposeVariable(const std::string& name, int* var) { m_vProperties.push_back({ name, PropType::Int, var }); }
	void ExposeVariable(const std::string& name, int16* var) { m_vProperties.push_back({ name, PropType::Int, var }); }
	void ExposeVariable(const std::string& name, float* var) { m_vProperties.push_back({ name, PropType::Float, var }); }
	void ExposeVariable(const std::string& name, bool* var) { m_vProperties.push_back({ name, PropType::Bool, var }); }
	void ExposeVariable(const std::string& name, std::string* var) { m_vProperties.push_back({ name, PropType::String, var }); }
	void ExposeVariable(const std::string& name, std::wstring* var) { m_vProperties.push_back({ name, PropType::WString, var }); }
	void ExposeVariable(const std::string& name, Vector2* var) { m_vProperties.push_back({ name, PropType::Vector2, var }); }
	void ExposeVariable(const std::string& name, D2D1_COLOR_F* var) { m_vProperties.push_back({ name, PropType::Color, var }); }
	void ExposeVariable(const std::string& name, D2D1_RECT_F* var) { m_vProperties.push_back({ name, PropType::Rect, var }); }
	void ExposeVariable(const std::string& name, std::vector<std::string>* var) { m_vProperties.push_back({ name, PropType::StringVector, var }); }
	void ExposeTexture(const std::string& name, std::wstring* textureKey) { m_vProperties.push_back({ name, PropType::Texture, textureKey }); }
	void ExposeTexture(const std::string& name, std::string* textureKey) { m_vProperties.push_back({ name, PropType::String, textureKey }); }

	template<typename T>
	void ExposeComponent(const std::string& name, T** componentPtr);

	void ExposeGameObject(const std::string& name, GameObject** gameObjectPtr);

	const std::vector<ExposedProperty>& GetProperties() const { return m_vProperties; }

public:
	GameObject& GetGameObjectInternal() const
	{
		assert(m_pOwnerGameObject != nullptr && "NullReference: GameObject is missing!");
		return *m_pOwnerGameObject;
	}
	TransformComponent& GetTransformInternal() const
	{
		assert(m_pTransform != nullptr && "NullReference: Transform is missing!");
		return *m_pTransform;
	}

protected:
	bool m_bIsEnabled = true;
	std::unordered_map<std::string, uint64> m_pendingObjectRefs;
	std::vector<ExposedProperty> m_vProperties;

private:
	GameObject* m_pOwnerGameObject = nullptr;
	TransformComponent* m_pTransform = nullptr;

	size_t m_sceneVectorIndex = 0;
	std::function<void(Component*)> m_OnDestroyCallback = nullptr;
};
