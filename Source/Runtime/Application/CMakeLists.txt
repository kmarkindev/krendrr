# Create Core and Platform Specific targets, then make an interface target, so other modules can link to required targets easily

add_subdirectory(Core)

make_target_name_from_path(UNDERSCORE_TARGET_NAME DOTTED_TARGET_NAME ${CMAKE_CURRENT_SOURCE_DIR})

add_library(${UNDERSCORE_TARGET_NAME} INTERFACE)
add_library(${DOTTED_TARGET_NAME} ALIAS ${UNDERSCORE_TARGET_NAME})
target_link_libraries(${UNDERSCORE_TARGET_NAME} INTERFACE krendrr::runtime::application::core)

if(WIN32)
    message(STATUS "Building Application targets for Windows Platform")
    add_subdirectory(Windows)
    target_link_libraries(${UNDERSCORE_TARGET_NAME} INTERFACE krendrr::runtime::application::windows)
else()
    message(FATAL_ERROR "Trying to build for unsupported platform")
endif()
