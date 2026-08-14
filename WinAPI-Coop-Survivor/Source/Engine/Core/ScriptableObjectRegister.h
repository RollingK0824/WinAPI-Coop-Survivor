#pragma once
#include "Engine/Manager/DataManager.h"
#include "Engine/Framework/Base/ScriptableObject.h"

template<typename T>
class SORegistrar
{
public:
	SORegistrar(const std::string& typeName)
	{
		DataManager::GetInstance()->RegisterSOFactory(typeName, []() -> std::shared_ptr<ScriptableObject> {
			return std::make_shared<T>();
		});
	}
};
