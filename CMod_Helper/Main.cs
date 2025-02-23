using Cosmoteer;
using Cosmoteer.Mods;
using Halfling.IO;
using Halfling.ObjectText;
using HarmonyLib;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Runtime.Loader;

[assembly: AssemblyVersion("0.0.2")]
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



namespace CMod_Helper {
    enum HelperLocation {
        LocalMods,
        WorkshopMods
    }

    public class Main {
        public static List<(string dllAbsPath, Assembly? assembly, string? targetNamespace)> cModsToLoad = new();
        public static Harmony harmony;

        private static string cModDllPathFromWithinCModDirectory = Path.Combine("CMod", "Main.dll");


        [UnmanagedCallersOnly]
        public static void InitializePatches() {
            FileLogger.LogInfo("Beep boop");

            if(GameApp.IsNoModsMode) {
                FileLogger.LogInfo("Game is launched with mods disabled. No CMods will be loaded.");
                return;
            }

            //string cwd = Utils.GetPathToCurrentDllDirectory();

            //HelperLocation helperLocation = cwd.Contains("Saved Games") ? HelperLocation.LocalMods : HelperLocation.WorkshopMods;

            //FileLogger.Separator();
            FileLogger.LogInfo("Discovering Mods:");

            //List<(AbsolutePath absolutePath, string modInstallSourceStr, string modDirname)> cModsToLoad = new();
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
                    FileLogger.LogInfo($"\t{modDirname} - Non-CMod [{modInstallSourceStr}]");
                    continue;
                }

                if(!Settings.EnabledMods.Contains(absolutePath)) {
                    FileLogger.LogInfo($"\t{modDirname} - CMod, Disabled [{modInstallSourceStr}]");
                    continue;
                }

                FileLogger.LogInfo($"\t{modDirname} - CMod, TO BE LOADED [{modInstallSourceStr}]");

                string dllPath = Path.Combine(absolutePath, cModDllPathFromWithinCModDirectory);

                cModsToLoad.Add((dllPath, null, null));
            }

            FileLogger.Separator();

            FileLogger.LogInfo("Invoking hooks by patching");

            harmony = new Harmony("cmod_core.aliser.helper");
            var assembly = Assembly.GetExecutingAssembly();
            harmony.PatchAll(assembly);

            //FileLogger.LogInfo("All done. Enjoy!");

            //FileLog.Log("Loading Mods:");

            //foreach((AbsolutePath absolutePath, string modInstallSourceStr, string modDirname) in cModsToLoad) {
            //    FileLog.Log($"\t{modDirname} - Loading [{modInstallSourceStr}]");

            //    string dllPath = Path.Combine(absolutePath, cModDllPathFromWithinCModDirectory);
            //    LoadModDll(dllPath);
            //}

            //FileLog.Separator();
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


        /// <summary>
        /// Loads CMods defined in the list of cmods to load and calling a specified method in 'Main' class inside of the predefined mod assembly.
        /// </summary>
        /// <param name="methodName"></param>
        /// <exception cref="Exception"></exception>
        public static void TryLoadCMods(string methodName) {
            for(int i = 0; i < cModsToLoad.Count; i++) {
                (string dllAbsPath, Assembly? assembly, string? targetNamespace) = cModsToLoad[i];

                FileLogger.LogInfo("Processing CMod: " + dllAbsPath);

                // load mod assembly and find target namespace on first load call, save for further uses.
                if(assembly == null || targetNamespace == null) {
                    // LoadFile allows to load assemblies with the same name, but different path.
                    // super useful for our use case since we have the same dll name for all the mods.
                    ModAssemblyLoadContext assemblyLoadCtx = new ModAssemblyLoadContext();
                    assemblyLoadCtx.LoadFromAssemblyPath(dllAbsPath);
                    assemblyLoadCtx.Resolving += ModAssemblyLoadContext.ResolveDependencies;
                    assembly = ModAssemblyLoadContext.GetMainAssembly(assemblyLoadCtx);

                    string[] namespaceMatches = assembly.GetTypes()
                         .Select(t => t.Namespace)
                         .OfType<string>()
                         .Where(n => n.StartsWith("CModEntrypoint"))
                         .Distinct()
                         .ToArray();

                    if(namespaceMatches.Length == 0) {
                        string msg = $"Entrypoint namespace 'CModEntrypoint*' not found for CMod: {dllAbsPath}. Define a namespace starting with 'CModEntrypoint' and a public 'Main' class to supress the error.";
                        FileLogger.LogFatal(msg);
                        throw new Exception(msg);
                    } else if(namespaceMatches.Length > 1) {
                        string msg = $"Found multiple entrypoint namespaces 'CModEntrypoint*' for CMod: {dllAbsPath}. Make sure only one such namespace is defined.";
                        FileLogger.LogFatal(msg);
                        throw new Exception(msg);
                    }

                    targetNamespace = namespaceMatches[0];

                    cModsToLoad[i] = (dllAbsPath, assembly, targetNamespace);
                }

                Type? classType = assembly.GetType($"{targetNamespace}.Main");
                if(classType == null) {
                    string msg = $"'Main' class not found in CMod: {dllAbsPath}. Make sure the class is defined and public.";
                    FileLogger.LogFatal(msg);
                    throw new Exception(msg);
                }

                MethodInfo? methodInfo = classType.GetMethod(methodName, BindingFlags.Static | BindingFlags.Public);
                if(methodInfo == null) {
                    continue;
                }

                FileLogger.LogInfo($"Invoking {methodName}()");

                methodInfo.Invoke(null, null);
            }
        }
    }

    class ModAssemblyLoadContext : AssemblyLoadContext {
        /// <summary>
        /// Libs shipped with the Helper mod.
        /// 
        /// Can be used as dependency fallback in CMods.
        /// </summary>
        static readonly string[] cModHelperModDependencyFallbackDlls = ["0Harmony.dll"];

        public ModAssemblyLoadContext()
        : base("ModAssemblyLoadContext") { }

        /// <summary>
        /// Returns the main assembly (it should be loaded first though).
        /// </summary>
        /// <returns></returns>
        /// <exception cref="NullReferenceException"></exception>
        public static Assembly GetMainAssembly(AssemblyLoadContext ctx) {
            Assembly? mainAssembly = ctx.Assemblies.FirstOrDefault();
            if(mainAssembly == null) {
                string msg = $"Failed to extract the main assembly";
                FileLogger.LogFatal(msg);
                throw new NullReferenceException(msg);
            }

            return mainAssembly;
        }

        /// <summary>
        /// Resolves dependencies for a given assembly.
        /// </summary>
        /// <param name="ctx"></param>
        /// <param name="depAssemblyName"></param>
        /// <returns></returns>
        /// <exception cref="DllNotFoundException"></exception>
        public static Assembly? ResolveDependencies(AssemblyLoadContext ctx, AssemblyName depAssemblyName) {
            string depAssemblyFilename = depAssemblyName.Name + ".dll";

            FileLogger.LogDebug("Resolving assembly dependency: " + depAssemblyFilename);

            Assembly mainAssembly = GetMainAssembly(ctx);
            string mainAssemblyLoadedFrom = Path.GetDirectoryName(mainAssembly.Location);

            // first, look through the mod libs
            string depAssemblyPathAlongMainAssembly = Path.Combine(mainAssemblyLoadedFrom, depAssemblyFilename);
            if(File.Exists(depAssemblyPathAlongMainAssembly)) {
                FileLogger.LogDebug("Found along with mod DLL at: " + depAssemblyPathAlongMainAssembly);
                return Assembly.LoadFrom(depAssemblyPathAlongMainAssembly);
            }

            // if not found, look through fallback libs shipped with Helper
            string cModHelperModDirPath = Utils.GetPathToModRoot();
            string depAssemblyPathAlongCModHelperModDir = Path.Combine(cModHelperModDirPath, depAssemblyFilename);
            if(File.Exists(depAssemblyPathAlongCModHelperModDir)) {
                FileLogger.LogDebug("Found in CMod Helper mod dir at: " + depAssemblyPathAlongCModHelperModDir);
                return Assembly.LoadFrom(depAssemblyPathAlongCModHelperModDir);
            }

            string msg = $"Failed to load CMod DLL at: {mainAssembly.Location}. Failed to locate a dependency. Tried paths: \n- {depAssemblyPathAlongMainAssembly} \n- {depAssemblyPathAlongCModHelperModDir}";
            FileLogger.LogFatal(msg);
            throw new DllNotFoundException(msg);
        }
    }

    [HarmonyPatch(typeof(ModInfo))]
    [HarmonyPatch("ApplyPreLoadMods")]
    [HarmonyPatch(new Type[] { typeof(OTFile) })]
    static class Patch_ApplyPreLoadMods {
        public static void Prefix() {
            FileLogger.LogInfo("Patching Cosmoteer.Mods.ModInfo.ApplyPreLoadMods() [Prefix]");

            Main.TryLoadCMods("Pre_ApplyPreLoadMods");
            FileLogger.Separator();

            var original = typeof(ModInfo).GetMethod("ApplyPreLoadMods", new Type[] { typeof(OTFile) });
            Main.harmony.Unpatch(original, HarmonyPatchType.Prefix);
        }

        public static void Postfix() {
            FileLogger.LogInfo("Patching Cosmoteer.Mods.ModInfo.ApplyPreLoadMods() [Postfix]");

            Main.TryLoadCMods("Post_ApplyPreLoadMods");

            FileLogger.Separator();

            var original = typeof(ModInfo).GetMethod("ApplyPreLoadMods", new Type[] { typeof(OTFile) });
            Main.harmony.Unpatch(original, HarmonyPatchType.Postfix);
        }
    }

    [HarmonyPatch(typeof(ModInfo))]
    [HarmonyPatch("ApplyPostLoadMods")]
    [HarmonyPatch(new Type[] { })]
    static class Patch_ApplyPostLoadMods {
        public static void Prefix() {
            FileLogger.LogInfo("Patching Cosmoteer.Mods.ModInfo.ApplyPostLoadMods() [Prefix]");

            Main.TryLoadCMods("Pre_ApplyPostLoadMods");

            FileLogger.Separator();

            var original = typeof(ModInfo).GetMethod("ApplyPostLoadMods", new Type[] { });
            Main.harmony.Unpatch(original, HarmonyPatchType.Prefix);
        }

        public static void Postfix() {
            FileLogger.LogInfo("Patching Cosmoteer.Mods.ModInfo.ApplyPostLoadMods() [Postfix]");

            Main.TryLoadCMods("Post_ApplyPostLoadMods");

            FileLogger.Separator();

            var original = typeof(ModInfo).GetMethod("ApplyPostLoadMods", new Type[] { });
            Main.harmony.Unpatch(original, HarmonyPatchType.Postfix);
        }
    }

}

