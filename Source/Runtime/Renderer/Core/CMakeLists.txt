add_krendrr_target(STATIC RendererCore TARGET_NAME)

target_link_libraries(${TARGET_NAME} PRIVATE
    krendrr::thirdparty::glm
)