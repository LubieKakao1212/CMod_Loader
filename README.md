# Cosmoteer C# Mod Loader (aka CMod Loader)

A utiliy mod that grants ability to load CMods (C# mods). CMod is just a DLL file written in C#, but packaged as a regular `.rules` file mod (with a few changes).

_Heavily based on [EnhancedModLoader](https://github.com/C0dingschmuser/EnhancedModLoader) by [C0dingschmuser](https://github.com/C0dingschmuser) & [eamondo2](https://github.com/eamondo2). The initial injector part is used as-is as I'm to dumb to understand it and do my own rewrite._

**DISCLAIMER: mods written in C# allow for a mod author to do basically anything on your device, so be cautios about which mods you install - do so at your own risk. Potential issues include, but not limited to: crashes, losing save files, breaking the game, executing malicious code.**

# Installation

See the [Workshop page](https://steamcommunity.com/sharedfiles/filedetails/?id=3430188109) for details.

Must be installed with Helper mod (which is a part of this repo under `/CMod_Helper`): https://steamcommunity.com/sharedfiles/filedetails/?id=3430199112

# Usage

Subscribe to CMods and enable them in-game like regular mods. Restarting the game will load them in.

# How to make a CMod

See the [Example CMod](https://github.com/murolem/CMod_Example).

# CMods Examples

-   The [Example CMod](https://github.com/murolem/CMod_Example) demonstrates the basic usage.
-   On the [Loader Workshop page](https://github.com/murolem/CMod_Loader), I will list new cool CMods.
-   Other than that, search "CMod" in Workshop.

# How it works

_**Copy-pasted from [EnhancedModLoader](https://github.com/C0dingschmuser/EnhancedModLoader)**_

AVRT.dll is a Library that Cosmoteer tries to load from it's local path that does not exist. Conveniently the local path is the first one that is being searched for this dll before searching in windows system folders and eventually loading it from there. I'm utilizing this to my advantage by copying the functionality of the original AVRT.dll + adding my own. This is called [Dll Hijacking](https://book.hacktricks.xyz/windows-hardening/windows-local-privilege-escalation/dll-hijacking).

I'm using a slightly modified version of [StackOverflowExcept1on's .net core injector](https://github.com/StackOverflowExcept1on/net-core-injector) to manually load a c# helper dll, EML_Helper.dll, to get all current enabled mods (that contain dlls) and pass them back to the original c++ dll which then loads them.

_**END Copy-pasted from EnhancedModLoader**_

`dllmain.cpp` is compiled to `AVRT.dll`. Using the install script, it's copied to Cosmoteer's `Bin` directory.
Cosmoteer loads it.

For our added functionality, the Loader searches for the Helper mod in both local and Workshop mods' directories (in that order), and injects the found Helper DLL.

The Helper is the actual loader that searches for CMods in local and Workshop mods' directories and loads them in. Helper uses Harmony (as do CMods) to patch some of the game's methods (eg post-mod-load) to server as entrypoint for CMods. The Loader searches for the same hooks defined in CMods, and if found, invokes them in the patched methods.

Both Loader and Helper are always loaded in, even if disabled as mods. But for actual CMods, the enabled/disabled state is respected, and disabled mods will not be loaded. Same goes for when launching the game in no-mods mode (Loader and Helper still get loaded in).

# Troubleshooting

If something goes wrong with the loading process, an error window will likely be shown and CMods disabled for that session. Other than that, both Loader and Helper have their own logfiles.

The Loader has it in Cosmoteer `Bin` directory. The Helper logfile can be found in the Helper mod folder → CMod.

If things keep crashing and erroring, the Loader can be uninstalled using the uninstall script that comes with it (see the Workshop page for details). This will also disable the Helper because it will no longer be loaded by the Loader.

# Developing & Contributing

This is my first C++ project ever, and I barely know the language. I know some C#, so we don't come from nothing. So there's probably some horrible, gut-wrenching, heart-attacking, ship-to-pieces-blowing, stinky :3 code in there.

By that I mean any improvements on the project are super heavily welcome! Especially on C++ part.

## Building

To help with some boring build things, a post-build script `IntegrateAfterBuild.ps1` exist for both Loader and Helper.

After build, it will:

-   Generate a mod directory (**clearing** it if it exists).
-   Copy build artifacts (build files) into the mod directory.
-   Copy `static` folder as-is to the mod directory.
-   Restart Cosmoteer (if enabled), with option to restart in dev mode.

The script **requires** configuration to work.

Check the project configuration before build - it's likely will contain some broken paths.

## Contributing

This is a list of things needed to improve the project, modders experience, etc. Anything goes.

- New hooks. More hooks = more ways to entrypoint the game = more happy modders.
- Code examples for the game's code. Various snippets and such, so the modders won't have to spent three days digging to figure out how to load an image. The [Wiki/Modding](https://cosmoteer.wiki.gg/wiki/Modding) section is one place to put them.
- More stable load flow. Currently there's an artifical delay before injecting Helper which makes the load flow not super-relible.

## Other things

Some things from the [Example mod README](https://github.com/murolem/CMod_Example#Making_a_CMod) can be applied here (especially to the Helper part of the project, as it is kind of similar).

# Links

-   [CMod Loader in Workshop](https://steamcommunity.com/sharedfiles/filedetails/?id=3430188109).
-   [CMod Helper in Workshop](https://steamcommunity.com/sharedfiles/filedetails/?id=3430199112).

# Credits
- [C0dingschmuser](https://github.com/C0dingschmuser) & [eamondo2](https://github.com/eamondo2) for making [EnhancedModLoader](https://github.com/C0dingschmuser/EnhancedModLoader) which this project is based upon.
- Dj0z for inspiration and coming up with the name for mods (CMods).