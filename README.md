# krendrr

![](/Media/room.gif)

PS. This room's window NEEDS antialiasing, lol

### Sponza

![](/Media/sponza.png)

400 meshes, 200 FPS, 1 dynamic (green) and 1 static (red) point light

### Futuristic Room

![](/Media/room.png)

50 meshes, 500 FPS, 1 dynamic point light

## ToDo

- Refactoring for deferred renderer class
- PBR
- Jobify model loading even more
- Jobify renderer (geometry pass, shadow map pass)
- Better PFC for shadow mapping
- Anti-Aliasing
- SSAO
- Integrate spdlog, replace all TODO with logs
- Support for Clang and GCC on Windows
- Assets system

## Building

All project's dependencies are downloaded and compiled by CMake. You only need to install platform-specific toolchains described below.

Each section describes CMake presets related to the platform/compiler and their requirements.

### Windows build using MSVC (Visual Studio)

Use Visual Studio Installer to install Windows SDK 10.0.x and MSVC v143 toolchain, preferably the latest version.

CMake presets:
- win-msvc-debug
- win-msvc-release

### (WIP) Windows build using Clang-cl

Use Visual Studio Installer to install Windows SDK 10.0.x and Clang 19.1.5 with MSBuild support (clang-cl)

CMake presets:
- win-clang-cl-debug
- win-clang-cl-release

## Tools

![](/Media/nsight_systems.png)

![](/Media/rd1.png)

![](/Media/rd2.png)