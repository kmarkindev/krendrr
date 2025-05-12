add_executable(RotatingCubesHelloWorld)
target_sources(RotatingCubesHelloWorld PRIVATE Main.cpp)

target_link_libraries(RotatingCubesHelloWorld PRIVATE
    Common
)

set(EXAMPLE_BINARY_DIR ${CMAKE_BINARY_DIR}/bin/Examples/RotatingCubesHelloWorld)

set_target_properties(RotatingCubesHelloWorld
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${EXAMPLE_BINARY_DIR}
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${EXAMPLE_BINARY_DIR}
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${EXAMPLE_BINARY_DIR}
    ARCHIVE_OUTPUT_DIRECTORY ${EXAMPLE_BINARY_DIR}
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${EXAMPLE_BINARY_DIR}
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${EXAMPLE_BINARY_DIR}
    LIBRARY_OUTPUT_DIRECTORY ${EXAMPLE_BINARY_DIR}
    LIBRARY_OUTPUT_DIRECTORY_DEBUG ${EXAMPLE_BINARY_DIR}
    LIBRARY_OUTPUT_DIRECTORY_RELEASE ${EXAMPLE_BINARY_DIR}
)

add_custom_command(
    OUTPUT
        ${EXAMPLE_BINARY_DIR}/shaders.hlsl
    COMMAND
        ${CMAKE_COMMAND} -E make_directory ${EXAMPLE_BINARY_DIR}
    COMMAND
        ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/shaders.hlsl
            ${EXAMPLE_BINARY_DIR}/shaders.hlsl
)

add_custom_target(RotatingCubesHelloWorld_CopyShader
    DEPENDS ${EXAMPLE_BINARY_DIR}/shaders.hlsl
)
add_dependencies(RotatingCubesHelloWorld RotatingCubesHelloWorld_CopyShader)