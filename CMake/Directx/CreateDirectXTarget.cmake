if(NOT DEFINED DIRECTX12AGILITYSDK_ROOT_DIR)
    message(FATAL_ERROR "DIRECTX12AGILITYSDK_ROOT_DIR is not specified
    (it should point to Agility SDK installation root)")
endif()

add_library(DirectX12AgilitySdk INTERFACE)

target_include_directories(DirectX12AgilitySdk INTERFACE ${DIRECTX12AGILITYSDK_ROOT_DIR}/include)

# Link to system-wide libs, they are going to find our lib by themselves if it is located near exe
target_link_libraries(DirectX12AgilitySdk INTERFACE D3D12.lib dxguid.lib Dxgi.lib D3DCompiler.lib)

# Copy DLLs into bin directory
#add_custom_command(
#    OUTPUT
#        ${CMAKE_BINARY_DIR}/bin/D3D12Core.dll
#        ${CMAKE_BINARY_DIR}/bin/d3d12SDKLayers.dll
#    COMMAND
#        ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/bin/
#    COMMAND
#        ${CMAKE_COMMAND} -E copy_if_different
#            ${DIRECTX12AGILITYSDK_ROOT_DIR}/bin/x64/D3D12Core.dll
#            ${DIRECTX12AGILITYSDK_ROOT_DIR}/bin/x64/d3d12SDKLayers.dll
#            ${CMAKE_BINARY_DIR}/bin/
#)
#
#add_custom_target(
#        DirectX12AgilitySdk_CopyDirectxDlls
#    DEPENDS
#        ${CMAKE_BINARY_DIR}/bin/D3D12Core.dll
#        ${CMAKE_BINARY_DIR}/bin/d3d12SDKLayers.dll
#)
#add_dependencies(DirectX12AgilitySdk DirectX12AgilitySdk_CopyDirectxDlls)