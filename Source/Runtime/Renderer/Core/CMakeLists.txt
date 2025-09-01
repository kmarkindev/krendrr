add_krendrr_target(STATIC RendererCore TARGET_NAME)

target_link_libraries(${TARGET_NAME} PUBLIC
    krendrr::thirdparty::stb_image
    krendrr::thirdparty::glm
    krendrr::runtime::renderapi::core

    krendrr::thirdparty::opengl
)