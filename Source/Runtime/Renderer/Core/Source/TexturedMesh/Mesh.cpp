#include "Runtime/Renderer/Core/TexturedMesh/Mesh.h"

namespace krendrr::Runtime::Renderer::Core
{
    Mesh::Mesh()
    {
    }

    Mesh::Mesh(Mesh&& Other) noexcept
    {
        MoveFrom(Other);
    }

    Mesh& Mesh::operator=(Mesh&& Other) noexcept
    {
        MoveFrom(Other);

        return *this;
    }

    Mesh::~Mesh()
    {
        if(VAO != 0)
            glDeleteVertexArrays(1, &VAO);

        if(VBO != 0)
            glDeleteBuffers(1, &VBO);

        if(EBO != 0)
            glDeleteBuffers(1, &EBO);
    }

    void Mesh::MoveFrom(Mesh& Other) noexcept
    {
        VAO = std::exchange(Other.VAO, 0);
        VBO = std::exchange(Other.VBO, 0);
        EBO = std::exchange(Other.EBO, 0);
        PrimitivesCount = Other.PrimitivesCount;
        PrimitivesOffset = Other.PrimitivesOffset;
    }

    bool Mesh::BindVAOAndDraw(GLuint Type) const
    {
        if (!CheckLoaded())
            return false;

        glBindVertexArray(VAO);

        if (IsUsingIndices())
            glDrawElements(Type, GetPrimitivesCount(), GL_UNSIGNED_INT, reinterpret_cast<void*>(GetPrimitivesOffset()));
        else
            glDrawArrays(Type, GetPrimitivesOffset(), GetPrimitivesCount());

        return true;
    }

    bool Mesh::Load(const BufferLayout& Layout, const std::span<const std::byte>& VertexData)
    {
        if(IsLoaded())
        {
            // TODO: log "Mesh already loaded"
            return false;
        }

        glCreateVertexArrays(1, &VAO);

        LoadAndBindVertexBuffer(Layout, VertexData);

        PrimitivesCount = static_cast<std::int32_t>(VertexData.size());

        return true;
    }

    bool Mesh::LoadIndexed(const BufferLayout& Layout, const std::span<const std::byte>& VertexData, const std::span<const std::uint32_t>& IndexData)
    {
        if(IsLoaded())
        {
            // TODO: log "Mesh already loaded"
            return false;
        }

        glCreateVertexArrays(1, &VAO);

        LoadAndBindVertexBuffer(Layout, VertexData);

        glCreateBuffers(1, &EBO);
        glNamedBufferData(EBO, static_cast<GLsizeiptr>(IndexData.size_bytes()), IndexData.data(), GL_STATIC_DRAW);
        glVertexArrayElementBuffer(VAO, EBO);

        PrimitivesCount = static_cast<std::int32_t>(IndexData.size());

        return true;
    }

    bool Mesh::IsLoaded() const
    {
        return VAO != 0 && VBO != 0;
    }

    bool Mesh::IsUsingIndices() const
    {
        if (!CheckLoaded())
            return false;

        return EBO != 0;
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

    bool Mesh::CheckLoaded() const
    {
        if(!IsLoaded())
        {
            // TODO: log error "Trying to use mesh which is not loaded"
            return false;
        }

        return true;
    }

    void Mesh::LoadAndBindVertexBuffer(const BufferLayout& Layout, const std::span<const std::byte>& VertexData)
    {
        glCreateBuffers(1, &VBO);

        glNamedBufferData(VBO, static_cast<GLsizeiptr>(VertexData.size_bytes()), VertexData.data(), GL_STATIC_DRAW);

        glVertexArrayVertexBuffer(VAO, 0, VBO, 0, Layout.Stride);

        for(int i = 0; i < Layout.Attributes.size(); i++)
        {
            const auto& [Offset, Type, Count] = Layout.Attributes[i];

            glEnableVertexArrayAttrib(VAO, i);
            glVertexArrayAttribFormat(VAO, i, Count, Type, GL_FALSE, Offset);
            glVertexArrayAttribBinding(VAO, i, 0);
        }
    }
}
