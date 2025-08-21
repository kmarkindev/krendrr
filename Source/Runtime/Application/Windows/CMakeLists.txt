add_krendrr_target(STATIC WindowsApplication TARGET_NAME)

target_link_libraries(${TARGET_NAME} PRIVATE
    krendrr::runtime::application::core
    krendrr::thirdparty::sdl3
)