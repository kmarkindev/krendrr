add_krendrr_target(EXECUTABLE DeferredRendering TARGET_NAME)

target_link_libraries(${TARGET_NAME} PRIVATE
    krendrr::runtime::application
    krendrr::runtime::renderer
    krendrr::thirdparty::glm
    krendrr::runtime::modelloader
    krendrr::runtime::camera
)