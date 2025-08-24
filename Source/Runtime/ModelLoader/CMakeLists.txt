add_krendrr_target(STATIC ModelLoader TARGET_NAME)

target_link_libraries(${TARGET_NAME} PUBLIC
    krendrr::runtime::renderer::core
)

target_link_libraries(${TARGET_NAME} PRIVATE
    krendrr::thirdparty::glm
    krendrr::thirdparty::opengl
    krendrr::thirdparty::assimp
)