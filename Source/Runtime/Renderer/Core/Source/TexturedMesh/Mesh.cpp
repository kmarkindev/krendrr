#include "Runtime/Renderer/Core/TexturedMesh/Mesh.h"
#include "Runtime/RenderApi/Core/ApiCallCheck.h"

namespace krendrr::Runtime::Renderer::Core
{
    Mesh::MeshLoadOperation Mesh::Load(const RenderApi::Core::RenderApi& RenderApi, ID3D12GraphicsCommandList& CommandList, const std::span<const std::byte>& VertexData)
    {
        if(IsLoaded())
        {
            // TODO: log "Mesh already loaded"
            return {};
        }

        Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferUpload = LoadVertexBuffer(RenderApi, CommandList, VertexData);
        if (!VertexBufferUpload)
        {
            // TODO: log error
            return {};
        }

        PrimitivesOffset = 0;
        VertexBufferStride = RenderApi.GetCommonMeshBufferLayout().Stride;
        PrimitivesCount = VertexData.size();

        return {
            .bWasSuccessful = true,
            .VertexBufferUploadBuffer = std::move(VertexBufferUpload),
            .IndexBufferUploadBuffer = {}
        };
    }

    Mesh::MeshLoadOperation Mesh::LoadIndexed(const RenderApi::Core::RenderApi& RenderApi, ID3D12GraphicsCommandList& CommandList, const std::span<const std::byte>& VertexData, const std::span<const std::uint32_t>& IndexData)
    {
        if(IsLoaded())
        {
            // TODO: log "Mesh already loaded"
            return {};
        }

        Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferUpload = LoadVertexBuffer(RenderApi, CommandList, VertexData);
        Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferUpload = RenderApi.CreateUploadBufferAndMap(RenderApi::Core::RenderApi::ContainerToBytes(IndexData));
        if (!VertexBufferUpload || !IndexBufferUpload)
        {
            // TODO: log error
            return {};
        }

        CD3DX12_RESOURCE_DESC VertexBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(IndexData.size_bytes());
        CD3DX12_HEAP_PROPERTIES HeapProperties {D3D12_HEAP_TYPE_DEFAULT};

        CHECKED(
            RenderApi.GetDevice()
                ->CreateCommittedResource(
                    &HeapProperties,
                    D3D12_HEAP_FLAG_NONE,
                    &VertexBufferDesc,
                    D3D12_RESOURCE_STATE_COMMON,
                    nullptr,
                    IID_PPV_ARGS(&IndexBuffer)
                ),
            "Failed to create index buffer"
        )

        CommandList.CopyResource(IndexBuffer.Get(), IndexBufferUpload.Get());

        IndexBufferUpload->SetName(L"Upload Index Buffer");
        IndexBuffer->SetName(L"Index Buffer");

        PrimitivesOffset = 0;
        PrimitivesCount = IndexData.size();
        VertexBufferStride = RenderApi.GetCommonMeshBufferLayout().Stride;

        return {
            .bWasSuccessful = true,
            .VertexBufferUploadBuffer = std::move(VertexBufferUpload),
            .IndexBufferUploadBuffer = std::move(IndexBufferUpload)
        };
    }

    Microsoft::WRL::ComPtr<ID3D12Resource> Mesh::LoadVertexBuffer(const RenderApi::Core::RenderApi& RenderApi, ID3D12GraphicsCommandList& CommandList, const std::span<const std::byte>& VertexData)
    {
        Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferUpload = RenderApi.CreateUploadBufferAndMap(VertexData);

        CD3DX12_RESOURCE_DESC VertexBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(VertexData.size_bytes());
        CD3DX12_HEAP_PROPERTIES HeapProperties {D3D12_HEAP_TYPE_DEFAULT};

        CHECKED(
            RenderApi.GetDevice()
                ->CreateCommittedResource(
                    &HeapProperties,
                    D3D12_HEAP_FLAG_NONE,
                    &VertexBufferDesc,
                    D3D12_RESOURCE_STATE_COMMON,
                    nullptr,
                    IID_PPV_ARGS(&VertexBuffer)
                ),
            "Failed to create vertex buffer"
        )

        CommandList.CopyResource(VertexBuffer.Get(), VertexBufferUpload.Get());

        VertexBufferUpload->SetName(L"Upload Vertex Buffer");
        VertexBuffer->SetName(L"Vertex Buffer");

        return VertexBufferUpload;
    }

    bool Mesh::IsLoaded() const
    {
        return VertexBuffer != nullptr;
    }

    bool Mesh::IsUsingIndices() const
    {
        if (!CheckLoaded())
            return false;

        return IndexBuffer != nullptr;
    }

    std::int32_t Mesh::GetPrimitivesCount() const
    {
        if (!CheckLoaded())
            return 0;

        return PrimitivesCount;
    }

    std::ptrdiff_t Mesh::GetPrimitivesOffset() const
    {
        if (!CheckLoaded())
            return 0;

        return PrimitivesOffset;
    }

    D3D12_VERTEX_BUFFER_VIEW Mesh::GetVertexBufferView() const
    {
        const D3D12_RESOURCE_DESC Desc = VertexBuffer->GetDesc();

        return {
            .BufferLocation = VertexBuffer->GetGPUVirtualAddress(),
            .SizeInBytes = static_cast<UINT>(Desc.Width),
            .StrideInBytes = VertexBufferStride,
        };
    }

    D3D12_INDEX_BUFFER_VIEW Mesh::GetIndexBufferView() const
    {
        const D3D12_RESOURCE_DESC Desc = IndexBuffer->GetDesc();

        return {
            .BufferLocation = IndexBuffer->GetGPUVirtualAddress(),
            .SizeInBytes = static_cast<UINT>(Desc.Width),
            .Format = DXGI_FORMAT_R32_UINT,
        };
    }

    bool Mesh::CheckLoaded() const
    {
        if(!IsLoaded())
        {
            // TODO: log error "Trying to use mesh which is not loaded"
            return false;
        }

        return true;
    }
}
