add_executable(PhongRenderer)

target_link_libraries(PhongRenderer PRIVATE
    Common
)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
target_sources(PhongRenderer PRIVATE ${SOURCES})

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


include(FetchContent)
FetchContent_Declare(
    glm
    GIT_REPOSITORY	https://github.com/g-truc/glm.git
    GIT_TAG 	0af55ccecd98d4e5a8d1fad7de25ba429d60e863 # tag 1.0.1
)
FetchContent_MakeAvailable(glm)
target_link_libraries(PhongRenderer PRIVATE glm::glm)

target_include_directories(PhongRenderer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})