add_executable(krendrr_blinnphong)
target_all_sources(krendrr_blinnphong)

target_link_libraries(krendrr_blinnphong PRIVATE
    krendrr::common::sdl_app_base
    krendrr::common::utils
    krendrr::common::render
    krendrr::thirdparty::glm
)

target_setup_content_folder(krendrr_blinnphong)