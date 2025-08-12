add_executable(krendrr_examples_simple_forward_rendering)
target_all_sources(krendrr_examples_simple_forward_rendering)

target_link_libraries(krendrr_examples_simple_forward_rendering PRIVATE
    krendrr::runtime::sdl_app_base
    krendrr::runtime::utils
    krendrr::runtime::renderer::render_core
    krendrr::thirdparty::glm
)

target_setup_content_folder(krendrr_examples_simple_forward_rendering)