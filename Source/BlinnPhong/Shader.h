#pragma once

#include <string_view>
#include <glad/gl.h>
#include <glm/fwd.hpp>

namespace krendrr::render
{
    class Shader final
    {
    public:

        Shader();
        ~Shader();

        void Load(const std::string_view& VertexShaderFileName, const std::string_view& FragmentShaderFileName);

        [[nodiscard]]
        bool IsLoaded() const;

        void Use() const;

        void SetBool(const std::string_view& Name, bool Value);

        void SetInt(const std::string_view& Name, int Value);

        void SetFloat(const std::string_view& Name, float Value);

        void SetMatrix4(const std::string_view& Name, const glm::mat4& Value);

        void SetVec3(const std::string_view& Name, const glm::vec3& Value);

    private:

        GLuint ProgramId;

        void CheckLoaded() const;

        static void CheckCompileErrors(GLuint Shader, std::string_view Type);

    };
}
