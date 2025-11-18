# UltimatePhobia

## Downloading the base game

To download the correct version of the base game, download [DepotDownloader](https://github.com/SteamRE/DepotDownloader/releases) and run `DepotDownloader -app 739630 -depot 739631 -manifest 1539425284896419288 -username steamuser123`. Replace `steamuser123` with your Steam username.

## Features

- Comes as a single `Phasmophobia.exe` replacement making it way harder to detect than other mods even by new versions of the game.
- Supports thorough C# reflection
- Very low hook latency thanks to [`UWHook`](https://github.com/niansa/uwhook)
- Can coexist with bepinex or MelonLoader
- Does not interact with official developer controlled environment
  - Connects to inofficial Photon server for multiplayer
  - Uses independent save directory (feel free to copy files over)
- Compiles using LLVM-MinGW on Windows or Linux
- Can generate [Il2CppDumper](https://github.com/Perfare/Il2CppDumper)-like [`script.json`](https://gitlab.com/niansa/ultimatephobia/-/blob/main/il2cpp_dump/script.json) file
- Includes basic game enhancements and cheats
- Uses "safe path" optionally outside game directory for log files, mods and config files
- Has various modloaders:
  - Native DLL
  - WebAssembly
  - ELF
  - Unix Socket
- Native DLL/WebAssembly/ELF mods can be written in all kinds of languages, including:
  - C++
  - AssemblyScript
  - Rust
  - Zig
  - C
- Unix socket mods can be written in even more languages, including:
  - All of the above
  - Python
  - C#
  - Javascript
  - Java
  - Kotlin
  - Go
- WebAssembly mod to native DLL mod compiler
- Unix socket client can load native DLL mods
- C++ wrappers for easy mod creation included
- Existing mods aim to never be broken by UltimatePhobia updates

## Patching CVE-2025-59489

```
wget -O UnityPlayer.dll https://security-patches.unity.com/bc0977e0-21a9-4f6e-9414-4f44b242110a/winplayers/b0280d3f53ca708c3ff088d253b2fa775e679225/UnityPlayerP.dll
```
