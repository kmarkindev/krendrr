#include "Mesh.h"
#include <stdexcept>

#include "BytesArray.h"

krendrr::render::Mesh::Mesh()
    : VAO{0}, VBO{0}, EBO{0}, PrimitivesCount{0}, PrimitivesOffset{0}
{
}

krendrr::render::Mesh::Mesh(Mesh&& Other) noexcept
{
    MoveFrom(Other);
}

krendrr::render::Mesh& krendrr::render::Mesh::operator=(Mesh&& Other) noexcept
{
    MoveFrom(Other);

    return *this;
}

krendrr::render::Mesh::~Mesh()
{
    if(VAO != 0)
        glDeleteVertexArrays(1, &VAO);

    if(VBO != 0)
        glDeleteBuffers(1, &VBO);

    if(EBO != 0)
        glDeleteBuffers(1, &EBO);
}

void krendrr::render::Mesh::MoveFrom(Mesh& Other) noexcept
{
    VAO = std::exchange(Other.VAO, 0);
    VBO = std::exchange(Other.VBO, 0);
    EBO = std::exchange(Other.EBO, 0);
    PrimitivesCount = Other.PrimitivesCount;
    PrimitivesOffset = Other.PrimitivesOffset;
}

void krendrr::render::Mesh::BindVAO() const
{
    CheckLoaded();

    glBindVertexArray(VAO);
}

void krendrr::render::Mesh::Load(const std::span<const VertexBufferLayout>& Layout, const std::span<const std::byte>& VertexData)
{
    glCreateVertexArrays(1, &VAO);

    LoadAndBindVertexBuffer(Layout, VertexData);

    PrimitivesCount = static_cast<std::int32_t>(VertexData.size());
}

void krendrr::render::Mesh::LoadIndexed(const std::span<const VertexBufferLayout>& Layout, const utils::BytesArray& VertexData, const std::span<const std::uint32_t>& IndexData)
{
    glCreateVertexArrays(1, &VAO);

    LoadAndBindVertexBuffer(Layout, VertexData);

    glCreateBuffers(1, &EBO);
    glNamedBufferData(EBO, static_cast<GLsizeiptr>(IndexData.size_bytes()), IndexData.data(), GL_STATIC_DRAW);
    glVertexArrayElementBuffer(VAO, EBO);

    PrimitivesCount = static_cast<std::int32_t>(IndexData.size());
}

bool krendrr::render::Mesh::IsLoaded() const
{
    return VAO != 0 && VBO != 0;
}

bool krendrr::render::Mesh::IsUsingIndices() const
{
    CheckLoaded();

    return EBO != 0;
}

std::int32_t krendrr::render::Mesh::GetPrimitivesCount() const
{
    CheckLoaded();

    return PrimitivesCount;
}

std::ptrdiff_t krendrr::render::Mesh::GetPrimitivesOffset() const
{
    CheckLoaded();

    return PrimitivesOffset;
}

void krendrr::render::Mesh::CheckLoaded() const
{
    if(!IsLoaded())
        throw std::runtime_error("Trying to use mesh which is not loaded");
}

void krendrr::render::Mesh::LoadAndBindVertexBuffer(const std::span<const VertexBufferLayout>& Layout, const utils::BytesArray& VertexData)
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
