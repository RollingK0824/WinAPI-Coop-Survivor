#include "Engine/Core/pch.h"
#include "ScriptableObject.h"

ScriptableObject::ScriptableObject()
{
	ExposeVariable("AssetID", &m_assetID);
	ExposeVariable("AssetName", &m_assetName);
}

void ScriptableObject::OnLoadFromJson(const json& j)
{
	if (j.contains("AssetID"))
	{
		m_assetID = j["AssetID"].get<uint32>();
	}

	if (j.contains("AssetName"))
	{
		m_assetName = j["AssetName"].get<std::string>();
	}
}

json ScriptableObject::SaveToJson() const
{
	json j;
	j["AssetID"] = m_assetID;
	j["AssetName"] = m_assetName;

	OnSaveToJson(j);

	return j;
}
