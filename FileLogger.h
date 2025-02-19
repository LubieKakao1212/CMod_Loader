#pragma once

#include <string>
#include <filesystem>
#include "Utils.h"
#include "Variables.h"
#include <fstream>
#include <sstream>
namespace fs = std::filesystem;

// Available log levels.
enum LogLevel {
	// Debug log level.
	BORING,
	// Info log level.
	INFORMATIVE,
	// Warning log level.
	UH_OH,
	// Error log level.
	ERRRRRRRR,
	// Fatal log level.
	SILKSONG_CAME_OUT
};

// Current log level.
extern LogLevel logLevel;

// Log a message with specified log level.
void Log(LogLevel level, std::string msg, bool lineBreak = true);

// Debug log level.
void LogBoring(std::string msg, bool lineBreak = true);

// Info log level.
void LogYap(std::string msg, bool lineBreak = true);

// Warn log level.
void LogUhOh(std::string msg, bool lineBreak = true);

// Error log level.
void LogErrrrrrrr(std::string msg, bool lineBreak = true);

// Fatal log level.
void LogSilksongCameOut(std::string msg, bool lineBreak = true);

static void InitializeIfNeeded();