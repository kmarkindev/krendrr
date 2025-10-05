add_krendrr_target(STATIC RendererDeferred TARGET_NAME)

target_link_libraries(${TARGET_NAME} PUBLIC
    krendrr::runtime::taskflowex
    krendrr::thirdparty::glm
)

target_link_libraries(${TARGET_NAME} PRIVATE
    krendrr::runtime::renderer::core
    krendrr::runtime::modelloader
    krendrr::thirdparty::nvtx
)