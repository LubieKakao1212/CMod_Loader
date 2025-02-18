using System.Reflection;

namespace CMod_Helper {
    class Utils {
        public static string GetPathToCurrentDllDirectory() {
            // this returns path to the directory, desipite the function saying "Name"
            return Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location);
        }
    }
}
