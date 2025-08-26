FetchContent_Declare(
    nvtx
    GIT_REPOSITORY https://github.com/NVIDIA/NVTX.git
    GIT_TAG 3ebbc93ded7285963bff932c678fa367eb393ba6 # 3.3.0
    GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(nvtx)

add_library(krendrr_thirdparty_nvtx INTERFACE)
add_library(krendrr::thirdparty::nvtx ALIAS krendrr_thirdparty_nvtx)

target_link_libraries(krendrr_thirdparty_nvtx INTERFACE nvtx3::nvtx3-cpp)