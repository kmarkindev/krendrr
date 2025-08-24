FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG 6fa36017cfd5731d617e1a934f0e5ea9c4445b13 # v1.15.3
    GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(spdlog)

add_library(krendrr_thirdparty_spdlog INTERFACE)
add_library(krendrr::thirdparty::spdlog ALIAS krendrr_thirdparty_spdlog)

target_link_libraries(krendrr_thirdparty_spdlog INTERFACE spdlog::spdlog)