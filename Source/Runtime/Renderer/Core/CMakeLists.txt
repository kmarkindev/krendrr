add_krendrr_target(STATIC RendererCore TARGET_NAME)

target_link_libraries(${TARGET_NAME} PUBLIC
    krendrr::thirdparty::glm
    krendrr::thirdparty::opengl
)