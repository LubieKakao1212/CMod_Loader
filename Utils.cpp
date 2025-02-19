#include <string>
#include <filesystem>
#include <Windows.h>
#include "include/json.hpp"
#include <fstream>
#include "Utils.h"
namespace fs = std::filesystem;
using json = nlohmann::json;

std::optional<std::string> Utils::ExtractTargetFrameworkFromRuntimeConfig(fs::path configPath) {
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

fs::path Utils::GetExecutableDirectory() {
	char buffer[MAX_PATH];
	GetModuleFileNameA(NULL, buffer, MAX_PATH);
	std::string::size_type pos = std::string(buffer).find_last_of("\\/");

	return std::string(buffer).substr(0, pos);
}

std::vector<fs::path> Utils::GetDirectories(const fs::path& path) {
	std::vector<fs::path> result{};
	for (auto& iterPath : fs::directory_iterator(path))
		if (iterPath.is_directory())
			result.push_back(iterPath.path());
	return result;
}

std::vector<fs::path> Utils::GetRegularFiles(const fs::path& path) {
	std::vector<fs::path> result{};
	for (auto& iterPath : fs::directory_iterator(path))
		if (iterPath.is_regular_file())
			result.push_back(iterPath.path());
	return result;
}