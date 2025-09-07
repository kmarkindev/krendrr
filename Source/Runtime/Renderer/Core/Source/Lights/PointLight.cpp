#include "Runtime/Renderer/Core/Lights/PointLight.h"

#include "Runtime/RenderApi/Core/ConstBufferHelper.h"

namespace krendrr::Runtime::Renderer::Core
{
    const glm::vec3& PointLight::GetPosition() const
    {
        return Position;
    }

    void PointLight::SetPosition(const glm::vec3& NewPosition)
    {
        Position = NewPosition;
    }

    const glm::vec3& PointLight::GetColor() const
    {
        return Color;
    }

    void PointLight::SetColor(const glm::vec3& NewColor)
    {
        Color = NewColor;
    }

    float PointLight::GetDistance() const
    {
        return Distance;
    }

    void PointLight::SetDistance(float NewDistance)
    {
        Distance = NewDistance;
    }

    float PointLight::GetAttenuationLinear() const
    {
        return AttenuationLinear;
    }

    void PointLight::SetAttenuationLinear(float NewAttenuationLinear)
    {
        AttenuationLinear = NewAttenuationLinear;
    }

    float PointLight::GetAttenuationQuad() const
    {
        return AttenuationQuad;
    }

    void PointLight::SetAttenuationQuad(float NewAttenuationQuad)
    {
        AttenuationQuad = NewAttenuationQuad;
    }

    float PointLight::GetAttenuationConstant() const
    {
        return AttenuationConstant;
    }

    bool PointLight::CastsShadows() const
    {
        return bCastsShadows;
    }

    void PointLight::SetCastsShadows(bool NewCastsShadows)
    {
        bCastsShadows = NewCastsShadows;
    }

    float PointLight::GetShadowFarDistance() const
    {
        return Distance + 1.f;
    }

    bool PointLight::HasShadowResources() const
    {
        return CastsShadows() && ShadowCubeMap != nullptr;
    }

    bool PointLight::CreateShadowCubeMapResource(const RenderApi::Core::RenderApi& RenderApi)
    {
        if (!CastsShadows())
        {
            // TODO: log error
            return false;
        }

        if (HasShadowResources())
            return true;

        // Create resource
        {
            CD3DX12_HEAP_PROPERTIES HeapProperties {D3D12_HEAP_TYPE_DEFAULT};
            CD3DX12_RESOURCE_DESC ResourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(
                CUBE_MAP_FORMAT,
                SHADOW_MAP_SIZE,
                SHADOW_MAP_SIZE,
                6,
                1,
                1,
                0,
                D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
            );

            D3D12_CLEAR_VALUE OptimizedClearValue = {
                .Format = CUBE_MAP_FORMAT,
                .Color = { 1.f, 1.f, 1.f, 1.f }
            };

            CHECKED(
                RenderApi.GetDevice()
                    ->CreateCommittedResource(
                        &HeapProperties,
                        D3D12_HEAP_FLAG_NONE,
                        &ResourceDesc,
                        D3D12_RESOURCE_STATE_GENERIC_READ,
                        &OptimizedClearValue,
                        IID_PPV_ARGS(&ShadowCubeMap)
                    ),
                "Can't create resource for shadow cube map"
            )
        }

        // Create SRV heap
        {
            D3D12_DESCRIPTOR_HEAP_DESC HeapDesc = {
                .Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
                .NumDescriptors = 1
            };

            CHECKED(
                RenderApi.GetDevice()
                    ->CreateDescriptorHeap(&HeapDesc, IID_PPV_ARGS(&ShadowCubeMapSrvHeap)),
                "Failed to create descriptor heap"
            )

            const D3D12_SHADER_RESOURCE_VIEW_DESC SrvDesc = CD3DX12_SHADER_RESOURCE_VIEW_DESC::TexCube(
                CUBE_MAP_FORMAT,
                1,
                0
            );

            RenderApi.GetDevice()
                ->CreateShaderResourceView(ShadowCubeMap.Get(), &SrvDesc, ShadowCubeMapSrvHeap->GetCPUDescriptorHandleForHeapStart());
        }

        // Create RTV heap
        {
            D3D12_DESCRIPTOR_HEAP_DESC HeapDesc = {
                .Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
                .NumDescriptors = 6
            };

            CHECKED(
                RenderApi.GetDevice()
                    ->CreateDescriptorHeap(&HeapDesc, IID_PPV_ARGS(&ShadowCubeMapFacesRtvHeap)),
                "Failed to create descriptor heap"
            )

            RtvIncrementSize = RenderApi.GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
            CD3DX12_CPU_DESCRIPTOR_HANDLE Handle {ShadowCubeMapFacesRtvHeap->GetCPUDescriptorHandleForHeapStart()};

            for (unsigned i = 0; i < 6; ++i)
            {
                D3D12_RENDER_TARGET_VIEW_DESC RtvDesc = {
                    .Format = CUBE_MAP_FORMAT,
                    .ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY,
                    .Texture2DArray = {
                        .MipSlice = 0,
                        .FirstArraySlice = i,
                        .ArraySize = 1,
                        .PlaneSlice = 0
                    }
                };

                RenderApi.GetDevice()
                    ->CreateRenderTargetView(ShadowCubeMap.Get(), &RtvDesc, Handle);

                Handle.Offset(1, RtvIncrementSize);
            }
        }

        // Create Depth resources
        {
            CD3DX12_HEAP_PROPERTIES HeapProperties {D3D12_HEAP_TYPE_DEFAULT};
            CD3DX12_RESOURCE_DESC ResourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(
                DEPTH_STENCIL_FORMAT,
                SHADOW_MAP_SIZE,
                SHADOW_MAP_SIZE,
                6,
                1,
                1,
                0,
                D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
            );

            D3D12_CLEAR_VALUE OptimizedClearValue = {
                .Format = DEPTH_STENCIL_FORMAT,
                .DepthStencil = {
                    .Depth = 1.f,
                    .Stencil = 0
                }
            };

            CHECKED(
                RenderApi.GetDevice()
                    ->CreateCommittedResource(
                        &HeapProperties,
                        D3D12_HEAP_FLAG_NONE,
                        &ResourceDesc,
                        D3D12_RESOURCE_STATE_DEPTH_WRITE,
                        &OptimizedClearValue,
                        IID_PPV_ARGS(&ShadowMapDepthStencil)
                    ),
                "Can't create resource for shadow cube map"
            )
        }

        // Create DSV heap
        {
            D3D12_DESCRIPTOR_HEAP_DESC HeapDesc = {
                .Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
                .NumDescriptors = 6
            };

            CHECKED(
                RenderApi.GetDevice()
                    ->CreateDescriptorHeap(&HeapDesc, IID_PPV_ARGS(&ShadowMapDepthDsvHeap)),
                "Failed to create descriptor heap"
            )

            DsvIncrementSize = RenderApi.GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
            CD3DX12_CPU_DESCRIPTOR_HANDLE Handle {ShadowMapDepthDsvHeap->GetCPUDescriptorHandleForHeapStart()};

            for (unsigned i = 0; i < 6; ++i)
            {
                D3D12_DEPTH_STENCIL_VIEW_DESC DsvDesc = {
                    .Format = DEPTH_STENCIL_FORMAT,
                    .ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY,
                    .Texture2DArray = {
                        .MipSlice = 0,
                        .FirstArraySlice = i,
                        .ArraySize = 1
                    }
                };

                RenderApi.GetDevice()
                    ->CreateDepthStencilView(ShadowMapDepthStencil.Get(), &DsvDesc, Handle);

                Handle.Offset(1, DsvIncrementSize);
            }
        }

        return true;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE PointLight::GetShadowCubeMapSrvHandle() const
    {
        return ShadowCubeMapSrvHeap->GetCPUDescriptorHandleForHeapStart();
    }

    D3D12_CPU_DESCRIPTOR_HANDLE PointLight::GetShadowCubeMapRtvHandle(int FaceIndex) const
    {
        return CD3DX12_CPU_DESCRIPTOR_HANDLE {
            ShadowCubeMapFacesRtvHeap->GetCPUDescriptorHandleForHeapStart(),
            FaceIndex,
            RtvIncrementSize
        };
    }

    D3D12_CPU_DESCRIPTOR_HANDLE PointLight::GetShadowCubeMapDsvHandle(int FaceIndex) const
    {
        return CD3DX12_CPU_DESCRIPTOR_HANDLE {
            ShadowMapDepthDsvHeap->GetCPUDescriptorHandleForHeapStart(),
            FaceIndex,
            DsvIncrementSize
        };
    }

    void PointLight::TransitionShadowCubeMapFromRenderTargetToRead(ID3D12GraphicsCommandList* CommandList)
    {
        CD3DX12_RESOURCE_BARRIER ResourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
            ShadowCubeMap.Get(),
            D3D12_RESOURCE_STATE_RENDER_TARGET,
            D3D12_RESOURCE_STATE_GENERIC_READ
        );

        CommandList->ResourceBarrier(1, &ResourceBarrier);
    }

    void PointLight::TransitionShadowCubeMapFromReadToRenderTarget(ID3D12GraphicsCommandList* CommandList)
    {
        CD3DX12_RESOURCE_BARRIER ResourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
            ShadowCubeMap.Get(),
            D3D12_RESOURCE_STATE_GENERIC_READ,
            D3D12_RESOURCE_STATE_RENDER_TARGET
        );

        CommandList->ResourceBarrier(1, &ResourceBarrier);
    }

    bool PointLight::UpdateConstantBuffer(const RenderApi::Core::RenderApi& RenderApi)
    {
        // Create buffer if not created
        if (ConstantBuffer == nullptr)
        {
            if (!InitializeConstantBuffer<ConstBuff_PointLight>(RenderApi, ConstantBuffer, CpuSrvHeap, L"Point Light Constant Buffer"))
                return false;
        }

        // Update buffer

        ConstBuff_PointLight* Buffer {};
        CHECKED_S(ConstantBuffer->Map(0, nullptr, reinterpret_cast<void**>(&Buffer)))

        *Buffer = {
            .Position = glm::vec4(Position, 0.f),
            .DiffuseColor = glm::vec4(Color, 0.f),
            .SpecularColor = Color,
            .Distance = Distance,
            .ShadowMapProjectionFarPlane = GetShadowFarDistance(),
            .AttenuationLinear = AttenuationLinear,
            .AttenuationQuad = AttenuationQuad,
            .AttenuationConstant = AttenuationConstant
        };

        ConstantBuffer->Unmap(0, nullptr);

        return true;
    }

    D3D12_GPU_VIRTUAL_ADDRESS PointLight::GetConstantBufferGpuHandle() const
    {
        if (CpuSrvHeap == nullptr)
            return {};

        return ConstantBuffer->GetGPUVirtualAddress();
    }
}
