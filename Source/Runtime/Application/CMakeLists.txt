add_krendrr_target(STATIC application TARGET_NAME)

target_link_libraries(${TARGET_NAME} PUBLIC
    krendrr::thirdparty::sdl3
    krendrr::thirdparty::imgui
    krendrr::thirdparty::opengl
    krendrr::thirdparty::imgui_sdl3
    krendrr::thirdparty::imgui_opengl3
    krendrr::thirdparty::glm
)