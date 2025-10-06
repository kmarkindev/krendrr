#pragma once

#include <memory>
#include <array>
#include "Runtime/Renderer/Core/Renderer.h"
#include "Runtime/RenderApi/Core/Resources/StaticMesh.h"
#include "Runtime/Renderer/Core/Scene/Objects/TexturedStaticMesh.h"

namespace krendrr::Runtime::Renderer::Deferred
{
    class DeferredRenderer final : public Core::Renderer
    {
    public:

        bool Initialize(std::shared_ptr<RenderApi::Core::RenderApi> NewRenderApi, std::shared_ptr<tf::Executor> NewTfExecutor) override;

        bool Render(const Core::Scene* Scene, const std::span<Core::SceneView>& SceneViews) override;

        bool Shutdown() override;

    private:

        std::shared_ptr<RenderApi::Core::RenderApi> RenderApi {};
        std::shared_ptr<tf::Executor> TfExecutor {};

        std::shared_ptr<const RenderApi::Core::StaticMesh> FullscreenQuadMesh {};
        std::shared_ptr<const RenderApi::Core::StaticMesh> SphereMesh {};
        bool InitBasicMeshes();

        struct PrePostRenderData
        {
            Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CommandAllocator {};
            Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> CommandList {};
        };

        PrePostRenderData PrePostRenderData {};

        bool InitPrePostRender();
        bool PreRender(const Core::Scene* Scene, const Core::SceneView& SceneView);
        bool PostRender(const Core::SceneView& SceneView);

        Microsoft::WRL::ComPtr<ID3D12Fence> FrameFence {};
        uint64_t FrameFenceValue {};

        bool WaitDirectQueue();

        const std::array<CD3DX12_STATIC_SAMPLER_DESC, 2>& GetCommonStaticSamplers();

        // Used when there is no texture in TexturedMesh. It is filled with 0s
        Microsoft::WRL::ComPtr<ID3D12Resource> EmptyTexture {};
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CpuEmptyTextureHeap {};

        bool InitEmptyTexture();

        struct GBuffer
        {
            glm::ivec2 Size {-1, -1};

            Microsoft::WRL::ComPtr<ID3D12Resource> DiffuseTexture {};
            Microsoft::WRL::ComPtr<ID3D12Resource> WorldPositionTexture {};
            Microsoft::WRL::ComPtr<ID3D12Resource> WorldNormalTexture {};
            Microsoft::WRL::ComPtr<ID3D12Resource> MetallicTexture {};
            Microsoft::WRL::ComPtr<ID3D12Resource> RoughnessTexture {};
            Microsoft::WRL::ComPtr<ID3D12Resource> EmissiveTexture {};
            Microsoft::WRL::ComPtr<ID3D12Resource> DepthStencilTexture {};

            // Descriptors follow same order as textures are declared in this struct
            Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CpuRtvDescriptorHeap {};
            Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CpuSrvDescriptorHeap {};
            inline constexpr static unsigned TEXTURES_COUNT = 6;

            // Contains only one descriptor
            Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CpuDsvDescriptorHeap {};

            // Used for GBuffer barrier transitions
            Microsoft::WRL::ComPtr<ID3D12CommandAllocator> PresentToReadTransitionAllocator {};
            Microsoft::WRL::ComPtr<ID3D12CommandAllocator> ReadToPresentTransitionAllocator {};
            Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> TransitionCommandList {};
        };

        GBuffer GBuffer {};

        // Called every time we need to update GBuffer, so it has same size as scene view
        bool InitGBufferForView(const Core::SceneView& SceneView);
        bool TransitionGBufferFromRenderTargetToReadState();
        bool TransitionGBufferFromReadToRenderTargetState();

        // Make sure our C++ <-> HLSL types have same sizes
        static_assert(sizeof(float) == 4);
        static_assert(sizeof(glm::mat4) == sizeof(float) * 16);
        static_assert(sizeof(glm::mat3) == sizeof(float) * 9);
        static_assert(sizeof(glm::vec3) == sizeof(float) * 3);
        static_assert(sizeof(int) == 4);
        static_assert(sizeof(glm::ivec2) == sizeof(int) * 2);

        struct alignas(256) ConstBuff_Frame
        {
            glm::mat4 ViewMatrix {};

            glm::mat4 ProjectionMatrix {};

            std::uint32_t bHasAmbientLight {};
            glm::vec3 AmbientColor {};

            float AmbientIntensity {};
            glm::vec3 DirectionalColor {};

            std::uint32_t bHasDirectionalLight {};
            glm::vec3 DirectionalDir {};

            float DirectionalIntensity {};
            glm::vec3 CameraPosition {};

            glm::ivec2 ViewportSize {};
        };

        struct FrameData
        {
            Microsoft::WRL::ComPtr<ID3D12Resource> ConstantBuffer {};
            Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CpuSrvHeap {};
        };

        FrameData FrameData {};

        bool UpdateFrameDataConstantBuffer(const Core::Scene* Scene, const Core::SceneView& SceneView);
        bool UpdateTexturedMeshConstantBuffers(const Core::Scene* Scene);
        bool UpdatePointLightConstantBuffers(const Core::Scene* Scene);

        struct GeometryPassData
        {
            // Shaders are going to get textures in the same order as declared here
            constexpr inline static unsigned TEXTURED_MESH_TEXTURES_COUNT = 5;
            constexpr inline static const char* DIFFUSE_TEXTURE_NAME = "diffuse";
            constexpr inline static const char* METALLIC_TEXTURE_NAME = "metallic";
            constexpr inline static const char* ROUGHNESS_TEXTURE_NAME = "roughness";
            constexpr inline static const char* NORMAL_TEXTURE_NAME = "normal";
            constexpr inline static const char* EMISSIVE_TEXTURE_NAME = "emissive";

            Microsoft::WRL::ComPtr<ID3D12RootSignature> RootSignature {};
            Microsoft::WRL::ComPtr<ID3D12PipelineState> PipelineState {};

            constexpr inline static unsigned PARALLEL_DRAWS_COUNT_ALLOWED = 1000;
            Microsoft::WRL::ComPtr<ID3D12CommandAllocator> DrawCommandAllocator {};
            Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> DrawCommandList {};

            Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GpuDescriptorHeap {};
        };
        GeometryPassData GeometryPassData {};

        bool InitializeGeometryPass();
        bool GeometryPass(const Core::Scene* Scene, const Core::SceneView& SceneView);

        struct LightPassData
        {
            glm::ivec2 Size {-1, -1};

            constexpr static DXGI_FORMAT COLOR_TEXTURE_FORMAT = DXGI_FORMAT_R16G16B16A16_FLOAT;
            Microsoft::WRL::ComPtr<ID3D12Resource> ColorTexture {};
            Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CpuRtvHeap {};
            Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CpuSrvHeap {};

            Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GpuSrvRenderHeap {};

            Microsoft::WRL::ComPtr<ID3D12CommandAllocator> RenderTargetToReadTransitionAllocator {};
            Microsoft::WRL::ComPtr<ID3D12CommandAllocator> ReadToRenderTargetTransitionAllocator {};
            Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> TransitionCommandList {};
        };
        LightPassData LightPassData {};

        bool InitLightPass();
        bool PrepareLightPassData(const Core::SceneView& SceneView);
        bool TransitionLightPassFromRenderTargetToReadState();
        bool TransitionLightPassFromReadToRenderTargetState();

        struct AmbientDirectionalLightPassData
        {
            Microsoft::WRL::ComPtr<ID3D12PipelineState> PipelineState {};
            Microsoft::WRL::ComPtr<ID3D12RootSignature> RootSignature {};

            Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CommandAllocator {};
            Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> CommandList {};

            Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GpuDescriptorHeap {};
        };
        AmbientDirectionalLightPassData AmbientDirectionalLightPassData {};

        bool InitAmbientDirectionalLightPass();
        bool AmbientDirectionalLightPass(const Core::SceneView& SceneView);

        struct PointLightShadowCubeMapData
        {
            Microsoft::WRL::ComPtr<ID3D12PipelineState> PipelineState {};
            Microsoft::WRL::ComPtr<ID3D12RootSignature> RootSignature {};

            Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CommandAllocator {};
            Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> CommandList {};
        };
        PointLightShadowCubeMapData PointLightShadowCubeMapData {};

        bool InitPointLightShadowCubeMapPass();
        bool PointLightShadowCubeMapsPass(const Core::Scene* Scene);

        struct PointLightVolumePassData
        {
            // TODO: uncap this, by implementing static cubemaps
            // (baked before rendering starts. e.g. during renderer init or when point light is rendered for the first time)
            constexpr static inline unsigned MAX_DYNAMIC_POINT_LIGHTS_COUNT = 10;

            Microsoft::WRL::ComPtr<ID3D12PipelineState> StencilPipelineState {};
            Microsoft::WRL::ComPtr<ID3D12PipelineState> ColorPipelineState {};

            Microsoft::WRL::ComPtr<ID3D12RootSignature> RootSignature {};

            Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CommandAllocator {};
            Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> CommandList {};

            Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GpuDescriptorHeap {};
        };
        PointLightVolumePassData PointLightVolumePassData{};

        bool InitPointLightVolumePass();
        bool PointLightVolumesPass(const Core::Scene* Scene, const Core::SceneView& SceneView);

        struct PostProcessingPassData
        {
            Microsoft::WRL::ComPtr<ID3D12RootSignature> RootSignature {};
            Microsoft::WRL::ComPtr<ID3D12PipelineState> PipelineState {};

            Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CommandAllocator {};
            Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> CommandList {};

            Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GpuDescriptorHeap {};
        };
        PostProcessingPassData PostProcessingPassData {};

        bool InitPostProcessingPass();
        bool PostProcessingPass(const Core::SceneView& SceneView);
    };
}

