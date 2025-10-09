add_krendrr_target(STATIC RenderAPI TARGET_NAME)

target_link_libraries(${TARGET_NAME} PUBLIC
    krendrr::thirdparty::directx12
    krendrr::thirdparty::glm
    krendrr::thirdparty::stb_image
)