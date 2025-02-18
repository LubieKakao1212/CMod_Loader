namespace CMod_Helper {
    public static class FileLog {
        private static bool initialized = false;
        private static string logfilePath;
        public static void Log(string message) {
            if(!initialized) {
                Initialize();
            }

            string datetimePrefix = DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss");

            File.AppendAllText(logfilePath, $"[{datetimePrefix}] {message}\n");
        }

        /// <summary>
        /// Logs a separator for visual clarity.
        /// </summary>
        public static void Separator() {
            if(!initialized) {
                Initialize();
            }

            Log("======================");
        }

        private static void Initialize() {
            logfilePath = Path.Combine(
                Utils.GetPathToCurrentDllDirectory(),
                "Logfile.log"
            );

            if(File.Exists(logfilePath)) {
                File.Delete(logfilePath);
            }

            initialized = true;
        }
    }
}
