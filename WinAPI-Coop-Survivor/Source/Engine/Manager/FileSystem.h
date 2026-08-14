#pragma once

namespace FileSystem
{
	bool Exists(const std::string& path);
	bool CreateDirectoryPath(const std::string& path);
	bool RemoveFile(const std::string& path);

	bool ReadJson(const std::string& filePath, json& outJson);
	bool WriteJson(const std::string& filePath, const json& inJson);

	std::vector<std::string> GetFilesInDirectory(
		const std::string& dirPath,
		const std::string& extension,
		bool recursive = false);
}