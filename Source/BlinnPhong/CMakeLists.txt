add_executable(krendrr_blinnphong)
target_all_sources(krendrr_blinnphong)

target_link_libraries(krendrr_blinnphong PRIVATE
    krendrr::thirdparty::sdl3
    krendrr::thirdparty::imgui
    krendrr::thirdparty::opengl
    krendrr::thirdparty::assimp
    krendrr::thirdparty::imgui_sdl3
    krendrr::thirdparty::imgui_opengl3
    krendrr::thirdparty::glm
    krendrr::thirdparty::stb_image
    krendrr::thirdparty::nlohmann_json
)

target_setup_content_folder(krendrr_blinnphong ${CMAKE_CURRENT_SOURCE_DIR}/Content ${CMAKE_CURRENT_BINARY_DIR}/Build/Content)