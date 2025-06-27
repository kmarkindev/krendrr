add_library(krendrr_common_sdl_app_base STATIC)
add_library(krendrr::common::sdl_app_base ALIAS krendrr_common_sdl_app_base)
target_all_sources(krendrr_common_sdl_app_base)

target_link_libraries(krendrr_common_sdl_app_base PUBLIC
    krendrr::thirdparty::sdl3
    krendrr::thirdparty::imgui
    krendrr::thirdparty::opengl
    krendrr::thirdparty::imgui_sdl3
    krendrr::thirdparty::imgui_opengl3
    krendrr::thirdparty::glm
)

target_include_directories(krendrr_common_sdl_app_base PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Include)