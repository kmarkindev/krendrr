if(NOT WIN32)
    message(WARNING "Skipping D3D12 target since we are not building for windows")
    return()
endif ()

# DirectX is shipped with every windows installation today, and we don't really need its latest features,
# so no need to download Agility SDK here, just link system libraries.
# BUT, it is a nightmare to use without DirectX-Headers, so download and include them.

add_library(krendrr_directx12 INTERFACE)
add_library(krendrr::directx12 ALIAS krendrr_directx12)

target_link_libraries(krendrr_directx12 INTERFACE D3D12.lib dxguid.lib Dxgi.lib D3DCompiler.lib)

FetchContent_Declare(
    directx12-headers
    GIT_REPOSITORY https://github.com/microsoft/DirectX-Headers.git
    GIT_TAG c94b9b23aaadc2034dd1cad656a5a69f1526f98a # v1.616.0
    EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(directx12-headers)
target_link_libraries(krendrr_directx12 INTERFACE DirectX-Headers)