add_krendrr_target(EXECUTABLE SimpleDeferredRendering TARGET_NAME)

target_link_libraries(${TARGET_NAME} PRIVATE
    krendrr::runtime::application
    krendrr::runtime::rendercore
    krendrr::thirdparty::assimp
    krendrr::thirdparty::glm
    krendrr::thirdparty::stb_image
)