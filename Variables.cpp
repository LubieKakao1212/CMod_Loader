#pragma once

#include <string>
#include "include/coreclr_delegates.h"
#include "include/hostfxr.h"
#include "Variables.h"

const int SLEEP_BEFORE_HELPER_LOAD_MS = 1000;
const std::string LOGFILE_FILENAME = "CMod_Loader.log";
const std::string COSMOTEER_APP_ID = "799600";
const std::string COSMOTEER_EXE_FILENAME = "Cosmoteer.exe";
const std::string COSMOTEER_DLL_FILENAME = "Cosmoteer.dll";
const std::string COSMOTEER_DLL_CONFIG_FILENAME = "Cosmoteer.runtimeconfig.json";
const std::string CMOD_HELPER_DLL_FILENAME = "CMod_Helper.dll";
const std::string CMOD_HELPER_CONFIG_FILENAME = "CMod_Helper.runtimeconfig.json";
// first is the path to the class, second is the dll filename without ext
const char_t* CMOD_HELPER_CLASS_ENTRYPOINT_NAME = L"CMod_Helper.Main, CMod_Helper";
const char_t* CMOD_HELPER_METHOD_ENTRYPOINT_NAME = L"InitializePatches";
const std::string CMOD_LOADER_DLL_FILENAME = "AVRT.dll";
