if(NOT WIN32)
    message(WARNING "Skipping D3D12 target since we are not building for windows")
    return()
endif ()

add_library(krendrr_thirdparty_directx12 INTERFACE)
add_library(krendrr::thirdparty::directx12 ALIAS krendrr_thirdparty_directx12)

# Note: Debug layer is not getting linked into Release build (recommended by agility sdk docs)

# Note: Since Agility SDK includes DirectX-Headers, no need to fetch them from GitHub

# IMPORTANT: After you make any changes to this script, run any example, stop at debug break, and type "image list" into debugger CLI (LLDB)
# You must see the import of D3D12Core.dll like this "S:\Dev\my\krendrr\Build\win-msvc-debug\Build\D3D12\x64\D3D12Core.dll",
# NOT like this "C:\System32\D3D12Core.dll"
# Note: system's dll always is going to be included in case it has newer version than our's

message(STATUS "Linking Directx 12 from Agility SDK")

set(AGILITY_SDK_NUGET_URL "https://www.nuget.org/api/v2/package/Microsoft.Direct3D.D3D12")
set(AGILITY_SDK_NUGET_VERSION "1.616.1") # Do not forget to update dllexport in .cpp when changing Agility SDK version
set(AGILITY_SDK_BINARIES_DIR ${CMAKE_BINARY_DIR}/Build/D3D12/x64) # Must be same as dllexport in .cpp

set(NUGET_PACKAGE_FILE ${CMAKE_CURRENT_BINARY_DIR}/Downloaded/package.nupkg)

# Download Agility SDK into binary folder
string(CONCAT AGILITY_SDK_FULL_URL ${AGILITY_SDK_NUGET_URL} "/" ${AGILITY_SDK_NUGET_VERSION})

if(NOT EXISTS ${NUGET_PACKAGE_FILE})
    message(STATUS "Downloading Nuget package")
    file(DOWNLOAD ${AGILITY_SDK_FULL_URL} ${NUGET_PACKAGE_FILE})
else()
    message(STATUS "Nuget package already downloaded")
endif()

# Unzip it
set(UNZIP_DIR ${CMAKE_CURRENT_BINARY_DIR}/Downloaded/Extracted)
file(ARCHIVE_EXTRACT INPUT ${NUGET_PACKAGE_FILE} DESTINATION ${UNZIP_DIR})

# Create static library, add includes and link binaries

add_library(agility_sdk STATIC)

# We still need to link D3D12.dll (it acts as a loader for out Agility SDK)
target_link_libraries(agility_sdk PUBLIC
    D3D12.lib
    dxguid.lib
    Dxgi.lib
    D3DCompiler.lib
)

target_include_directories(agility_sdk PUBLIC ${UNZIP_DIR}/build/native/include)
target_link_libraries(krendrr_thirdparty_directx12 INTERFACE agility_sdk)

# There is a .cpp file in nuget package. Build it.
target_sources(agility_sdk PRIVATE ${UNZIP_DIR}/build/native/src/d3dx12/d3dx12_property_format_table.cpp)
target_include_directories(agility_sdk PRIVATE ${UNZIP_DIR}/build/native/include/d3dx12)

# Add file with dllexport of Agility SDK config
# Must be public, so it is compiled into .exe to provide dllexports
target_sources(agility_sdk PUBLIC agility_sdk_params.cpp)

# Because of unique nature of how Agility SDK is linking it's binaries, instead of linking to .lib, we
# need to copy .dlls to configured path
file(MAKE_DIRECTORY ${AGILITY_SDK_BINARIES_DIR})

file(COPY_FILE ${UNZIP_DIR}/build/native/bin/x64/D3D12Core.dll ${AGILITY_SDK_BINARIES_DIR}/D3D12Core.dll ONLY_IF_DIFFERENT INPUT_MAY_BE_RECENT)
if (${CMAKE_BUILD_TYPE} STREQUAL "Debug" OR ${CMAKE_BUILD_TYPE} STREQUAL "RelWithDebInfo")
    file(COPY_FILE ${UNZIP_DIR}/build/native/bin/x64/d3d12SDKLayers.dll ${AGILITY_SDK_BINARIES_DIR}/d3d12SDKLayers.dll ONLY_IF_DIFFERENT INPUT_MAY_BE_RECENT)

    file(COPY_FILE ${UNZIP_DIR}/build/native/bin/x64/d3d12SDKLayers.pdb ${AGILITY_SDK_BINARIES_DIR}/d3d12SDKLayers.pdb ONLY_IF_DIFFERENT INPUT_MAY_BE_RECENT)
    file(COPY_FILE ${UNZIP_DIR}/build/native/bin/x64/D3D12Core.pdb ${AGILITY_SDK_BINARIES_DIR}/D3D12Core.pdb ONLY_IF_DIFFERENT INPUT_MAY_BE_RECENT)
endif()