#include <string>
#include <filesystem>
#include "Utils.h"
#include <numeric>
#include "Variables.h"
namespace fs = std::filesystem;

class CosmoteerUtils {
	/// <summary>
	/// Searches for CMod Loader mod directory in Local and Workshop mods folders.
	/// </summary>
	/// <returns></returns>
	static std::optional<fs::path> FindCModLoaderModDirectory();

	/// <summary>
	/// Searches for CMod Helper mod directory in Local and Workshop mods folders.
	/// </summary>
	/// <returns></returns>
	static std::optional<fs::path> FindCModHelperModDirectory();

	/// <summary>
	/// Searches for CMod Helper mod directory in specified directory.
	/// </summary>
	static std::optional<fs::path> FindCModHelperModDirectoryInDirectory(fs::path dirPath);

	/// <summary>
	/// Searches for a specific filename in immediate subdirectories of given directory.
	/// 
	/// Returns path to the found file, if any.
	/// </summary>
	static std::optional<fs::path> FindFilenameInImmediateSubdirectories(fs::path dirPath, std::string filename);

	/// <summary>
	/// Search for the Workshop mods folder.
	/// </summary>
	/// <returns></returns>
	static std::optional<fs::path> FindWorkshopModsDirectoryPath();

	/// <summary>
	/// Search for the Local mods folder.
	/// </summary>
	/// <returns></returns>
	static std::optional<fs::path> FindLocalModsDirectoryPath();

	/// <summary>
	/// Get current user's steam ID based on Cosmoteer data in Saved Games.
	/// </summary>
	/// <param name="cosmoteerSavedGamesDirPath"></param>
	static std::optional<std::string> FindSteamId();

	/// <summary>
	/// Searches for the Cosmoteer directory in the saved games folder.
	/// </summary>
	/// <returns></returns>
	static std::optional<fs::path> FindCosmoteerDirInSavedGames();

	/// <summary>
	/// Check if we are in Cosmoteer Bin directory.
	/// </summary>
	/// <returns></returns>
	static bool inCosmoteerBinDir();
};