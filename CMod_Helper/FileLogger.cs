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
        public static LogLevel logLevel = LogLevel.INFO;

        private static bool initialized = false;
        private static StreamWriter logStreamWriter;
        private static bool wasLastLogMessageWithLinebreak = true;

        // Log a message with specified log level.
        public static void Log(LogLevel level, string msg) {
            InitializeIfNeeded();

            DateTime now = DateTime.Now;
            string formattedTime = now.ToString("yyyy/MM/dd HH:mm:ss.ff");
            string logLevelStr = LogLevelToString(level);

            logStreamWriter.WriteLine($"[{formattedTime}] [{logLevelStr}] {msg}");
            logStreamWriter.Flush();
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
            InitializeIfNeeded();

            LogInfo("======================");
        }

        private static void InitializeIfNeeded() {
            if(initialized) {
                return;
            }

            string logfilePath = Path.Combine(Utils.GetPathToModRoot(), "CMod_Helper.log");
            if(Path.Exists(logfilePath)) {
                File.Delete(logfilePath);
            }

            logStreamWriter = new StreamWriter(logfilePath);
            AppDomain.CurrentDomain.ProcessExit += CurrentDomain_ProcessExit;

            initialized = true;
        }

        private static void CurrentDomain_ProcessExit(object? sender, EventArgs e) {
            LogInfo("Disposing");

            logStreamWriter.Close();
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