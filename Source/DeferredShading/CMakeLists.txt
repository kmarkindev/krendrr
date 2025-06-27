add_executable(krendrr_deferred_shading)
target_all_sources(krendrr_deferred_shading)

target_link_libraries(krendrr_deferred_shading PRIVATE
    krendrr::common::sdl_app_base
    krendrr::thirdparty::assimp
    krendrr::thirdparty::glm
    krendrr::thirdparty::stb_image
)

target_setup_content_folder(krendrr_deferred_shading)
