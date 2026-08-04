#pragma once
#include "Engine/Framework/Base/Component.h"

class TransformComponent : public Component
{
public:
	CLONEABLE_COMPONENT(TransformComponent)

	TransformComponent(GameObject* owner);
	virtual ~TransformComponent() = default;

	virtual std::string_view GetComponentType() const override
	{
		return EngineKey::Component::Trnasform;
	}

	virtual void Serialize(json& outJson)const override
	{
		Component::Serialize(outJson);

		outJson[EngineKey::Property::Position.data()] = { {"x",m_Position.x},{"y",m_Position.y} };
		outJson[EngineKey::Property::Rotation.data()] = m_Rotation.angle;
		outJson[EngineKey::Property::Scale.data()] = { {"x",m_Scale.x},{"y",m_Scale.y} };
	}

	virtual void Deserialize(const json& inJson)override
	{
		Component::Deserialize(inJson); 

		if (inJson.contains(EngineKey::Property::Position.data()))
		{
			const auto& posJson = inJson[EngineKey::Property::Position.data()];
			m_Position.x = posJson["x"].get<float>();
			m_Position.y = posJson["y"].get<float>();
		}
		if (inJson.contains(EngineKey::Property::Rotation.data()))
		{
			m_Rotation.angle = inJson[EngineKey::Property::Rotation.data()].get<float>();
		}
		if (inJson.contains(EngineKey::Property::Scale.data()))
		{
			const auto& scaleJson = inJson[EngineKey::Property::Scale.data()];
			m_Scale.x = scaleJson["x"].get<float>();
			m_Scale.y = scaleJson["y"].get<float>();
		}
	}

	const Vector2& GetPosition() const { return m_Position; }
	void SetPosition(float x, float y) { m_Position.x = x; m_Position.y = y; }
	void SetPosition(Vector2 position) { m_Position = position; }

	const Rotation& GetRotation() const { return m_Rotation; }
	void SetRotation(float angle) { m_Rotation.angle = angle; }
	void SetRotation(Rotation rotation) { m_Rotation = rotation; }

	const Vector2& GetScale() const { return m_Scale; }
	void SetScale(float scaleX, float scaleY) { m_Scale.x = scaleX; m_Scale.y = scaleY; }
	void SetScale(Vector2 scale) { m_Scale = scale; }

private:
	Vector2 m_Position = { 0.0f,0.0f };
	Rotation m_Rotation = { 0.0f };
	Vector2 m_Scale = { 1.0f,1.0f };
};
