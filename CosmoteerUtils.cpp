#include <string>
#include <filesystem>
#include "Utils.h"
#include <numeric>
#include "Variables.h"
#include "CosmoteerUtils.h"
namespace fs = std::filesystem;

class CosmoteerUtils {
public:	
	static std::optional<fs::path> FindCModLoaderModDirectory() {
		std::vector<fs::path> modsLocations{};

		std::optional<fs::path> localModsDirPathResult = FindLocalModsDirectoryPath();
		if (localModsDirPathResult.has_value()) {
			modsLocations.push_back(localModsDirPathResult.value());
		}

		std::optional<fs::path> workshopModsDirPathResult = FindWorkshopModsDirectoryPath();
		if (workshopModsDirPathResult.has_value()) {
			modsLocations.push_back(workshopModsDirPathResult.value());
		}

		if (modsLocations.size() == 0) {
			return {};
		}

		for (auto& modsDirPath : modsLocations) {
			std::optional<fs::path> cModLoaderDirResult = FindCModLoaderModDirectoryInDirectory(modsDirPath);
			if (cModLoaderDirResult.has_value()) {
				return cModLoaderDirResult.value();
			}
		}

		return {};
	}

	static std::optional<fs::path> FindCModHelperModDirectory() {
		std::vector<fs::path> modsLocations{};

		std::optional<fs::path> localModsDirPathResult = FindLocalModsDirectoryPath();
		if (localModsDirPathResult.has_value()) {
			modsLocations.push_back(localModsDirPathResult.value());
		}

		std::optional<fs::path> workshopModsDirPathResult = FindWorkshopModsDirectoryPath();
		if (workshopModsDirPathResult.has_value()) {
			modsLocations.push_back(workshopModsDirPathResult.value());
		}

		if (modsLocations.size() == 0) {
			return {};
		}

		for (auto& modsDirPath : modsLocations) {
			std::optional<fs::path> cModHelperDirResult = FindCModHelperModDirectoryInDirectory(modsDirPath);
			if (cModHelperDirResult.has_value()) {
				// found dll, but runtime config might be missing
				fs::path cModHelperModDirPath = cModHelperDirResult.value();

				fs::path cModHelperConfigPath = cModHelperModDirPath;
				cModHelperConfigPath /= CMOD_HELPER_CONFIG_FILENAME;
				if (fs::exists(cModHelperConfigPath)) {
					return cModHelperModDirPath;
				}
			}
		}

		return {};
	}

	static std::optional<fs::path> FindCModHelperModDirectoryInDirectory(fs::path dirPath) {
		std::optional<fs::path> pathToCModHelperDll = FindFilenameInImmediateSubdirectories(dirPath, CMOD_HELPER_DLL_FILENAME);
		if (pathToCModHelperDll.has_value()) {
			// not actual filename, but a name of a directory
			return pathToCModHelperDll.value().parent_path().filename();
		}

		return {};
	}

	static std::optional<fs::path> FindFilenameInImmediateSubdirectories(fs::path dirPath, std::string filename) {
		std::vector<fs::path> subDirs = Utils::GetDirectories(dirPath);

		for (auto& subDir : subDirs) {
			fs::path filePath = subDir;
			subDir /= filename;

			if (fs::exists(filePath)) {
				return filePath;
			}
		}

		return {};
	}

	static std::optional<fs::path> FindWorkshopModsDirectoryPath() {
		// make sure we're in the Bin dir
		if (!inCosmoteerBinDir()) {
			return {};
		}

		// steamapps\common\Cosmoteer\bin\
		// ^^^^^^^^^                  ^^^
		// to here                    go from here
		fs::path steamappsDir = Utils::GetExecutableDirectory().parent_path().parent_path().parent_path();

		// find workshop mods
		fs::path fullPath = steamappsDir;
		fullPath /= "workshop";
		fullPath /= "content";
		fullPath /= COSMOTEER_APP_ID;
		if (!fs::exists(fullPath)) {
			//ShowErrorMessageBox(GenerateErrorStr("Workshop mods folder path does not exist. Make sure CMod Heleper mod is installed. \nExpected folder at path: \n" + fullPath.string()));
			return {};
		}

		return fullPath;
	}

	static std::optional<fs::path> FindLocalModsDirectoryPath() {
		std::optional<fs::path> savedGamesCosmoteerDirResult = FindCosmoteerDirInSavedGames();
		if (!savedGamesCosmoteerDirResult.has_value()) {
			return {};
		}

		fs::path fullPath = savedGamesCosmoteerDirResult.value();

		std::optional<std::string> steamIdResult = FindSteamId();
		if (!steamIdResult.has_value()) {
			return {};
		}

		fullPath /= steamIdResult.value();
		fullPath /= "Mods";
		if (!fs::exists(fullPath)) {
			//ShowErrorMessageBox(GenerateErrorStr("Mods folder not found in Saved Games >> Cosmoteer folder. Make sure it exists and contains the CMod Helper mod. \nPath: \n" + fullPath.string()));
			return {};
		}

		return fullPath;
	}

	static std::optional<std::string> FindSteamId() {
		std::optional<fs::path> savedGamesCosmoteerDirResult = FindCosmoteerDirInSavedGames();
		if (!savedGamesCosmoteerDirResult.has_value()) {
			return {};
		}

		//struct 
		std::vector<fs::path> profileDirs = Utils::GetDirectories(savedGamesCosmoteerDirResult.value());
		//auto aaaaa = std::accumulate(profileDirs.begin(), profileDirs.end(), "", [](fs::path accum, fs:: path b) {

		//});

		std::string resultingSteamId = "";
		std::filesystem::file_time_type freshestLogFileModifyTimeInProfileDir{};
		for (auto& profileDirPath : profileDirs) {
			fs::path logDirPath = profileDirPath;
			logDirPath /= "Logs";

			if (!fs::exists(logDirPath)) {
				continue;
			}

			// not filename, but the directory name
			std::string steamId = profileDirPath.filename().string();

			std::vector<fs::path> logFilesPaths = Utils::GetRegularFiles(logDirPath);
			for (auto& logFilePath : logFilesPaths) {
				std::filesystem::file_time_type ftime = std::filesystem::last_write_time(logFilePath);
				if (ftime > freshestLogFileModifyTimeInProfileDir) {
					freshestLogFileModifyTimeInProfileDir = ftime;
					resultingSteamId = steamId;
				}
			}
		}

		if (resultingSteamId == "") {
			return {};
		}

		return resultingSteamId;
	}

	static std::optional<fs::path> FindCosmoteerDirInSavedGames() {
		char* userDirPathCStr = getenv("USERPROFILE");
		if (userDirPathCStr == NULL) {
			//ShowErrorMessageBox(GenerateErrorStr("Path to the user profile does not exist. This indicates a problem with the CMod Loader itself."));
			return {};
		}
		std::string userDirPath{ userDirPathCStr };

		fs::path fullPath = userDirPath;
		fullPath /= "Saved Games";
		fullPath /= "Cosmoteer";
		if (!fs::exists(fullPath)) {
			//ShowErrorMessageBox(GenerateErrorStr("Path to user profile >> Saved Games >> Cosmoteer folder does not exist. This indicates a problem with the CMod Loader itself. \nPath: \n" + fullPath.string()));
			return {};
		}

		return fullPath;
	}

	static bool inCosmoteerBinDir() {
		fs::path cosmoteerExePath = Utils::GetExecutableDirectory();
		cosmoteerExePath /= "Cosmoteer.exe";

		return fs::exists(cosmoteerExePath);
	}

private:
	static std::optional<fs::path> FindCModLoaderModDirectoryInDirectory(fs::path dirPath) {
		std::optional<fs::path> pathToCModLoadDll = FindFilenameInImmediateSubdirectories(dirPath, CMOD_LOADER_DLL_FILENAME);
		if (pathToCModLoadDll.has_value()) {
			// not actual filename, but a name of a directory
			return pathToCModLoadDll.value().parent_path().filename();
		}

		return {};
	}
};