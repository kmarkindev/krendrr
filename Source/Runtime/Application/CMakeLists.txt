# Create Core and Platform Specific targets, then make an interface target, so other modules can link to required targets easily

add_subdirectory(Core)

add_library(krendrr_runtime_application INTERFACE)
add_library(krendrr::runtime::application ALIAS krendrr_runtime_application)
target_link_libraries(krendrr_runtime_application INTERFACE krendrr::runtime::application::core)

if(WIN32)
    message(STATUS "Building Application targets for Windows Platform")
    add_subdirectory(Windows)
    target_link_libraries(krendrr_runtime_application INTERFACE krendrr::runtime::application::windows)
else()
    message(FATAL_ERROR "Trying to build for unsupported platform")
endif()
