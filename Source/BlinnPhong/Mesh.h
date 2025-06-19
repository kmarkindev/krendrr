#pragma once

#include <span>
#include <glad/gl.h>
#include "BytesArray.h"

namespace krendrr::render
{

class Mesh 
{
public:

    struct VertexBufferLayout
    {
        GLint Stride;
        std::uintptr_t Offset;
        GLenum Type;
        GLint Count;
    };

    Mesh();
    ~Mesh();

    void BindVAO();

    void Load(const std::span<const VertexBufferLayout>& Layout, const std::span<const std::byte>& VertexData);

    void LoadIndexed(const std::span<const VertexBufferLayout>& Layout, const utils::BytesArray& VertexData, const std::span<const std::uint32_t>& IndexData);

    [[nodiscard]]
    bool IsLoaded() const;

    [[nodiscard]]
    bool IsUsingIndices() const;

    [[nodiscard]]
    std::int32_t GetPrimitivesCount() const;

    [[nodiscard]]
    std::ptrdiff_t GetPrimitivesOffset() const;

private:

    GLuint VAO;
    GLuint VBO;
    GLuint EBO;

    std::uint32_t PrimitivesCount;
    std::ptrdiff_t PrimitivesOffset;

    void CheckLoaded() const;

    void LoadAndBindVertexBuffer(const std::span<const VertexBufferLayout>& Layout, const utils::BytesArray& VertexData);

};

}
