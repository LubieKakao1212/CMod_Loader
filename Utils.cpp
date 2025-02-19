#include <string>
#include <filesystem>
#include <Windows.h>
#include "include/json.hpp";
#include <fstream>
namespace fs = std::filesystem;
using json = nlohmann::json;


static class Utils {
public:
	/// <summary>
	/// Loads runtime config specified by the config path and extracts the target framework field value.
	/// </summary>
	/// <param name="configPath"></param>
	/// <returns></returns>
	static std::optional<std::string> ExtractTargetFrameworkFromRuntimeConfig(fs::path configPath) {
		if (!fs::exists(configPath)) {
			return {};
		}

		std::ifstream configRs(configPath);
		json runtimeOptions = json::parse(configRs)["runtimeOptions"];
		
		if (!runtimeOptions.contains("tfm")) {
			return {};
		}

		return runtimeOptions["tfm"].dump();
	}

	/// <summary>
	/// Returns the current executable directory.
	/// </summary>
	/// <returns></returns>
	static fs::path GetExecutableDirectory() {
		char buffer[MAX_PATH];
		GetModuleFileNameA(NULL, buffer, MAX_PATH);
		std::string::size_type pos = std::string(buffer).find_last_of("\\/");

		return std::string(buffer).substr(0, pos);
	}

	/// <summary>
	/// Returns a list of directory paths found in directory `path`.
	/// 
	/// Based on: https://stackoverflow.com/a/46589798
	/// </summary>
	/// <param name="path"></param>
	/// <returns></returns>
	static std::vector<fs::path> GetDirectories(const fs::path& path) {
		std::vector<fs::path> result{};
		for (auto& iterPath : fs::directory_iterator(path))
			if (iterPath.is_directory())
				result.push_back(iterPath.path());
		return result;
	}

	// <summary>
	/// Returns a list of regular file paths found in directory `path`.
	/// 
	/// Based on: https://stackoverflow.com/a/46589798
	/// </summary>
	/// <param name="path"></param>
	/// <returns></returns>
	static std::vector<fs::path> GetRegularFiles(const fs::path& path) {
		std::vector<fs::path> result{};
		for (auto& iterPath : fs::directory_iterator(path))
			if (iterPath.is_regular_file())
				result.push_back(iterPath.path());
		return result;
	}
};