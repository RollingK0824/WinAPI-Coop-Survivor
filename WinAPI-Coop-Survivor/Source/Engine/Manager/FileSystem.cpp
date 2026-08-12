#include "Engine/Core/pch.h"
#include "FileSystem.h"

namespace FileSystem
{
	bool Exists(const std::string& path)
	{
		return std::filesystem::exists(path);
	}

	bool CreateDirectoryPath(const std::string& path)
	{
		std::error_code ec;
		std::filesystem::create_directories(path, ec);
		return !ec;
	}

	bool RemoveFile(const std::string& path)
	{
		if (!std::filesystem::exists(path)) return false;
		std::error_code ec;
		std::filesystem::remove(path, ec);
		return !ec;
	}

	bool ReadJson(const std::string& filePath, json& outJson)
	{
		std::ifstream file(filePath);
		if (!file.is_open()) return false;

		try
		{
			file >> outJson;
		}
		catch (...)
		{
			return false;
		}
		return true;
	}

	bool WriteJson(const std::string& filePath, const json& inJson)
	{
		std::filesystem::path p(filePath);
		if (p.has_parent_path())
		{
			std::filesystem::create_directories(p.parent_path());
		}

		std::ofstream file(filePath);
		if (!file.is_open()) return false;

		file << inJson.dump(4);
		return true;
	}

	std::vector<std::string> GetFilesInDirectory(
		const std::string& dirPath,
		const std::string& extension,
		bool recursive)
	{
		std::vector<std::string> result;
		if (!std::filesystem::exists(dirPath)) return result;

		auto processEntry = [&](const std::filesystem::directory_entry& entry)
		{
			if (entry.is_regular_file() && entry.path().extension() == extension)
				result.push_back(entry.path().string());
		};

		if (recursive)
		{
			for (const auto& entry : std::filesystem::recursive_directory_iterator(dirPath))
				processEntry(entry);
		}
		else
		{
			for (const auto& entry : std::filesystem::directory_iterator(dirPath))
				processEntry(entry);
		}

		return result;
	}
}
