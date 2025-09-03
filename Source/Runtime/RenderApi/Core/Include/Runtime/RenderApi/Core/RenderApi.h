#pragma once

#include <dxgi1_6.h>
#include <array>
#include <span>

#include "d3dx12/d3dx12.h"

namespace krendrr::Runtime::RenderApi::Core
{
    /**
     * Object of this class holds D3D Device and Direct Queue, so make sure it lives longer than anything that interacts with it,
     * or at least nothing is trying to access D3D resources after RenderApi was destroyed.
     *
     * In case you have created multiple instances of this, make sure you properly share resources and not using "foreign" resources.
     */
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

            bool bEnableShadersDebug {};

            static InitParams Default()
            {
                // https://bugs.llvm.org/show_bug.cgi?id=36684
                return {};
            }
        };

        // not thread-safe
        bool Initialize(const InitParams& Params = InitParams::Default());
        // thread-safe
        [[nodiscard]] bool IsValid() const;
        // not thread-safe
        bool Shutdown();

        // thread-safe
        [[nodiscard]] Microsoft::WRL::ComPtr<ID3D12Device> GetDevice() const;
        // thread-safe
        [[nodiscard]] Microsoft::WRL::ComPtr<ID3D12CommandQueue> GetDirectQueue() const;
        // thread-safe
        [[nodiscard]] Microsoft::WRL::ComPtr<ID3D12CommandQueue> GetCopyQueue() const;
        // thread-safe
        [[nodiscard]] Microsoft::WRL::ComPtr<IDXGIFactory6> GetDXGIFactory() const;

        [[nodiscard]] bool IsShadersDebugEnabled() const;
        [[nodiscard]] unsigned GetShaderCompileFlags() const;

        struct BufferLayout
        {
            std::uint32_t Stride {};
            std::array<D3D12_INPUT_ELEMENT_DESC, 4> Layout {};
        };

        /**
         * For simplicity, all meshes should use same buffer layout
         * (we don't have our own shader compiler to alter root signatures in shaders anyway)
         *
         * thread-safe
         */
        [[nodiscard]] const BufferLayout& GetCommonMeshBufferLayout() const;

        template<typename T>
        static std::span<const std::byte> ContainerToBytes(const T& Container)
        {
            return std::span<const std::byte>(
                reinterpret_cast<const std::byte*>(&*std::begin(Container)), // use &* to support both pointer and iterator values
                std::size(Container) * sizeof(Container[0])
            );
        }

        // thread-safe
        Microsoft::WRL::ComPtr<ID3D12Resource> CreateUploadBufferAndMap(const std::span<const std::byte>& Data, bool bSkipMap = false) const;

    private:

        Microsoft::WRL::ComPtr<ID3D12Device> D3dDevice {};
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> D3dDirectCommandQueue {};
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> D3dCopyCommandQueue {};
        Microsoft::WRL::ComPtr<IDXGIFactory6> DxgiFactory {};

        bool bShadersDebugEnabled {};

        bool SetupDebugLayer(UINT& DxgiFactoryFlags, const InitParams& Params);
        bool CreateDevice(UINT DxgiFactoryFlags);
        bool CreateCommandQueues();

    };
}
