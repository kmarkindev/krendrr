add_library(Common STATIC)

target_link_libraries(Common PUBLIC
    #Directxtk12
    DirectX12AgilitySdk
)

target_include_directories(Common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(Utils)
add_subdirectory(Window)
add_subdirectory(Render)