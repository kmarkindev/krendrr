# Download Dear ImGui and since it doesn't have cmake, link all required sources manually

FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG f5befd2d29e66809cd1110a152e375a7f1981f06 # v1.91.9b
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(imgui)

# First create a target for backend-agnostic sources
add_library(krendrr_imgui STATIC)
add_library(krendrr::imgui ALIAS krendrr_imgui)

target_sources(krendrr_imgui PRIVATE
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
)

# Copy .h files into more appropriate folder to support better #include paths,
# and only then link them into main target

file(GLOB IMGUI_COMMON_HEADERS "${imgui_SOURCE_DIR}/*.h")
file(COPY ${IMGUI_COMMON_HEADERS} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/copied_headers)
file(REMOVE ${CMAKE_CURRENT_BINARY_DIR}/copied_headers/imconfig.h)
file(CREATE_LINK ${CMAKE_CURRENT_SOURCE_DIR}/imconfig.h ${CMAKE_CURRENT_BINARY_DIR}/copied_headers/imconfig.h)

target_include_directories(krendrr_imgui PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/copied_headers/)

# And create targets for different backends, so other modules can choose what they need

macro(setup_imgui_backend BACKEND_NAME DEPENDENCIES)

    add_library(krendrr_imgui_${BACKEND_NAME} STATIC EXCLUDE_FROM_ALL)
    add_library(krendrr::imgui_${BACKEND_NAME} ALIAS krendrr_imgui_${BACKEND_NAME})

    target_sources(krendrr_imgui_${BACKEND_NAME} PRIVATE
            ${imgui_SOURCE_DIR}/backends/imgui_impl_${BACKEND_NAME}.cpp
    )
    file(COPY ${imgui_SOURCE_DIR}/backends/imgui_impl_${BACKEND_NAME}.h DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/copied_headers_backed_${BACKEND_NAME})
    target_include_directories(krendrr_imgui_${BACKEND_NAME} PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/copied_headers_backed_${BACKEND_NAME})
    target_link_libraries(krendrr_imgui_${BACKEND_NAME} PUBLIC krendrr_imgui ${DEPENDENCIES})

endmacro()

setup_imgui_backend(opengl3 "")
setup_imgui_backend(dx12 "")
setup_imgui_backend(win32 "")
setup_imgui_backend(sdl3 krendrr::sdl3)
