FetchContent_Declare(
    D3D12MemoryAllocator
    GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/D3D12MemoryAllocator.git
    GIT_TAG 9ef66bc14edd10dee0de3a545b98578363552f66 # 3.0.1
    GIT_PROGRESS TRUE
)

set(D3D12MA_AGILITY_SDK_DIRECTORY ${CMAKE_BINARY_DIR}/Source/ThirdParty/D3D12/Downloaded/Extracted)
FetchContent_MakeAvailable(D3D12MemoryAllocator)

add_library(krendrr_thirdparty_d3d12memoryallocator INTERFACE)
add_library(krendrr::thirdparty::d3d12memoryallocator ALIAS krendrr_thirdparty_d3d12memoryallocator)

target_link_libraries(krendrr_thirdparty_d3d12memoryallocator INTERFACE D3D12MemoryAllocator)