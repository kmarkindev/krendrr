add_executable(BuildTest)
target_all_sources(BuildTest)

target_link_libraries(BuildTest krendrr::sdl3 krendrr::imgui krendrr::opengl krendrr::assimp krendrr::imgui_sdl3 krendrr::imgui_opengl3 krendrr::glm)