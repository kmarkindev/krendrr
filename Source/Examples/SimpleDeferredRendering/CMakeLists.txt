add_executable(krendrr_examples_simple_deferred_rendering)
target_all_sources(krendrr_examples_simple_deferred_rendering)

target_link_libraries(krendrr_examples_simple_deferred_rendering PRIVATE
    krendrr::runtime::sdl_app_base
    krendrr::runtime::renderer::render_core
    krendrr::thirdparty::assimp
    krendrr::thirdparty::glm
    krendrr::thirdparty::stb_image
)

target_setup_content_folder(krendrr_examples_simple_deferred_rendering)
