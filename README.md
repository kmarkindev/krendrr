# krendrr


## Building

All project's dependencies are downloaded and compiled by CMake. You only need to install platform-specific toolchains described below.

Each section describes CMake presets related to the platform/compiler and their requirements.

### Windows build using MSVC (Visual Studio)

Use Visual Studio Installer to install Windows SDK 10.0.x and MSVC v143 toolchain, preferably the latest version.

CMake presets:
- win-msvc-debug
- win-msvc-release

### (Currently not working) Windows build using Clang-cl

Use Visual Studio Installer to install Windows SDK 10.0.x and Clang 19.1.5 with MSBuild support (clang-cl)

CMake presets:
- win-clang-cl-debug
- win-clang-cl-release

### Notes on CMake

- All non-third party and non-custom target names start with krendrr_ prefix
- krendrr target name always includes path to the target in Source folder
- Third party targets should not be linked into krendrr targets directly. 
Instead, a library target with krendrr_ prefix should be created and then used for linking 
- Prefer using add_krendrr_library instead of creating targets manually

