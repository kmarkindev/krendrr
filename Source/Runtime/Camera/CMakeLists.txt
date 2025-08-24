
add_krendrr_target(STATIC Camera TARGET_NAME)

target_link_libraries(${TARGET_NAME} PRIVATE
    krendrr::runtime::renderer::core
    krendrr::runtime::application::core
    krendrr::thirdparty::glm
)