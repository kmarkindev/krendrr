#pragma once

#include <span>
#include <wrl/client.h>
#include "Runtime/RenderApi/Core/RenderApi.h"
#include "d3dx12/d3dx12.h"

namespace krendrr::Runtime::RenderApi::Core
{
    class StaticMesh
    {
    public:

        StaticMesh() = default;
        StaticMesh(const StaticMesh& Other) = delete;
        StaticMesh& operator=(const StaticMesh& Other) = delete;
        StaticMesh(StaticMesh&& Other) noexcept = default;
        StaticMesh& operator=(StaticMesh&& Other) noexcept = default;
        ~StaticMesh() = default;

        struct MeshLoadOperation
        {
            bool bWasSuccessful {};
            Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferUploadBuffer {};
            Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferUploadBuffer {};

            bool WasSuccessful() const
            {
                return bWasSuccessful;
            }
        };

        /**
         * Creates upload heaps and fills provided command list with copy operations.
         * Caller need to execute the command list and keep upload buffers alive while copy is not finished.
         */
        MeshLoadOperation Load(const RenderApi& RenderApi, ID3D12GraphicsCommandList& CommandList, const std::span<const std::byte>& VertexData);

        /**
         * Creates upload heaps and fills provided command list with copy operations.
         * Caller need to execute the command list and keep upload buffers alive while copy is not finished.
         */
        MeshLoadOperation LoadIndexed(const RenderApi& RenderApi, ID3D12GraphicsCommandList& CommandList,
            const std::span<const std::byte>& VertexData, const std::span<const std::uint32_t>& IndexData);

        /**
         * This only checks if internal resource objects were created.
         * It doesn't check if async load operations were completed.
         */
        [[nodiscard]] bool IsLoaded() const;

        [[nodiscard]] bool IsUsingIndices() const;
        [[nodiscard]] std::int32_t GetPrimitivesCount() const;
        [[nodiscard]] std::ptrdiff_t GetPrimitivesOffset() const;

        [[nodiscard]] D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView() const;
        [[nodiscard]] D3D12_INDEX_BUFFER_VIEW GetIndexBufferView() const;

    private:

        Microsoft::WRL::ComPtr<ID3D12Resource> VertexBuffer {};
        Microsoft::WRL::ComPtr<ID3D12Resource> IndexBuffer {};

        std::uint32_t VertexBufferStride {};
        std::uint32_t PrimitivesCount{};
        std::uint32_t PrimitivesOffset{};

        [[nodiscard]] bool CheckLoaded() const;

        Microsoft::WRL::ComPtr<ID3D12Resource> LoadVertexBuffer(const RenderApi& RenderApi, ID3D12GraphicsCommandList& CommandList,
            const std::span<const std::byte>& VertexData);
    };
}

