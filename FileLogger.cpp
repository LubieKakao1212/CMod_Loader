#include <string>
#include <filesystem>
#include "Utils.h"
#include "Variables.h"
#include <fstream>
#include <sstream>
#include "FileLogger.h"
namespace fs = std::filesystem;

bool initialized = false;
std::ofstream logStream;
bool wasLastLogMessageWithLinebreak = true;

LogLevel logLevel = LogLevel::INFORMATIVE;

static void InitializeIfNeeded() {
	if (initialized) {
		return;
	}

	fs::path logfilePath = Utils::GetExecutableDirectory();
	logfilePath /= LOGFILE_FILENAME;

	if (fs::exists(logfilePath)) {
		fs::remove(logfilePath);
	}

	logStream = std::ofstream(logfilePath, std::ios_base::app | std::ios_base::out);

	initialized = true;
}

std::string LogLevelToString(LogLevel logLevel) {
	switch (logLevel) {
		case LogLevel::BORING:
			return "DEBUG";
		case LogLevel::INFORMATIVE:
			return "INFO";
		case LogLevel::UH_OH:
			return "WARNING";
		case LogLevel::ERRRRRRRR:
			return "ERROR";		
		case LogLevel::SILKSONG_CAME_OUT:
			return "FATAL";
		default:
			throw std::runtime_error("Unknown log level for file logger: " + logLevel);
	}
}

void Log(LogLevel level, std::string msg, bool lineBreak) {
	InitializeIfNeeded();

	if (lineBreak == true) {
		auto now = std::chrono::system_clock::now();
		std::string formattedTime = std::format("{0:%F %T}", now);

		logStream << std::endl << "[" << formattedTime << "]" << " " << "[" << LogLevelToString(level) << "]" << " " << msg;
	} else {
		logStream << msg;
	}

	// write on each log message
	logStream.flush();
}

void LogBoring(std::string msg, bool lineBreak) {
	Log(LogLevel::BORING, msg, lineBreak);
}

void LogYap(std::string msg, bool lineBreak) {
	Log(LogLevel::INFORMATIVE, msg, lineBreak);
}

void LogUhOh(std::string msg, bool lineBreak) {
	Log(LogLevel::UH_OH, msg, lineBreak);
}

void LogErrrrrrrr(std::string msg, bool lineBreak) {
	Log(LogLevel::ERRRRRRRR, msg, lineBreak);
}

void LogSilksongCameOut(std::string msg, bool lineBreak) {
	Log(LogLevel::SILKSONG_CAME_OUT, msg, lineBreak);
}