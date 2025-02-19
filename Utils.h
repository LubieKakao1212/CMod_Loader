#pragma once

#include <string>
#include <filesystem>
#include <Windows.h>
#include "include/json.hpp"
#include <fstream>
namespace fs = std::filesystem;
using json = nlohmann::json;

class Utils {
	/// <summary>
	/// Loads runtime config specified by the config path and extracts the target framework field value.
	/// </summary>
	/// <param name="configPath"></param>
	/// <returns></returns>
	static std::optional<std::string> ExtractTargetFrameworkFromRuntimeConfig(fs::path configPath);

	/// <summary>
	/// Returns the current executable directory.
	/// </summary>
	/// <returns></returns>
	static fs::path GetExecutableDirectory();

	/// <summary>
	/// Returns a list of directory paths found in directory `path`.
	/// 
	/// Based on: https://stackoverflow.com/a/46589798
	/// </summary>
	/// <param name="path"></param>
	/// <returns></returns>
	static std::vector<fs::path> GetDirectories(const fs::path& path);

	// <summary>
	/// Returns a list of regular file paths found in directory `path`.
	/// 
	/// Based on: https://stackoverflow.com/a/46589798
	/// </summary>
	/// <param name="path"></param>
	/// <returns></returns>
	static std::vector<fs::path> GetRegularFiles(const fs::path& path);
};