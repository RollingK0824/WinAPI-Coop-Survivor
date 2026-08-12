#pragma once
#include "Engine/Core/Singleton.h"
#include "nlohmann/json.hpp"
#include <string>
#include <fstream>

class FileSystem : public Singleton<FileSystem>
{
	friend class Singleton<FileSystem>;
public:
	std::string GetResourcePath(const std::string& path)
	{
		return "Resources/" + path;
	}

	bool ReadJson(const std::string& filePath, nlohmann::json& outJson)
	{
		std::ifstream file(GetResourcePath(filePath));
		if (!file.is_open())return false;

		file >> outJson;
		file.close();
		return true;
	}
	bool WriteJson(const std::string& filePath, const nlohmann::json& inJson)
	{
		std::ofstream file(GetResourcePath(filePath));
		if (!file.is_open())return false;

		file << inJson.dump(4);
		file.close();
		return true;
	}

};