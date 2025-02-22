using System.Reflection;

namespace CMod_Helper {
    class Utils {
        /// <summary>
        /// Get path to the current mod root directory.
        /// </summary>
        /// <returns></returns>
        public static string GetPathToModRoot() {
            // this returns path to the directory, desipite the function saying "Name"
            string? path = Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location);
            if(path == null) {
                throw new NullReferenceException("failed to get the current assembly");
            }

            return path;
        }
    }
}
