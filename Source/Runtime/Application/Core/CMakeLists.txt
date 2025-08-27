add_krendrr_target(STATIC ApplicationCore TARGET_NAME)

target_link_libraries(${TARGET_NAME} PUBLIC
    krendrr::thirdparty::glm
    krendrr::runtime::renderapi::core
)