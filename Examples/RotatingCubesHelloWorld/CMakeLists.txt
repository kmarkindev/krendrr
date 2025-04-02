add_executable(RotatingCubesHelloWorld)
target_sources(RotatingCubesHelloWorld PRIVATE Main.cpp)

target_link_libraries(RotatingCubesHelloWorld PRIVATE Directxtk12 DirectX12AgilitySdk)

set_target_properties(RotatingCubesHelloWorld
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/lib
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/lib
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/lib
    LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/lib
)

add_custom_command(
    OUTPUT
        ${CMAKE_BINARY_DIR}/bin/shaders.hlsl
    COMMAND
        ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/bin/
    COMMAND
        ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/shaders.hlsl
            ${CMAKE_BINARY_DIR}/bin/shaders.hlsl
)

add_custom_target(RotatingCubesHelloWorld_CopyShader
    DEPENDS ${CMAKE_BINARY_DIR}/bin/shaders.hlsl
)
add_dependencies(RotatingCubesHelloWorld RotatingCubesHelloWorld_CopyShader)