using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

[assembly: IgnoresAccessChecksTo("Cosmoteer")]

namespace System.Runtime.CompilerServices {
    [AttributeUsage(AttributeTargets.Assembly, AllowMultiple = true)]
    public class IgnoresAccessChecksToAttribute : Attribute {
        public IgnoresAccessChecksToAttribute(string assemblyName) {
            AssemblyName = assemblyName;
        }

        public string AssemblyName { get; }
    }
}

// this helper finds present cmods and saves paths to them into a file, 
// for the dll injector to use

namespace CMod_Helper {
    public class Main {
        private static string cmodLoaderHelperDllName = "CMod_LoaderHelper.dll";
        private static string cmodModsDir = "CMods";
        private static string modDllName = "Main.dll";

        [UnmanagedCallersOnly]
        public static void InitializePatches() {
            FileLog.Log("initializing...");

            string pathToCurrentDir = Utils.GetPathToCurrentDll();

            string cmodModsDirPath = Path.Combine(pathToCurrentDir, cmodModsDir);

            if(!Directory.Exists(cmodModsDirPath)) {
                FileLog.Log("CMods directory not found");
                return;
            }

            string[] modsDirPaths = Directory.GetDirectories(cmodModsDirPath);
            if(modsDirPaths.Length == 0) {
                FileLog.Log("No mods found in CMods directory");
            } else {
                FileLog.Log($"Found {modsDirPaths.Length} mods");
            }

            for(int i = 0; i < modsDirPaths.Length; i++) {
                string modDirPath = modsDirPaths[i];
                string modDllPath = Path.Join(modDirPath, modDllName);

                // GetFileName will actually return the dir name.
                // it just returns the last part of a path.
                FileLog.Log($"[{i + 1} of {modsDirPaths.Length}] Loading mod: {Path.GetFileName(modDirPath)}");
                FileLog.Log(modDllPath);

                if(!File.Exists(modDllPath)) {
                    FileLog.Log($"Main mod DLL {modDllName} not found within the mod folder");
                    return;
                }

                //string[] files = Directory.GetFiles(modDirPath);
                //if(!files.Contains(modDllName)) {
                //}

                LoadModDll(modDllPath);

                FileLog.Log("Mod loaded!");
            }

            FileLog.Log("Loading for all mods is complete!");
        }

        public static void LoadModDll(string dllAbsPath) {
            string dllFilenameWithoutExt = Path.GetFileNameWithoutExtension(dllAbsPath);

            Assembly assembly = Assembly.LoadFrom(dllAbsPath);

            Type? classType = assembly.GetType("CMod_Example.Main");
            if(classType == null) throw new Exception("No main class type.");
            MethodInfo? methodInfo = classType.GetMethod("InitializePatches", BindingFlags.Static | BindingFlags.Public);
            if(methodInfo == null) throw new Exception("No methodinfo.");

            methodInfo.Invoke(null, null);
        }

        /// <summary>
        /// Checks if the given path is a directory.
        /// </summary>
        /// <param name="path"></param>
        /// <exception cref="DirectoryNotFoundException">When the given path doesn't exist.</exception>
        public static bool IsDirectory(string path) {
            if(!Path.Exists(path)) {
                throw new DirectoryNotFoundException(path);
            }

            return File.GetAttributes(path).HasFlag(FileAttributes.Directory);
        }
    }
}