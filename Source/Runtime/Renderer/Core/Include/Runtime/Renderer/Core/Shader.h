#pragma once

#include <span>
#include <glad/gl.h>
#include <string_view>
#include "glm/fwd.hpp"

namespace krendrr::Runtime::Renderer::Core
{
    class Shader
    {
    public:

        Shader();
        Shader(const Shader& Other) = delete;
        Shader& operator=(const Shader& Other) = delete;
        Shader(Shader&& Other) noexcept;
        Shader& operator=(Shader&& Other) noexcept;
        ~Shader();

        void MoveFrom(Shader& Other) noexcept;

        enum class ShaderType
        {
            Vertex,
            Geometry,
            Fragment
        };

        bool Attach(ShaderType Type, std::string_view ShaderFileName);
        bool Link();

        struct ShaderToLink
        {
            Shader& Shader;
            std::string_view VertexShader;
            std::string_view GeometryShader;
            std::string_view FragmentShader;
        };

        static bool AttachLinkAll(const std::span<ShaderToLink>& ShadersToLink);

        [[nodiscard]] bool IsValid() const;

        bool Use() const;

        bool SetBool(const std::string_view& Name, bool Value);

        bool SetInt(const std::string_view& Name, int Value);

        bool SetFloat(const std::string_view& Name, float Value);

        bool SetMatrix3(const std::string_view& Name, const glm::mat3& Value);

        bool SetMatrix4(const std::string_view& Name, const glm::mat4& Value);

        bool SetVec2(const std::string_view& Name, const glm::vec2& Value);

        bool SetVec3(const std::string_view& Name, const glm::vec3& Value);

    private:

        GLuint AttachedVertexShader {};
        GLuint AttachedGeometryShader {};
        GLuint AttachedFragmentShader {};

        GLuint ProgramId;

        bool CheckValid() const;

        static bool CheckCompileErrors(GLuint Shader, std::string_view Type);

    };
}
