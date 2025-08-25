#pragma once

#include <span>
#include <glad/gl.h>

namespace krendrr::Runtime::Renderer::Core
{
    class Mesh
    {
    public:

        struct BufferLayoutAttribute
        {
            GLuint Offset {};
            GLenum Type {};
            GLint Count {};
        };

        struct BufferLayout
        {
            GLint Stride {};

            std::span<const BufferLayoutAttribute> Attributes {};
        };

        Mesh();
        Mesh(const Mesh& Other) = delete;
        Mesh& operator=(const Mesh& Other) = delete;
        Mesh(Mesh&& Other) noexcept;
        Mesh& operator=(Mesh&& Other) noexcept;
        ~Mesh();

        void MoveFrom(Mesh& Other) noexcept;

        bool BindVAOAndDraw(GLuint Type = GL_TRIANGLES) const;

        template<typename T>
        static std::span<const std::byte> ContainerToBytes(const T& Container)
        {
            return std::span<const std::byte>(
                reinterpret_cast<const std::byte*>(&*std::begin(Container)), // use &* to support both pointer and iterator values
                std::size(Container) * sizeof(Container[0])
            );
        }

        bool Load(const BufferLayout& Layout, const std::span<const std::byte>& VertexData);

        bool LoadIndexed(const BufferLayout& Layout, const std::span<const std::byte>& VertexData, const std::span<const std::uint32_t>& IndexData);

        [[nodiscard]]
        bool IsLoaded() const;

        [[nodiscard]]
        bool IsUsingIndices() const;

        [[nodiscard]]
        std::int32_t GetPrimitivesCount() const;

        [[nodiscard]]
        std::ptrdiff_t GetPrimitivesOffset() const;

    private:

        GLuint VAO{};
        GLuint VBO{};
        GLuint EBO{};

        std::int32_t PrimitivesCount{};
        std::ptrdiff_t PrimitivesOffset{};

        bool CheckLoaded() const;

        void LoadAndBindVertexBuffer(const BufferLayout& Layout, const std::span<const std::byte>& VertexData);

    };
}

