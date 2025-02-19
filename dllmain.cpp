#define WIN32_LEAN_AND_MEAN  

#include <Windows.h>
#include <string>
#include <filesystem>
#include <fstream>
#include <sstream>
#include "include/coreclr_delegates.h";
#include "include/hostfxr.h";
#include "CosmoteerUtils.cpp"
#include "FileLogger.cpp"
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

/// <summary>
/// Shows a message box window with given message.
/// </summary>
/// <param name="message"></param>
void ShowInfoMessageBox(std::string message) {
	message = "CMod Loader message:\n" + message;

	MessageBoxA(NULL, message.c_str(), "CMod Loader Message", MB_OK | MB_ICONINFORMATION);
}

/// <summary>
/// Shows a message box window custmozied for warning messages with given message.
/// </summary>
/// <param name="message"></param>
void ShowWarningMessageBox(std::string message) {
	message = "CMod Loader warning:\n" + message;

	MessageBoxA(NULL, message.c_str(), "CMod Loader Warning", MB_OK | MB_ICONWARNING);
}

/// <summary>
/// Shows a message box window custmozied for error messages with given message.
/// Also adds a note saying that no CMods will be loaded, assuming this method is called right before stopping execution due to errors.
/// </summary>
/// <param name="message"></param>
void ShowErrorMessageBox(std::string message) {
	message = "CMod Loader error:\n" + message + "\n\nThe game will load, but no CMods will be loaded.";

	MessageBoxA(NULL, message.c_str(), "CMod Loader Error", MB_OK | MB_ICONERROR);
}




/// <summary>
/// Check whether a new version of the Loader is available in Local/Workshop mods.
/// Show an notice message to the user if a new version is found.
/// </summary>
void CheckForLoaderUpdates() {
	LogYap("Checking for Loader updates by looking through installed mods.");

	std::optional<fs::path> cModLoaderModDir = CosmoteerUtils::FindCModLoaderModDirectory();
	if (!cModLoaderModDir.has_value()) {
		std::string msg = "Failed to find the Loader mod folder when checking for updates.";
		LogErrrrrrrr(msg);
		ShowErrorMessageBox(msg);
	}

	// todo
}

// ====================================

DWORD WINAPI dllThread(HMODULE hModule)
{
	DWORD dwExit = 0;

	// useful logging done right
	LogYap("AWOOOOOGA");
	LogYap("Loader starting");
	LogYap("Checking where we are");

	// shouldn't happen, but maybe
	if (!CosmoteerUtils::inCosmoteerBinDir()) {
		std::string msg = "Not in Cosmoteer directory. Huh?";
		LogErrrrrrrr(msg);
		ShowErrorMessageBox(msg);
		return dwExit;
	}

	fs::path cosmoteerBinDirPath = Utils::GetExecutableDirectory();

	// load Cosmoteer runtime config; get target framework

	LogYap("Loading Cosmoteer runtime config");

	fs::path cosmoteerDllConfigPath = cosmoteerBinDirPath;
	cosmoteerDllConfigPath /= COSMOTEER_DLL_CONFIG_FILENAME;

	std::optional<std::string> cosmoteerTfmResult = Utils::ExtractTargetFrameworkFromRuntimeConfig(cosmoteerDllConfigPath);
	if (!cosmoteerTfmResult.has_value()) {
		std::string msg = "Failed to load Cosmoteer runtime config. It either doesn't exist or has the target framework field missing.";
		LogErrrrrrrr(msg);
		ShowErrorMessageBox(msg);
		return dwExit;
	}
	std::string cosmoteerTfm = cosmoteerTfmResult.value();

	LogYap("Cosmoteer target framework: " + cosmoteerTfm);

	// find Helper dir

	LogYap("Searching for Helper");
	std::optional<fs::path> cModHelperDirPathResult = CosmoteerUtils::FindCModHelperModDirectory();
	if (!cModHelperDirPathResult.has_value()) {
		std::string msg = "CMod Helper mod not found. Make sure it is installed.";
		LogErrrrrrrr(msg);
		ShowErrorMessageBox(msg);
		return dwExit;
	}
	fs::path cModHelperDirPath = cModHelperDirPathResult.value();

	LogYap("Helper found at: " + cModHelperDirPath.string());

	// load Helper runtime config; get target framework

	fs::path cModHelperConfigPath = cModHelperDirPath;
	cModHelperConfigPath /= CMOD_HELPER_CONFIG_FILENAME;

	std::optional<std::string> cModHelperTfmResult = Utils::ExtractTargetFrameworkFromRuntimeConfig(cModHelperConfigPath);
	if (!cModHelperTfmResult.has_value()) {
		std::string msg = "Failed to load CMod Helper runtime config. Make sure it's installed correctly.";
		LogErrrrrrrr(msg);
		ShowErrorMessageBox(msg);
		return dwExit;
	}
	std::string cModHelperTfm = cModHelperTfmResult.value();

	LogYap("CMod Helper target framework: " + cosmoteerTfm);

	// check target framework compatability between Cosmoteer and CMod Helper
	LogYap("Checking for target framework combatability between Cosmoteer and CMod Helper");
	if (cosmoteerTfm != cModHelperTfm) {
		std::string msg = "Mismatch between target framework versions of Cosmoteer(" + cosmoteerTfm + ") and CMod Helper (" + cModHelperTfm + "). Make sure the CMod Helper is up to date.";
		LogErrrrrrrr(msg);
		ShowErrorMessageBox(msg);
		return dwExit;
	}

	// load Helper dll
	LogYap("Sleeping before loading Helper DLL to not anger the space gods");

	// Give the game time to initialize
	// Needed for the Loader to be able to access Cosmoteer variables and such.
	Sleep(1000);

	LogYap("Wakey-wakey! Let the Helper DLL loading commence.");

	fs::path cModHelperDllPath = cModHelperDirPath;
	cModHelperDllPath /= CMOD_HELPER_DLL_FILENAME;

	InitializeResult result = LoadDll(cModHelperConfigPath.c_str(), cModHelperDllPath.c_str(), CMOD_HELPER_CLASS_ENTRYPOINT_NAME, CMOD_HELPER_METHOD_ENTRYPOINT_NAME);
	std::string resultStr = InitResultToStr(result);
	if (resultStr != "Success") {
		std::string msg = "Failed to load CMod Helper, reason being: " + resultStr;
		LogErrrrrrrr(msg);
		ShowErrorMessageBox(msg);
		return dwExit;
	}

	LogYap("CMod Helper has been successfully loaded. Have Fun!");

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