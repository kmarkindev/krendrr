add_executable(PhongRenderer)

set(BUILD_SHARED_LIBS OFF)

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

add_custom_target(PhongRenderer_CopyContent
    COMMENT
        "Updating content for Phong Renderer"
    COMMAND
    ${CMAKE_COMMAND} -E make_directory ${EXAMPLE_BINARY_DIR}/Content/
    COMMAND
    ${CMAKE_COMMAND} -E copy_directory_if_different
    ${CMAKE_CURRENT_SOURCE_DIR}/Content/
    ${EXAMPLE_BINARY_DIR}/Content/
)
add_dependencies(PhongRenderer PhongRenderer_CopyContent)

include(FetchContent)
FetchContent_Declare(
    glm
    GIT_REPOSITORY	https://github.com/g-truc/glm.git
    GIT_TAG 	0af55ccecd98d4e5a8d1fad7de25ba429d60e863 # tag 1.0.1
)

FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG 802cd454f25469d3123e678af41364153c132c2a
)

FetchContent_Declare(
    assimp
    GIT_REPOSITORY https://github.com/assimp/assimp.git
    GIT_TAG c35200e38ea8f058812b83de2ef32c6093b0ece2 # tag 5.4.3
)

FetchContent_MakeAvailable(assimp glm stb)

target_link_libraries(PhongRenderer PRIVATE glm::glm assimp::assimp)
target_include_directories(PhongRenderer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${stb_SOURCE_DIR}
)