using Cosmoteer;
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

namespace CMod_LoaderHelper {
    public class Main {
        private static string cmodLoaderHelperDllName = "CMod_LoaderHelper.dll";
        private static string cmodDllsDir = "bin";
        private static string cmodEntrypointDllFilename = "main.dll";


        [DllImport("user32.dll", SetLastError = true, CharSet = CharSet.Auto)]
        public static extern int MessageBox(int hWnd, string text, string caption, uint type);

        //public static List<string> modDllPaths = new List<string>();
        public static bool modsLoaded = false;

        [UnmanagedCallersOnly]
        public static void InitializePatches() {
            //Halfling.App.Director.FrameEnded += Worker;
            //var cmodDllsAbsPaths = GetCModDllsAbsPathsToLoad()

            FileLog.Log("initializing...");

            List<string> cmodDllsToLoadAbsPaths = new();
            foreach(var (modAbsDirPath, i) in GetAllModDirsAbsPaths().Select((value, i) => (value, i))) {
                FileLog.Log($"[{i + 1}] checking mod for dlls: " + Path.GetDirectoryName(modAbsDirPath));
                var cmodDlls = GetCModDllsAbsPathsToLoad(modAbsDirPath);
                cmodDllsToLoadAbsPaths.AddRange(cmodDlls);
            }

            if(cmodDllsToLoadAbsPaths.Count == 0) {
                FileLog.Log("cmod dlls to be loaded: NONE");
            } else {
                FileLog.Log($"cmod dlls to be loaded ({cmodDllsToLoadAbsPaths.Count}): \n" + String.Join('\n', cmodDllsToLoadAbsPaths));
            }

            foreach(string cmodDllPath in cmodDllsToLoadAbsPaths) {
                LoadDll(cmodDllPath);
            }


        }

        public static void LoadDll(string dllAbsPath) {
            string dllFilenameWithoutExt = Path.GetFileNameWithoutExtension(dllAbsPath);

            Assembly assembly = Assembly.LoadFrom(dllAbsPath);

            Type? classType = assembly.GetType("CMod_Example.Main");
            if(classType == null) throw new Exception("No main class type.");
            MethodInfo? methodInfo = classType.GetMethod("InitializePatches", BindingFlags.Static | BindingFlags.Public);
            if(methodInfo == null) throw new Exception("No methodinfo.");


            //Type? namespaceType = assembly.GetType("CMod_Example");
            //Type? classType = namespaceType.GetNestedType("Main", BindingFlags.Static | BindingFlags.Public);

            //string typeName = $"{dllFilenameWithoutExt}.InitializePatches";
            //Type? type = assembly.GetType("CMod_Example.Main.InitializePatches");
            //if(type == null) {
            //    throw new Exception($"failed to retrieve the InitializePatches method from a CMod dll to be loaded: " + Path.GetFileName(dllAbsPath));
            //}

            //object? instance = Activator.CreateInstance(type);
            //if(instance == null) {
            //    throw new Exception("failed to obtain the instance of InitializePatches method from a CMod dll to be loaded: " + Path.GetFileName(dllAbsPath));
            //}

            methodInfo.Invoke(null, null);

        }

        //public static void Worker(object? sender, EventArgs e) {
        //    //Called after each frame

        //    IAppState? currentState = App.Director.States.OfType<IAppState>().FirstOrDesfault();

        //    if(currentState != null) {
        //        if(currentState.GetType() == typeof(TitleScreen) && !modsLoaded) {
        //            WriteDllPathsToLoad();
        //        }
        //    }
        //}

        //public static void WriteDllPathsToLoad() {
        //    modsLoaded = true;

        //    foreach(Halfling.IO.AbsolutePath folder in GetAllModDirsAbsPaths()) {
        //        GetEnabledCModDllsAbsPaths(folder);
        //    }

        //    //Write enabled mods to file

        //    string exePath = Application.ExecutablePath;
        //    exePath = exePath.Substring(0, exePath.LastIndexOf('\\') + 1);

        //    string modPath = exePath + "eml_mods.ini";
        //    string lockPath = modPath + ".lock";

        //    if(File.Exists(modPath)) {
        //        File.Delete(modPath);
        //    }

        //    if(Main.modDllPaths.Count > 0) {
        //        using StreamWriter file = new(modPath);

        //        foreach(string mod in Main.modDllPaths) {
        //            file.WriteLine(mod);
        //        }
        //    }

        //    File.WriteAllText(lockPath, "done");

        //    Halfling.App.Director.FrameEnded -= Worker;
        //}

        /// <summary>
        /// Returns all the cmod dlls that can be loaded from the given mod folder.
        /// <br></br><br></br>
        /// - If a mod is not a CMod, an empty array will be returned.<br></br>
        /// - If a CMod is disabled, an empty array will be returned.<br></br>
        /// - If it's a CMod Loader directory, an empty array will be returned.
        /// 
        /// </summary>
        /// <param name="modAbsDirPath">Cmod absolute path</param>
        /// <returns></returns>
        /// <exception cref="Exception"></exception> 
        public static string[] GetCModDllsAbsPathsToLoad(string modAbsDirPath) {
            if(!IsDirectory(modAbsDirPath)) {
                throw new DirectoryNotFoundException(modAbsDirPath);
            }

            if(!Settings.EnabledMods.Contains((Halfling.IO.AbsolutePath)modAbsDirPath)) {
                // if disabled, ignore it
                FileLog.Log("   mod is disabled and will be ignored");
                return new string[0];
            }

            string dllsDirAbsPath = Path.Combine(modAbsDirPath, cmodDllsDir);
            if(!Path.Exists(dllsDirAbsPath) || !IsDirectory(dllsDirAbsPath)) {
                // not a cmod, if has no dll dir
                FileLog.Log("   mod is not a CMod (doesn't have a dir for dlls) and will be ignored");
                return new string[0];
            }

            string[] dllsAbsPaths = Directory.GetFiles(dllsDirAbsPath, "*.dll", SearchOption.AllDirectories);
            if(dllsAbsPaths.Length == 0) {
                // doesn't have any dlls
                FileLog.Log("   no dlls found in dlls dir, mod is either not a CMod or just doesn't have any dlls");
                return new string[0];
            }

            List<string> allowedDllsAbsPaths = new List<string>();
            foreach(string dllAbsPath in dllsAbsPaths) {
                FileLog.Log("   | found dll: " + dllAbsPath);

                string dllFilename = Path.GetFileName(dllAbsPath);

                // if a mod loader dir, ignore the mod~
                if(dllFilename == cmodLoaderHelperDllName) {
                    FileLog.Log("   > this is CMod loader itself, the mod dir will be ignored");
                    return new string[0];
                } else if(dllFilename != cmodEntrypointDllFilename) {
                    FileLog.Log("   > not an entrypoint dll, this dll will be ignored");
                    continue;
                }

                allowedDllsAbsPaths.Add(dllAbsPath);
            }

            return allowedDllsAbsPaths.ToArray();
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
        /// Returns a list of absolute paths, each pointing to a mod folder. 
        /// This includes all mods (regular and cmods).
        /// </summary>
        /// <returns></returns>
        public static List<string> GetAllModDirsAbsPaths() {
            // a list of absolute paths to the dll mod folders
            List<string> dllModAbsDirPaths = new List<string>();

            //if(Directory.Exists(Paths.BuiltInModsFolder) && Paths.BuiltInModsFolder != Paths.UserModsFolder) {
            //    string[] directories = Directory.GetDirectories((string?)Paths.BuiltInModsFolder);
            //    foreach(string text in directories) {
            //        dllModAbsoluteDirPaths.Add((Halfling.IO.AbsolutePath)text);
            //    }
            //}

            FileLog.Log("searching for mods in User Mods dir: " + Paths.UserModsFolder);

            // C:\Users\<username>\Saved Games\Cosmoteer\<SteamID>\Mods
            if(Directory.Exists(Paths.UserModsFolder)) {
                string[] dirAbsPaths = Directory.GetDirectories(Paths.UserModsFolder);
                foreach(string dirAbsPath in dirAbsPaths) {
                    dllModAbsDirPaths.Add((Halfling.IO.AbsolutePath)dirAbsPath);
                    FileLog.Log("found mod: " + Path.GetDirectoryName(dirAbsPath));
                }
            }

            //string exeDir = Application.ExecutablePath;
            ////only get directory
            //exeDir = exeDir.Substring(0, exeDir.LastIndexOf('\\') + 1);
            ////go up 3 directories
            //int lastIndex = exeDir.LastIndexOf('\\');
            //if(lastIndex != -1) {
            //    for(int i = 0; i < 3; i++) {
            //        lastIndex = exeDir.LastIndexOf('\\', lastIndex - 1);
            //        if(lastIndex == -1)
            //            break;
            //    }
            //}
            //string workshopDir = lastIndex != -1 ? exeDir.Substring(0, lastIndex) : exeDir;

            //workshopDir += "\\workshop\\content\\799600\\";

            //string[] wdirectories = Directory.GetDirectories(workshopDir);
            //foreach(string folder in wdirectories) {
            //    dllModAbsDirPaths.Add((Halfling.IO.AbsolutePath)folder);
            //}


            FileLog.Log($"search is complete, found mods: {(dllModAbsDirPaths.Count == 0 ? "NONE" : dllModAbsDirPaths.Count)}");

            return dllModAbsDirPaths;
        }
    }
}