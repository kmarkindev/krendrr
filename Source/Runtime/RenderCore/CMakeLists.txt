add_krendrr_target(STATIC RenderCore TARGET_NAME)

target_link_libraries(${TARGET_NAME} PUBLIC
    krendrr::thirdparty::opengl
    krendrr::thirdparty::assimp
    krendrr::thirdparty::stb_image
    krendrr::thirdparty::glm
    krendrr::runtime::utils
)
