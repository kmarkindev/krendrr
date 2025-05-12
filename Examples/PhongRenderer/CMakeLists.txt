add_executable(PhongRenderer)
target_sources(PhongRenderer PRIVATE Main.cpp)

target_link_libraries(PhongRenderer PRIVATE
    Common
)

set(EXAMPLE_BINARY_DIR ${CMAKE_BINARY_DIR}/bin/Examples/PhongRenderer)

set_target_properties(PhongRenderer
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

add_custom_target(PhongRenderer_CopyShaders
    COMMENT
        "Updating shaders for Phong Renderer"
    COMMAND
    ${CMAKE_COMMAND} -E make_directory ${EXAMPLE_BINARY_DIR}/Shaders/
    COMMAND
    ${CMAKE_COMMAND} -E copy_directory_if_different
    ${CMAKE_CURRENT_SOURCE_DIR}/Shaders/
    ${EXAMPLE_BINARY_DIR}/Shaders/
)
add_dependencies(PhongRenderer PhongRenderer_CopyShaders)