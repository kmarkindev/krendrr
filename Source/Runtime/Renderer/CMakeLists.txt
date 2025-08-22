make_target_name_from_path(UNDERSCORE_TARGET_NAME DOTTED_TARGET_NAME ${CMAKE_CURRENT_SOURCE_DIR})

add_library(${UNDERSCORE_TARGET_NAME} INTERFACE)
add_library(${DOTTED_TARGET_NAME} ALIAS ${UNDERSCORE_TARGET_NAME})

add_subdirectory_all_directories()

target_link_libraries(${UNDERSCORE_TARGET_NAME} INTERFACE
    krendrr::runtime::renderer::core
    krendrr::runtime::renderer::deferred
)