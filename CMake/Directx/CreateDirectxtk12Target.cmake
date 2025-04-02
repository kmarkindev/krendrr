if(NOT DEFINED DIRECTXTK12_ROOT_DIR)
    message(FATAL_ERROR "DIRECTXTK12_ROOT_DIR is not set
    (it should point to DirectXTK 12 installation root)")
endif()

add_library(Directxtk12 INTERFACE)

target_include_directories(Directxtk12 INTERFACE ${DIRECTXTK12_ROOT_DIR}/include)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(LIB_PATH "native/lib/x64/Debug/DirectXTK12.lib")
else()
    set(LIB_PATH "native/lib/x64/Release/DirectXTK12.lib")
endif()

target_link_libraries(Directxtk12 INTERFACE ${DIRECTXTK12_ROOT_DIR}/${LIB_PATH})
