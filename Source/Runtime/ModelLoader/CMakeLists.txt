add_krendrr_target(STATIC ModelLoader TARGET_NAME)

target_link_libraries(${TARGET_NAME} PUBLIC
    krendrr::runtime::renderer::core
    krendrr::runtime::renderapi::core
    krendrr::runtime::taskflowex
)

target_link_libraries(${TARGET_NAME} PRIVATE
    krendrr::thirdparty::glm
    krendrr::thirdparty::assimp
    krendrr::thirdparty::nvtx
    krendrr::runtime::mipmapsgenerator
)