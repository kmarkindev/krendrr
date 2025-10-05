FetchContent_Declare(
    taskflow
    GIT_REPOSITORY https://github.com/taskflow/taskflow.git
    GIT_TAG 2dfa50a567d48b8439807f5da8a041ba64d4fb63 # 3.10.0
    GIT_PROGRESS TRUE
)

set(TF_BUILD_TESTS OFF)
set(TF_BUILD_EXAMPLES OFF)
FetchContent_MakeAvailable(taskflow)

add_library(krendrr_thirdparty_taskflow INTERFACE)
add_library(krendrr::thirdparty::taskflow ALIAS krendrr_thirdparty_taskflow)

target_link_libraries(krendrr_thirdparty_taskflow INTERFACE Taskflow::Taskflow)