namespace CMod_LoaderHelper {
    public static class FileLog {
        private static bool wasLogFileClearedIfPreviouslyExisted = false;
        public static void Log(string message) {
            string logFilePath = Path.Combine(
                Environment.GetFolderPath(Environment.SpecialFolder.Desktop),
                "CMod_LoaderHelper.log.txt"
            );

            if(!wasLogFileClearedIfPreviouslyExisted && Path.Exists(logFilePath)) {
                File.Delete(logFilePath);
                wasLogFileClearedIfPreviouslyExisted = true;
            }

            string datetimePrefix = DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss");

            File.AppendAllText(logFilePath, $"{datetimePrefix}: {message}\n");
        }
    }
}
