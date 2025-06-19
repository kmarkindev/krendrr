#include "Mesh.h"
#include <stdexcept>

#include "BytesArray.h"

krendrr::render::Mesh::Mesh()
    : VAO{0}, VBO{0}, EBO{0}, PrimitivesCount{0}, PrimitivesOffset{0}
{
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

void krendrr::render::Mesh::BindVAO()
{
    glBindVertexArray(VAO);
}

void krendrr::render::Mesh::Load(const std::span<const VertexBufferLayout>& Layout, const std::span<const std::byte>& VertexData)
{
    glCreateVertexArrays(1, &VAO);
    LoadAndBindVertexBuffer(Layout, VertexData);

    PrimitivesCount = VertexData.size();

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void krendrr::render::Mesh::LoadIndexed(const std::span<const VertexBufferLayout>& Layout, const utils::BytesArray& VertexData, const std::span<const std::uint32_t>& IndexData)
{
    glCreateVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    LoadAndBindVertexBuffer(Layout, VertexData);

    glCreateBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(IndexData.size_bytes()), IndexData.data(), GL_STATIC_DRAW);

    PrimitivesCount = static_cast<std::int32_t>(IndexData.size());

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

bool krendrr::render::Mesh::IsLoaded() const
{
    return VAO != 0 && VBO != 0;
}

bool krendrr::render::Mesh::IsUsingIndices() const
{
    return EBO != 0;
}

std::int32_t krendrr::render::Mesh::GetPrimitivesCount() const
{
    return PrimitivesCount;
}

std::ptrdiff_t krendrr::render::Mesh::GetPrimitivesOffset() const
{
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
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(VertexData.size_bytes()), VertexData.data(), GL_STATIC_DRAW);

    for(int i = 0; i < Layout.size(); i++)
    {
        glVertexAttribPointer(i, Layout[i].Count, Layout[i].Type, GL_FALSE, Layout[i].Stride, reinterpret_cast<void*>(Layout[i].Offset));
        glEnableVertexAttribArray(i);
    }
}
