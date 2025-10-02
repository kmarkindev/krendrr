add_krendrr_target(STATIC MipMapsGenerator TARGET_NAME)

target_link_libraries(${TARGET_NAME} PUBLIC
    krendrr::runtime::renderer::core
    krendrr::runtime::renderapi::core
)

target_link_libraries(${TARGET_NAME} PRIVATE
    krendrr::thirdparty::glm
    krendrr::thirdparty::nvtx
)
