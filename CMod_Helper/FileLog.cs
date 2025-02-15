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

        private static void Initialize() {
            logfilePath = Path.Combine(
                Utils.GetPathToCurrentDll(),
                "CMod_Helper.log"
            );

            if(File.Exists(logfilePath)) {
                File.Delete(logfilePath);
            }

            initialized = true;
        }
    }
}
