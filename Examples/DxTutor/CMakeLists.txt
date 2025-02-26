add_executable(DxTutor)
target_sources(DxTutor PRIVATE Main.cpp)

target_link_libraries(DxTutor PRIVATE D3D12.lib dxguid.lib Dxgi.lib D3DCompiler.lib)