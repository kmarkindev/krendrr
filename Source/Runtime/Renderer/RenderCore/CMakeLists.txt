add_library(krendrr_runtime_renderer_render_core STATIC)
add_library(krendrr::runtime::renderer::render_core ALIAS krendrr_runtime_renderer_render_core)
target_all_sources(krendrr_runtime_renderer_render_core)

target_link_libraries(krendrr_runtime_renderer_render_core PUBLIC
    krendrr::thirdparty::opengl
    krendrr::thirdparty::assimp
    krendrr::thirdparty::stb_image
    krendrr::thirdparty::glm
    krendrr::runtime::utils
)

target_include_directories(krendrr_runtime_renderer_render_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Include)