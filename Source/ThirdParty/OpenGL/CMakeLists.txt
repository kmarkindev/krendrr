# We are using GLAD2 to load OpenGL and extensions if needed. Here is the GLAD setup:
# https://gen.glad.sh/#generator=c&api=gl%3D4.6&profile=gl%3Dcore%2Cgles1%3Dcommon
# Extract it from zip during generation, so we don't index GLAD sources in GitHub, etc.

add_library(krendrr_opengl STATIC)
add_library(krendrr::opengl ALIAS krendrr_opengl)

file(ARCHIVE_EXTRACT INPUT ${CMAKE_CURRENT_SOURCE_DIR}/glad.zip DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/extracted_glad_zip)

target_sources(krendrr_opengl PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/extracted_glad_zip/src/gl.c)
target_include_directories(krendrr_opengl PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/extracted_glad_zip/include)

if(WIN32)
    target_sources(krendrr_opengl PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/extracted_glad_zip/src/wgl.c)
    target_link_libraries(krendrr_opengl PRIVATE opengl32.lib)
endif()