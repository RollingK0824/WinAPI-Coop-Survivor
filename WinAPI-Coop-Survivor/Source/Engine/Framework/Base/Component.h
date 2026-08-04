#pragma once
#include "Engine/Core/Define.h"

class GameObject;
class ColliderComponent;
class TransformComponent;

enum class PropType
{
	Int,
	Float,
	Bool,
	String,
	WString,
	Vector2,
	Color,
	StringVector,
	Texture
};

struct ExposedProperty
{
	std::string name;
	PropType type;
	void* data;
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
		:m_pOwnerGameObject(owner)
		, m_pTransform(reinterpret_cast<TransformComponent*>(this))
		, m_bIsEnabled(true)
	{
	}

	virtual ~Component() = default;

	virtual Component* Clone() = 0;

	virtual void OnEnable() {}
	virtual void OnDisable() {}

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

	void ExposeVariable(const std::string& name, int* var) { m_vProperties.push_back({ name,PropType::Int,var }); }
	void ExposeVariable(const std::string& name, int16* var) { m_vProperties.push_back({ name,PropType::Int,var }); }
	void ExposeVariable(const std::string& name, float* var) { m_vProperties.push_back({ name,PropType::Float,var }); }
	void ExposeVariable(const std::string& name, bool* var) { m_vProperties.push_back({ name,PropType::Bool,var }); }
	void ExposeVariable(const std::string& name, std::string* var) { m_vProperties.push_back({ name,PropType::String,var }); }
	void ExposeVariable(const std::string& name, std::wstring* var) { m_vProperties.push_back({ name, PropType::WString, var }); }
	void ExposeVariable(const std::string& name, Vector2* var) { m_vProperties.push_back({ name, PropType::Vector2, var }); }
	void ExposeVariable(const std::string& name, D2D1_COLOR_F* var) { m_vProperties.push_back({ name, PropType::Color, var }); }
	void ExposeVariable(const std::string& name, std::vector<std::string>* var) { m_vProperties.push_back({ name, PropType::StringVector, var }); }
	void ExposeTexture(const std::string& name, std::string* textureKey) { m_vProperties.push_back({ name, PropType::Texture, textureKey }); }
	void ExposeTexture(const std::string& name, std::wstring* textureKey) { m_vProperties.push_back({ name, PropType::Texture, textureKey }); }

	const std::vector<ExposedProperty>& GetProperties() const { return m_vProperties; }

	virtual void Awake() {};
	virtual void Start() {};

	void SetEnabled(bool enabled)
	{
		if (m_bIsEnabled == enabled) return;
		m_bIsEnabled = enabled;

		if (m_bIsEnabled)OnEnable();
		else OnDisable();
	}

	bool IsEnabled() const { return m_bIsEnabled; }

	virtual void OnCollision(ColliderComponent* pOtherCollider) {};

	void SetSceneVectorIndex(size_t index) { m_sceneVectorIndex = index; }
	size_t GetSceneVectorIndex()const { return m_sceneVectorIndex; }

	virtual std::string_view GetComponentType() const = 0;

	virtual void Serialize(json& outJson) const
	{
		outJson[EngineKey::Property::IsEnabled.data()] = m_bIsEnabled;
	}
	virtual void Deserialize(const json& inJson)
	{
		if (inJson.contains(EngineKey::Property::IsEnabled.data()))
		{
			m_bIsEnabled = inJson[EngineKey::Property::IsEnabled.data()].get<bool>();
		}
	}

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

private:
	GameObject* m_pOwnerGameObject = nullptr;
	TransformComponent* m_pTransform = nullptr;

private:
	size_t m_sceneVectorIndex = 0;
	std::vector<ExposedProperty> m_vProperties;

	std::function<void(Component*)> m_OnDestroyCallback = nullptr;
};
