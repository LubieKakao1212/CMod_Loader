#include "string";
#include <filesystem>
#include "Utils.cpp"
#include "Variables.cpp"
#include <fstream>
#include <sstream>
namespace fs = std::filesystem;

//std::string LOGFILE_PATH = "";
bool initialized = false;
std::ofstream logStream;
bool wasLastLogMessageWithLinebreak = true;

enum LogLevel {
	BORING,
	INFORMATIVE,
	UH_OH,
	ERRRRRRRR
};

LogLevel logLevel = LogLevel::INFORMATIVE;

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
		default:
			throw std::runtime_error("Unknown log level: " + logLevel);
	}
}

// Log a message with specified log level.
void Log(LogLevel level, std::string msg, bool lineBreak = true) {
	InitializeIfNeeded();

	if (lineBreak == true) {
		auto now = std::chrono::system_clock::now();
		std::string formatted_time = std::format("{0:%F_%T}", now);

		logStream << std::endl << "[" << formatted_time << "]" << " " << "[" << LogLevelToString << "]" << " " << msg;
	} else {
		logStream << msg;
	}
}

// Debug log level.
void LogBoring(std::string msg, bool lineBreak = true) {
	Log(LogLevel::BORING, msg);
}

// Info log level.
void LogYap(std::string msg, bool lineBreak = true) {
	Log(LogLevel::INFORMATIVE, msg);
}

// Warn log level.
void LogUhOh(std::string msg, bool lineBreak = true) {
	Log(LogLevel::UH_OH, msg);
}

// Error log level.
void LogErrrrrrrr(std::string msg, bool lineBreak = true) {
	Log(LogLevel::ERRRRRRRR, msg);
}

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