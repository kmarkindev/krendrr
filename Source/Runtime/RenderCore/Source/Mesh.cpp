#include "Render/Mesh.h"
#include <stdexcept>
#include "Utils/BytesArray.h"

namespace krendrr::Render
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

void Mesh::BindVAO() const
{
    CheckLoaded();

    glBindVertexArray(VAO);
}

void Mesh::Load(const std::span<const VertexBufferLayout>& Layout, const std::span<const std::byte>& VertexData)
{
    if(VAO > 0)
        throw std::runtime_error("Mesh already loaded");

    glCreateVertexArrays(1, &VAO);

    LoadAndBindVertexBuffer(Layout, VertexData);

    PrimitivesCount = static_cast<std::int32_t>(VertexData.size());
}

void Mesh::LoadIndexed(const std::span<const VertexBufferLayout>& Layout, const Utils::BytesArray& VertexData, const std::span<const std::uint32_t>& IndexData)
{
    if(VAO > 0)
        throw std::runtime_error("Mesh already loaded");

    glCreateVertexArrays(1, &VAO);

    LoadAndBindVertexBuffer(Layout, VertexData);

    glCreateBuffers(1, &EBO);
    glNamedBufferData(EBO, static_cast<GLsizeiptr>(IndexData.size_bytes()), IndexData.data(), GL_STATIC_DRAW);
    glVertexArrayElementBuffer(VAO, EBO);

    PrimitivesCount = static_cast<std::int32_t>(IndexData.size());
}

bool Mesh::IsLoaded() const
{
    return VAO != 0 && VBO != 0;
}

bool Mesh::IsUsingIndices() const
{
    CheckLoaded();

    return EBO != 0;
}

std::int32_t Mesh::GetPrimitivesCount() const
{
    CheckLoaded();

    return PrimitivesCount;
}

std::ptrdiff_t Mesh::GetPrimitivesOffset() const
{
    CheckLoaded();

    return PrimitivesOffset;
}

void Mesh::CheckLoaded() const
{
    if(!IsLoaded())
        throw std::runtime_error("Trying to use mesh which is not loaded");
}

void Mesh::LoadAndBindVertexBuffer(const std::span<const VertexBufferLayout>& Layout, const Utils::BytesArray& VertexData)
{
    glCreateBuffers(1, &VBO);

    glNamedBufferData(VBO, static_cast<GLsizeiptr>(VertexData.size_bytes()), VertexData.data(), GL_STATIC_DRAW);

    if(!Layout.empty())
        glVertexArrayVertexBuffer(VAO, 0, VBO, 0, Layout[0].Stride);

    for(int i = 0; i < Layout.size(); i++)
    {
        glEnableVertexArrayAttrib(VAO, i);
        glVertexArrayAttribFormat(VAO, i, Layout[i].Count, Layout[i].Type, GL_FALSE, Layout[i].Offset);
        glVertexArrayAttribBinding(VAO, i, 0);
    }
}

}
