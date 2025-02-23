namespace CMod_Helper {
    // Available log levels.
    enum LogLevel {
        // Debug log level.
        DEBUG,
        // Info log level.
        INFO,
        // Warning log level.
        WARNING,
        // Error log level.
        ERROR,
        // Fatal log level. Something we can't recover from.
        FATAL
    };

    static class FileLogger {
        public static LogLevel logLevel = LogLevel.DEBUG;

        private static readonly string logfileFilename = "Logfile.log";

        private static readonly string logfilePath = Path.Combine(Utils.GetPathToModRoot(), logfileFilename);
        private static bool wasLogFileCleared = false;

        // Log a message with specified log level.
        public static void Log(LogLevel level, string msg) {
            // ensure logfile is fresh every time
            if(!wasLogFileCleared) {
                if(Path.Exists(logfilePath)) {
                    File.Delete(logfilePath);
                }

                wasLogFileCleared = true;
            }

            DateTime now = DateTime.Now;
            string formattedTime = now.ToString("yyyy/MM/dd HH:mm:ss.ff");
            string logLevelStr = LogLevelToString(level);

            File.AppendAllText(logfilePath, $"[{formattedTime}] [{logLevelStr}] {msg}\n");
        }

        // Debug log level.
        public static void LogDebug(string msg) {
            Log(LogLevel.DEBUG, msg);
        }

        // Info log level.
        public static void LogInfo(string msg) {
            Log(LogLevel.INFO, msg);
        }

        // Warn log level.
        public static void LogWarning(string msg) {
            Log(LogLevel.WARNING, msg);
        }

        // Error log level.
        public static void LogError(string msg) {
            Log(LogLevel.ERROR, msg);
        }

        // Fatal log level.
        public static void LogFatal(string msg) {
            Log(LogLevel.FATAL, msg);
        }

        /// <summary>
        /// Logs a separator for visual clarity.
        /// </summary>
        public static void Separator() {
            LogInfo("======================");
        }

        private static string LogLevelToString(LogLevel logLevel) {
            switch(logLevel) {
                case LogLevel.DEBUG:
                    return "DEBUG";
                case LogLevel.INFO:
                    return "INFO";
                case LogLevel.WARNING:
                    return "WARNING";
                case LogLevel.ERROR:
                    return "ERROR";
                case LogLevel.FATAL:
                    return "FATAL";
                default:
                    throw new Exception("Unknown log level for file logger: " + logLevel);
            }
        }
    }
}