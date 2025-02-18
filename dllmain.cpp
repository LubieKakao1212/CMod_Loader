#define WIN32_LEAN_AND_MEAN  

#include <Windows.h>
#include <string>
#include <filesystem>
#include <fstream>
#include <sstream>
#include "include/coreclr_delegates.h";
#include "include/hostfxr.h";
#include "include/json.hpp";
using json = nlohmann::json;
namespace fs = std::filesystem;

#pragma comment(linker,"/export:AvCreateTaskIndex=C:\\Windows\\System32\\avrt.AvCreateTaskIndex,@1")
#pragma comment(linker,"/export:AvQuerySystemResponsiveness=C:\\Windows\\System32\\avrt.AvQuerySystemResponsiveness,@2")
#pragma comment(linker,"/export:AvQueryTaskIndexValue=C:\\Windows\\System32\\avrt.AvQueryTaskIndexValue,@3")
#pragma comment(linker,"/export:AvRevertMmThreadCharacteristics=C:\\Windows\\System32\\avrt.AvRevertMmThreadCharacteristics,@4")
#pragma comment(linker,"/export:AvRtCreateThreadOrderingGroup=C:\\Windows\\System32\\avrt.AvRtCreateThreadOrderingGroup,@5")
#pragma comment(linker,"/export:AvRtCreateThreadOrderingGroupExA=C:\\Windows\\System32\\avrt.AvRtCreateThreadOrderingGroupExA,@6")
#pragma comment(linker,"/export:AvRtCreateThreadOrderingGroupExW=C:\\Windows\\System32\\avrt.AvRtCreateThreadOrderingGroupExW,@7")
#pragma comment(linker,"/export:AvRtDeleteThreadOrderingGroup=C:\\Windows\\System32\\avrt.AvRtDeleteThreadOrderingGroup,@8")
#pragma comment(linker,"/export:AvRtJoinThreadOrderingGroup=C:\\Windows\\System32\\avrt.AvRtJoinThreadOrderingGroup,@9")
#pragma comment(linker,"/export:AvRtLeaveThreadOrderingGroup=C:\\Windows\\System32\\avrt.AvRtLeaveThreadOrderingGroup,@10")
#pragma comment(linker,"/export:AvRtWaitOnThreadOrderingGroup=C:\\Windows\\System32\\avrt.AvRtWaitOnThreadOrderingGroup,@11")
#pragma comment(linker,"/export:AvSetMmMaxThreadCharacteristicsA=C:\\Windows\\System32\\avrt.AvSetMmMaxThreadCharacteristicsA,@12")
#pragma comment(linker,"/export:AvSetMmMaxThreadCharacteristicsW=C:\\Windows\\System32\\avrt.AvSetMmMaxThreadCharacteristicsW,@13")
#pragma comment(linker,"/export:AvSetMmThreadCharacteristicsA=C:\\Windows\\System32\\avrt.AvSetMmThreadCharacteristicsA,@14")
#pragma comment(linker,"/export:AvSetMmThreadCharacteristicsW=C:\\Windows\\System32\\avrt.AvSetMmThreadCharacteristicsW,@15")
#pragma comment(linker,"/export:AvSetMmThreadPriority=C:\\Windows\\System32\\avrt.AvSetMmThreadPriority,@16")
#pragma comment(linker,"/export:AvSetMultimediaMode=C:\\Windows\\System32\\avrt.AvSetMultimediaMode,@17")
#pragma comment(linker,"/export:AvTaskIndexYield=C:\\Windows\\System32\\avrt.AvTaskIndexYield,@18")
#pragma comment(linker,"/export:AvTaskIndexYieldCancel=C:\\Windows\\System32\\avrt.AvTaskIndexYieldCancel,@19")
#pragma comment(linker,"/export:AvThreadOpenTaskIndex=C:\\Windows\\System32\\avrt.AvThreadOpenTaskIndex,@20")

//Credits to StackOverflowExcept1on for this injector
//https://github.com/StackOverflowExcept1on/net-core-injector

// this injector is loaded first into the game,
// then it loads the helper. the helper searches for existing cmods, saving the paths to their dlls into a file.
// then, the injector injects the cmods into the game, calling a method inside each of the loaded dlls.

class Module
{
public:
	static void* getBaseAddress(const char* library)
	{
#ifdef _WIN32
		auto base = GetModuleHandleA(library);
#else
		auto base = dlopen(library, RTLD_LAZY);
#endif
		return reinterpret_cast<void*>(base);
	}

	static void* getExportByName(void* module, const char* name)
	{
#ifdef _WIN32
		auto address = GetProcAddress((HMODULE)module, name);
#else
		auto address = dlsym(module, name);
#endif
		return reinterpret_cast<void*>(address);
	}

	template<typename T>
	static T getFunctionByName(void* module, const char* name)
	{
		return reinterpret_cast<T>(getExportByName(module, name));
	}
};

enum class InitializeResult : uint32_t
{
	Success,
	HostFxrLoadError,
	InitializeRuntimeConfigError,
	GetRuntimeDelegateError,
	EntryPointError,
};

InitializeResult LoadDll(const char_t* runtime_config_path, const char_t* assembly_path, const char_t* type_name, const char_t* method_name)
{
	/// Get module base address
#ifdef _WIN32
	auto libraryName = "hostfxr.dll";
#else
	auto libraryName = "libhostfxr.so";
#endif
	void* module = Module::getBaseAddress(libraryName);
	if (!module)
	{
		return InitializeResult::HostFxrLoadError;
	}

	/// Obtaining useful exports
	auto hostfxr_initialize_for_runtime_config_fptr =
		Module::getFunctionByName<hostfxr_initialize_for_runtime_config_fn>(module, "hostfxr_initialize_for_runtime_config");

	auto hostfxr_get_runtime_delegate_fptr =
		Module::getFunctionByName<hostfxr_get_runtime_delegate_fn>(module, "hostfxr_get_runtime_delegate");

	auto hostfxr_close_fptr =
		Module::getFunctionByName<hostfxr_close_fn>(module, "hostfxr_close");

	/// Load runtime config
	hostfxr_handle ctx = nullptr;
	int rc = hostfxr_initialize_for_runtime_config_fptr(runtime_config_path, nullptr, &ctx);

	/// Success_HostAlreadyInitialized = 0x00000001
	/// @see https://github.com/dotnet/runtime/blob/main/docs/design/features/host-error-codes.md
	if (rc != 1 || ctx == nullptr)
	{
		hostfxr_close_fptr(ctx);
		return InitializeResult::InitializeRuntimeConfigError;
	}

	/// From docs: native function pointer to the requested runtime functionality
	void* delegate = nullptr;
	int ret = hostfxr_get_runtime_delegate_fptr(ctx, hostfxr_delegate_type::hdt_load_assembly_and_get_function_pointer,
		&delegate);

	if (ret != 0 || delegate == nullptr)
	{
		return InitializeResult::GetRuntimeDelegateError;
	}

	/// `void *` -> `load_assembly_and_get_function_pointer_fn`, undocumented???
	auto load_assembly_fptr = reinterpret_cast<load_assembly_and_get_function_pointer_fn>(delegate);

	typedef void (CORECLR_DELEGATE_CALLTYPE* custom_entry_point_fn)();
	custom_entry_point_fn custom = nullptr;

	ret = load_assembly_fptr(assembly_path, type_name, method_name, UNMANAGEDCALLERSONLY_METHOD, nullptr,
		(void**)&custom);

	if (ret != 0 || custom == nullptr)
	{
		return InitializeResult::EntryPointError;
	}

	custom();

	hostfxr_close_fptr(ctx);

	return InitializeResult::Success;
}

std::string InitResultToStr(InitializeResult result)
{
	std::string data;

	switch (result)
	{
	case InitializeResult::Success:
		data = "Success";
		break;
	case InitializeResult::HostFxrLoadError:
		data = "HostFxrLoadError";
		break;
	case InitializeResult::InitializeRuntimeConfigError:
		data = "InitializeRuntimeConfigError";
		break;
	case InitializeResult::GetRuntimeDelegateError:
		data = "GetRuntimeDelegateError";
		break;
	case InitializeResult::EntryPointError:
		data = "EntryPointError";
		break;
	}

	return data;
}

// ====================================

const std::string LOGFILE_FILENAME = "CMod_Loader.log";
const int COSMOTEER_APP_ID = 799600;
const std::string COSMOTEER_EXE_FILENAME = "Cosmoteer.exe";
const std::string COSMOTEER_DLL_FILENAME = "Cosmoteer.dll";
const std::string COSMOTEER_DLL_CONFIG_FILENAME = "Cosmoteer.runtimeconfig.json";
const std::string CMOD_HELPER_DLL_FILENAME = "CMod_Helper.dll";
const std::string CMOD_HELPER_CONFIG_FILENAME = "CMod_Helper.runtimeconfig.json";
// first is the path to the class, second is the dll filename without ext
const char_t* CMOD_HELPER_CLASS_ENTRYPOINT_NAME = L"CMod_Helper.Main, CMod_Helper";
const char_t* CMOD_HELPER_METHOD_ENTRYPOINT_NAME = L"InitializePatches";

std::string LOGFILE_PATH = "";

// ====================================

std::string GetExecutableDirectory() {
	char buffer[MAX_PATH];
	GetModuleFileNameA(NULL, buffer, MAX_PATH);
	std::string::size_type pos = std::string(buffer).find_last_of("\\/");

	return std::string(buffer).substr(0, pos);
}

void SetupLog() {
	fs::path logfilePath = GetExecutableDirectory();
	logfilePath /= LOGFILE_FILENAME;

	if (fs::exists(logfilePath)) {
		fs::remove(logfilePath);
	}

	LOGFILE_PATH = logfilePath.string();
}

void Log(std::string msg, bool newLine = true) {
	std::ofstream log(LOGFILE_PATH, std::ios_base::app | std::ios_base::out);
	log << msg;

	if (newLine) {
		log << std::endl;
	}
}

// ====================================

// returns a list of directory paths for a given path.
// based on: https://stackoverflow.com/a/46589798
std::vector<std::string> GetDirectories(const std::string& path) {
	std::vector<std::string> dirs;
	for (auto& iterPath : std::filesystem::directory_iterator(path))
		if (iterPath.is_directory())
			dirs.push_back(iterPath.path().string());
	return dirs;
}


void ShowWarningMessageBox(std::string message) {
	MessageBoxA(NULL, message.c_str(), "CMod Loader Warning", MB_OK | MB_ICONWARNING);
}

void ShowErrorMessageBox(std::string message) {
	MessageBoxA(NULL, message.c_str(), "CMod Loader Error", MB_OK | MB_ICONERROR);
}

std::string GenerateErrorStr(std::string message) {
	return "CMod Loader error:\n" + message + "\n\nThe game will load, but no CMods will be loaded.";
}

bool AssertDirectoryIsValidCModHelperDirectory(fs::path dirPath, bool suppressMessages = false) {
	// check dll
	fs::path dllPath = dirPath;
	dllPath /= CMOD_HELPER_DLL_FILENAME;
	if (!fs::exists(dllPath)) {
		if (!suppressMessages)
			ShowErrorMessageBox(GenerateErrorStr(CMOD_HELPER_DLL_FILENAME + " not found within the CMod Helper folder. Make sure CMod Helper mod is installed propertly. \nCMod Helper folder: \n" + dirPath.string()));
		return false;
	}

	// check dll config
	fs::path dllConfigPath = dirPath;
	dllConfigPath /= CMOD_HELPER_CONFIG_FILENAME;
	if (!fs::exists(dllConfigPath)) {
		if (!suppressMessages)
			ShowErrorMessageBox(GenerateErrorStr(CMOD_HELPER_CONFIG_FILENAME + " not found within the CMod Helper folder.Make sure CMod Helper mod is installed propertly. \nCMod Helper folder : \n" + dirPath.string()));
		return false;
	}

	return true;
}

/// <summary>
/// Tries to find the CMod Helper directory in the local mods directory.
/// </summary>
/// <returns></returns>
std::optional<fs::path> FindCModHelperFolderInLocalMods() {
	char* userDirPathCStr = getenv("USERPROFILE");
	if (userDirPathCStr == NULL) {
		//ShowErrorMessageBox(GenerateErrorStr("Path to the user profile does not exist. This indicates a problem with the CMod Loader itself."));
		return {};
	}
	std::string userDirPath = std::string(userDirPathCStr);

	fs::path fullPath = userDirPath;
	fullPath /= "Saved Games";
	fullPath /= "Cosmoteer";
	if (!fs::exists(fullPath)) {
		//ShowErrorMessageBox(GenerateErrorStr("Path to user profile >> Saved Games >> Cosmoteer folder does not exist. This indicates a problem with the CMod Loader itself. \nPath: \n" + fullPath.string()));
		return {};
	}

	std::vector<std::string> steamProfileDirs = GetDirectories(fullPath.string());
	if (steamProfileDirs.size() == 0) {
		//ShowErrorMessageBox(GenerateErrorStr("No Steam profile dirs found within Saved Games >> Cosmoteer folder. This indicates a problem with the CMod Loader itself. \nPath: \n" + fullPath.string()));
		return {};
	} else if (steamProfileDirs.size() > 1) {
		//ShowErrorMessageBox(GenerateErrorStr("Found multiple Steam profile dirs within Saved Games >> Cosmoteer folder. Multiple profiles are not supported. \nPath: \n" + fullPath.string()));
		return {};
	}

	fullPath /= steamProfileDirs[0];
	fullPath /= "Mods";
	if (!fs::exists(fullPath)) {
		//ShowErrorMessageBox(GenerateErrorStr("Mods folder not found in Saved Games >> Cosmoteer folder. Make sure it exists and contains the CMod Helper mod. \nPath: \n" + fullPath.string()));
		return {};
	}

	fullPath /= "cmod_helper";
	if (!fs::exists(fullPath)) {
		//ShowErrorMessageBox(GenerateErrorStr("CMod Helper mod folder not found. Make sure CMod Heleper mod is installed. \nExpected folder at path: \n" + fullPath.string()));
		return {};
	}

	return fullPath;
}

std::optional<fs::path> FindCModHelperFolderInWorkshopMods() {
	fs::path cosmoteerBinDir(GetExecutableDirectory());

	// steamapps\common\Cosmoteer\bin\
	// ^^^^^^^^^                  ^^^
	// to here                    go from here
	fs::path steamappsDir = cosmoteerBinDir.parent_path().parent_path().parent_path();

	// find workshop mods
	fs::path fullPath = steamappsDir;
	fullPath /= "workshop";
	fullPath /= "content";
	fullPath /= std::to_string(COSMOTEER_APP_ID);
	if (!fs::exists(fullPath)) {
		ShowErrorMessageBox(GenerateErrorStr("Workshop mods folder path does not exist. Make sure CMod Heleper mod is installed. \nExpected folder at path: \n" + fullPath.string()));
		return {};
	}

	// look through the mod files searching for the Helper
	fs::path cModHelperDirPath = "";
	for (std::string dirPath : GetDirectories(fullPath.string())) {
		fs::path cModHelperPotentialDirPath = dirPath;
		cModHelperPotentialDirPath /= "dirPath";

		if (AssertDirectoryIsValidCModHelperDirectory(cModHelperPotentialDirPath, true)) {
			cModHelperDirPath = cModHelperPotentialDirPath;
		}
	}
	if (cModHelperDirPath == "") {
		ShowErrorMessageBox(GenerateErrorStr("CMod Helper not found in the Workshop mods folder. Make sure CMod Heleper mod is installed. \nWorkshop mods folder: \n" + fullPath.string()));
		return {};
	}

	return cModHelperDirPath;
}

/// <summary>
/// Returns path to a valid CMod Helper directory.
/// An install is considered valid if it has both the dll and the runtimeconfig files of the Helper.
/// 
/// First, the local mods directory is checked. If Helper is not found there, the Workshop directory is checked next.
/// 
/// In case of an invalid install (missing dll or/and runtimeconfig), an early return with an empty result is executed.
/// </summary>
/// <returns></returns>
std::optional<fs::path> FindCModHelperDirPath() {
	std::optional<fs::path> helperDirPathInLocalMods = FindCModHelperFolderInLocalMods();
	if (helperDirPathInLocalMods.has_value()) {
		if (AssertDirectoryIsValidCModHelperDirectory(helperDirPathInLocalMods.value())) {
			return helperDirPathInLocalMods.value();
		} else {
			return {};
		}
	}

	std::optional<fs::path> helperDirPathInWorkshopMods = FindCModHelperFolderInWorkshopMods();
	if (helperDirPathInWorkshopMods.has_value()) {
		if (AssertDirectoryIsValidCModHelperDirectory(helperDirPathInWorkshopMods.value())) {
			return helperDirPathInWorkshopMods.value();
		} else {
			return {};
		}
	}

	return {};
}

// ====================================

DWORD WINAPI dllThread(HMODULE hModule)
{
	DWORD dwExit = 0;
	SetupLog();

	// useful logging done right
	Log("AWOOOOOGA");
	Log("Loader started");

	// assert we are where we need to be (in the Cosmoteer bin dir)

	Log("Locating current working directory");
	std::string cosmoteerBinDirPath = GetExecutableDirectory();

	Log("Locating Cosmoteer executable");
	fs::path cosmoteerExePath = cosmoteerBinDirPath;
	cosmoteerExePath /= COSMOTEER_EXE_FILENAME;
	if (!fs::exists(cosmoteerExePath)) {
		ShowErrorMessageBox(GenerateErrorStr(COSMOTEER_EXE_FILENAME + " not found in the exectuable folder. Make sure the CMod Loader is installed correctly. Executable folder: " + cosmoteerBinDirPath));
		return dwExit;
	}

	// load Cosmoteer runtime config; get target framework

	Log("Loading Cosmoteer runtime config");
	fs::path cosmoteerDllConfigPath = cosmoteerBinDirPath;
	cosmoteerDllConfigPath /= COSMOTEER_DLL_CONFIG_FILENAME;

	if (!fs::exists(cosmoteerDllConfigPath)) {
		ShowErrorMessageBox(GenerateErrorStr(COSMOTEER_DLL_CONFIG_FILENAME + " not found in the exectuable folder. It's expected to be there, so I don't know what to tell ya >:C Executable folder: " + cosmoteerBinDirPath));
		return dwExit;
	}

	std::ifstream cosmoteerExeConfigRs(cosmoteerDllConfigPath);
	json cosmoteerTfm = json::parse(cosmoteerExeConfigRs)["runtimeOptions"]["tfm"];

	// find Helper dir

	Log("Searching for Helper");
	std::optional<fs::path> cModHelperDirPathResult = FindCModHelperDirPath();
	if (!cModHelperDirPathResult.has_value()) {
		Log("Helper not found anywhere.");
		return dwExit;
	}
	fs::path cModHelperDirPath = cModHelperDirPathResult.value();

	Log("Helper directory found: " + cModHelperDirPath.string());

	// get Helper dll path

	fs::path cModHelperDllPath = cModHelperDirPath;
	cModHelperDllPath /= CMOD_HELPER_DLL_FILENAME;

	// load Helper runtime config; get target framework

	Log("Loading Helper runtime config");
	fs::path cModHelperDllConfigPath = cModHelperDirPath;
	cModHelperDllConfigPath /= CMOD_HELPER_CONFIG_FILENAME;

	if (!fs::exists(cModHelperDllConfigPath)) {
		ShowErrorMessageBox(GenerateErrorStr(CMOD_HELPER_CONFIG_FILENAME + " not found in the CMod Helper folder. Make sure the CMod Helper is installed properly. CMod Helper folder: " + cModHelperDirPath.string()));
		return dwExit;
	}

	std::ifstream cModHelperConfigRs(cModHelperDllConfigPath);
	json cModHelperTfm = json::parse(cModHelperConfigRs)["runtimeOptions"]["tfm"];

	// check target framework compatability between Cosmoteer and CMod Helper
	Log("Comparing target frameworks between Cosmoteer and Helper");
	if (cosmoteerTfm.dump() != cModHelperTfm.dump()) {
		Log("Error: Mismatch between framework versions of Cosmoteer (" + cosmoteerTfm.dump() + ") and CMod Helper (" + cModHelperTfm.dump() + ")");
		ShowErrorMessageBox(GenerateErrorStr("Mismatch between framework versions of Cosmoteer (" + cosmoteerTfm.dump() + ") and CMod Helper (" + cModHelperTfm.dump() + "). Make sure the installed CMod Helper is up to date. CMod Helper folder: " + cModHelperDirPath.string()));
		return dwExit;
	}

	// load Helper dll
	Log("Sleeping before loading Helper DLL");

	// Give the game time to initialize
	// Needed for the Loader to be able to access Cosmoteer variables and such.
	Sleep(1000);

	Log("Wakey-wakey! Loading Helper DLL");

	InitializeResult result = LoadDll(cModHelperDllConfigPath.c_str(), cModHelperDllConfigPath.c_str(), CMOD_HELPER_CLASS_ENTRYPOINT_NAME, CMOD_HELPER_METHOD_ENTRYPOINT_NAME);
	std::string resultStr = InitResultToStr(result);

	//LogLine(logPath, resultStr);
	if (resultStr != "Success") {
		Log("Failed to load Helper DLL, reason: " + resultStr);
		ShowErrorMessageBox(GenerateErrorStr("Failed to load CMod Helper DLL, reason: " + resultStr + ". CMod Helper folder : " + cModHelperDirPath.string()));
		return dwExit;
	}

	Log("Helper DLL successfully loaded. Have Fun!");

	FreeLibraryAndExitThread(hModule, 0);
	return dwExit;
}

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)dllThread, hModule, 0, nullptr);
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}