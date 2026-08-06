#pragma once
#include "Engine/Core/ExposedProperty.h"

class ScriptableObject : public std::enable_shared_from_this<ScriptableObject>
{
public:
	ScriptableObject();
	virtual ~ScriptableObject() = default;

	uint32 GetAssetID() const { return m_assetID; }
	void SetAssetID(uint32 id) { m_assetID = id; }

	const std::string& GetAssetName() const { return m_assetName; }
	void SetAssetName(const std::string& name) { m_assetName = name; }

	const std::string& GetFilePath() const { return m_filePath; }
	void SetFilePath(const std::string& path) { m_filePath = path; }

	virtual void OnLoadFromJson(const json& j);
	virtual json SaveToJson() const;
	virtual void OnSaveToJson(json& j) const {}

	const std::vector<ExposedProperty>& GetProperties() const { return m_properties; }

protected:
	template<typename T>
	void ExposeVariable(const std::string& name, T* ptr)
	{
		ExposedProperty prop;
		prop.name = name;
		prop.data = ptr;

		if constexpr (std::is_same_v<T, int32> || std::is_same_v<T, int>) prop.type = PropType::Int;
		else if constexpr (std::is_same_v<T, float>) prop.type = PropType::Float;
		else if constexpr (std::is_same_v<T, bool>) prop.type = PropType::Bool;
		else if constexpr (std::is_same_v<T, std::string>) prop.type = PropType::String;
		else if constexpr (std::is_same_v<T, std::wstring>) prop.type = PropType::WString;
		else if constexpr (std::is_same_v<T, Vector2>) prop.type = PropType::Vector2;
		else if constexpr (std::is_same_v<T, D2D1_COLOR_F>) prop.type = PropType::Color;
		else if constexpr (std::is_same_v<T, D2D1_RECT_F>) prop.type = PropType::Rect;
		else if constexpr (std::is_same_v<T, std::vector<std::string>>) prop.type = PropType::StringVector;

		m_properties.push_back(prop);
	}

	void ExposeVariable(const std::string& name, std::vector<std::string>* var)
	{
		ExposedProperty prop;
		prop.name = name;
		prop.data = var;
		prop.type = PropType::StringVector;
		m_properties.push_back(prop);
	}

	void ExposeTexture(const std::string& name, std::wstring* ptr)
	{
		ExposedProperty prop;
		prop.name = name;
		prop.data = ptr;
		prop.type = PropType::Texture;
		m_properties.push_back(prop);
	}

protected:
	uint32 m_assetID = 0;
	std::string m_assetName = "";
	std::string m_filePath = "";
	std::vector<ExposedProperty> m_properties;
};
