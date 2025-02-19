using Cosmoteer;
using Cosmoteer.Mods;
using Halfling.IO;
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
    enum HelperLocation {
        LocalMods,
        WorkshopMods
    }

    public class Main {
        private static string cModDllPathFromWithinCModDirectory = Path.Combine("CMod", "Main.dll");
        private static string cModLoaderModDirname = "CMod_Loader";
        private static string cModLoaderDllRealFilename = "CMod_Loader";
        private static string cModLoaderDllFakeFilename = "AVRT.dll";

        [UnmanagedCallersOnly]
        public static void InitializePatches() {
            FileLog.Log("Initializing patches");

            if(GameApp.IsNoModsMode) {
                FileLog.Log("Game is launched with mods disabled. No CMods will be loaded.");
                return;
            }

            string cwd = Utils.GetPathToCurrentDllDirectory();

            //HelperLocation helperLocation = cwd.Contains("Saved Games") ? HelperLocation.LocalMods : HelperLocation.WorkshopMods;

            FileLog.Separator();
            FileLog.Log("Processing Mods:");

            List<(AbsolutePath absolutePath, string modInstallSourceStr, string modDirname)> cModsToLoad = new();
            foreach((AbsolutePath absolutePath, ModInstallSource modInstallSource, string workshopId) in ModInfo.GetAllModFolders()) {
                string modInstallSourceStr;
                switch(modInstallSource) {
                    case ModInstallSource.BuiltIn:
                        modInstallSourceStr = "Built-In";
                        break;
                    case ModInstallSource.User:
                        modInstallSourceStr = "Local";
                        break;
                    case ModInstallSource.Workshop:
                        modInstallSourceStr = "Workshop";
                        break;
                    default:
                        modInstallSourceStr = "UNKNOWN";
                        break;
                }

                string modDirname = GetPathLastBit(absolutePath);

                if(!IsCModModDirectory(absolutePath)) {
                    FileLog.Log($"\t{modDirname} - Non-CMod [{modInstallSourceStr}]");
                    continue;
                }

                if(!Settings.EnabledMods.Contains(absolutePath)) {
                    FileLog.Log($"\t{modDirname} - CMod, Disabled [{modInstallSourceStr}]");
                    continue;
                }

                FileLog.Log($"\t{modDirname} - CMod, TO BE LOADED [{modInstallSourceStr}]");
                cModsToLoad.Add((absolutePath, modInstallSourceStr, modDirname));
            }

            FileLog.Separator();

            FileLog.Log("Loading Mods:");

            foreach((AbsolutePath absolutePath, string modInstallSourceStr, string modDirname) in cModsToLoad) {
                FileLog.Log($"\t{modDirname} - Loading [{modInstallSourceStr}]");

                string dllPath = Path.Combine(absolutePath, cModDllPathFromWithinCModDirectory);
                LoadModDll(dllPath);
            }

            FileLog.Separator();

            FileLog.Log("Loading complete. Enjoy!");
        }

        /// <summary>
        /// Check 
        /// </summary>
        static void CheckForCModLoaderVersionMismatch() {

        }

        /// Checks whether given directory is a valid CMod directory.
        /// </summary>
        /// <returns></returns>
        static bool IsCModModDirectory(AbsolutePath path) {
            return File.Exists(Path.Combine(path, cModDllPathFromWithinCModDirectory));
        }

        /// <summary>
        /// Returns the last segment of a path.
        /// </summary>
        /// <param name="path"></param>
        /// <returns></returns>
        static string GetPathLastBit(AbsolutePath path) {
            return path.ToString().Split(Path.DirectorySeparatorChar).Last();
        }

        public static void LoadModDll(string dllAbsPath) {
            string dllFilenameWithoutExt = Path.GetFileNameWithoutExtension(dllAbsPath);

            Assembly assembly = Assembly.LoadFrom(dllAbsPath);

            Type? classType = assembly.GetType("CMod.Main");
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