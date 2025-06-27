add_library(krendrr_common_render STATIC)
add_library(krendrr::common::render ALIAS krendrr_common_render)
target_all_sources(krendrr_common_render)

target_link_libraries(krendrr_common_render PUBLIC
    krendrr::thirdparty::opengl
    krendrr::thirdparty::assimp
    krendrr::thirdparty::stb_image
    krendrr::thirdparty::glm
    krendrr::common::utils
)

target_include_directories(krendrr_common_render PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Include)