#pragma once

#include <dxgi1_6.h>

#include "d3dx12/d3dx12.h"

namespace krendrr::Runtime::RenderApi::Core
{
    class RenderApi
    {
    public:

        struct InitParams
        {
            enum class Debug
            {
                None,
                DebugLayer,
                DebugLayerWithGpuBasedValidation
            };

            /**
             * Note: Always none for non Debug builds
             */
            Debug Debug {};

            static InitParams Default()
            {
                // https://bugs.llvm.org/show_bug.cgi?id=36684
                return {};
            }
        };

        bool Initialize(const InitParams& Params = InitParams::Default());
        [[nodiscard]] bool IsValid() const;
        bool Shutdown();

        Microsoft::WRL::ComPtr<ID3D12Device> GetDevice() const;
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> GetCommandQueue() const;
        Microsoft::WRL::ComPtr<IDXGIFactory6> GetDXGIFactory() const;

    private:

        Microsoft::WRL::ComPtr<ID3D12Device> D3dDevice {};
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> D3dCommandQueue {};
        Microsoft::WRL::ComPtr<IDXGIFactory6> DxgiFactory {};

        bool SetupDebugLayer(UINT& DxgiFactoryFlags, const InitParams& Params);
        bool CreateDevice(UINT DxgiFactoryFlags);
        bool CreateCommandQueue(const InitParams& Params);

    };
}
