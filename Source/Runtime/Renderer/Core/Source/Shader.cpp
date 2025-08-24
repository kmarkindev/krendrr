#include "Runtime/Renderer/Core/Shader.h"

#include <sstream>
#include <iostream>
#include <glad/gl.h>
#include <fstream>
#include <glm/gtc/type_ptr.hpp>

namespace krendrr::Runtime::Renderer::Core
{
    Shader::Shader()
        : ProgramId{0}
    {

    }

    bool Shader::Use() const
    {
        if (!CheckValid())
            return false;

        glUseProgram(ProgramId);

        return true;
    }

    bool Shader::SetBool(const std::string_view& Name, bool Value)
    {
        if (!CheckValid())
            return false;

        glUniform1i(glGetUniformLocation(ProgramId, Name.data()), static_cast<int>(Value));

        return true;
    }

    bool Shader::SetInt(const std::string_view& Name, int Value)
    {
        if (!CheckValid())
            return false;

        glUniform1i(glGetUniformLocation(ProgramId, Name.data()), Value);

        return true;
    }

    bool Shader::SetFloat(const std::string_view& Name, float Value)
    {
        if (!CheckValid())
            return false;

        glUniform1f(glGetUniformLocation(ProgramId, Name.data()), Value);

        return true;
    }

    bool Shader::SetMatrix4(const std::string_view& Name, const glm::mat4& Value)
    {
        if (!CheckValid())
            return false;

        glUniformMatrix4fv(glGetUniformLocation(ProgramId, Name.data()), 1, GL_FALSE, glm::value_ptr(Value));

        return true;
    }

    bool Shader::SetVec2(const std::string_view& Name, const glm::vec2& Value)
    {
        if (!CheckValid())
            return false;

        glUniform2f(glGetUniformLocation(ProgramId, Name.data()), Value.x, Value.y);

        return true;
    }

    bool Shader::SetMatrix3(const std::string_view& Name, const glm::mat3& Value)
    {
        if (!CheckValid())
            return false;

        glUniformMatrix3fv(glGetUniformLocation(ProgramId, Name.data()), 1, GL_FALSE, glm::value_ptr(Value));

        return true;
    }

    bool Shader::SetVec3(const std::string_view& Name, const glm::vec3& Value)
    {
        if (!CheckValid())
            return false;

        glUniform3f(glGetUniformLocation(ProgramId, Name.data()), Value.x, Value.y, Value.z);

        return true;
    }

    bool Shader::CheckValid() const
    {
        if(!IsValid())
        {
            // TODO: log "Trying to use invalid shader"
            return false;
        }

        return true;
    }

    bool Shader::CheckCompileErrors(GLuint Shader, std::string_view Type)
    {
        int Success {};
        std::stringstream ErrorStream {};

        constexpr int LOG_BUFFER_SIZE = 1024;
        char InfoLog[LOG_BUFFER_SIZE] {};

        if (Type != "PROGRAM")
        {
            glGetShaderiv(Shader, GL_COMPILE_STATUS, &Success);
            if (!Success)
            {
                glGetShaderInfoLog(Shader, LOG_BUFFER_SIZE, nullptr, InfoLog);
                ErrorStream << "Shader compilation error: "
                    << Type
                    << "\n"
                    << InfoLog
                    << std::endl;
            }
        }
        else
        {
            glGetProgramiv(Shader, GL_LINK_STATUS, &Success);
            if (!Success)
            {
                glGetProgramInfoLog(Shader, LOG_BUFFER_SIZE, nullptr, InfoLog);
                ErrorStream << "Program linking error: "
                    << Type
                    << "\n"
                    << InfoLog
                    << std::endl;
            }
        }

        if (ErrorStream.tellp() != std::streampos(0))
        {
            // TODO: log error ErrorStream.str()
            return false;
        }

        return true;
    }

    Shader::Shader(Shader&& Other) noexcept
    {
        MoveFrom(Other);
    }

    Shader& Shader::operator=(Shader&& Other) noexcept
    {
        MoveFrom(Other);
        return *this;
    }

    void Shader::MoveFrom(Shader& Other) noexcept
    {
        ProgramId = std::exchange(Other.ProgramId, 0);
    }

    bool Shader::Attach(ShaderType Type, std::string_view ShaderFileName)
    {
        if (IsValid())
        {
            // TODO: log already linked
            return false;
        }

        GLuint* ShaderIdPtr {};
        GLuint ShaderType {};
        std::string_view ShaderErrorType {};
        switch (Type)
        {
            case ShaderType::Vertex:
                ShaderIdPtr = &AttachedVertexShader;
                ShaderErrorType = "VERTEX";
                ShaderType = GL_VERTEX_SHADER;

                if(!ShaderFileName.ends_with(".vert"))
                {
                    // TODO: log error std::string{"Invalid vertex shader provided, file not ending with .vert, got \""} + ShaderFileName.data() + "\""
                    return false;
                }

                break;
            case ShaderType::Geometry:
                ShaderIdPtr = &AttachedGeometryShader;
                ShaderErrorType = "GEOMETRY";
                ShaderType = GL_GEOMETRY_SHADER;

                if(!ShaderFileName.ends_with(".geom"))
                {
                    // TODO: log error std::string{"Invalid vertex shader provided, file not ending with .geom, got \""} + ShaderFileName.data() + "\""
                    return false;
                }

                break;
            case ShaderType::Fragment:
                ShaderIdPtr = &AttachedFragmentShader;
                ShaderErrorType = "FRAGMENT";
                ShaderType = GL_FRAGMENT_SHADER;

                if(!ShaderFileName.ends_with(".frag"))
                {
                    // TODO: log error std::string{"Invalid vertex shader provided, file not ending with .frag, got \""} + ShaderFileName.data() + "\""
                    return false;
                }

                break;
            default:
                assert(false);
                return false;
        }

        if (*ShaderIdPtr > 0)
        {
            // TODO: log error already attached
            return false;
        }

        std::string ShaderCode {};
        std::stringstream StringStream {};
        std::ifstream FileStream {};

        FileStream.open(ShaderFileName.data());

        if (!FileStream.is_open())
        {
            // TODO: log error cant open shader file
            return false;
        }

        StringStream << FileStream.rdbuf();
        ShaderCode = StringStream.str();
        const char* ShaderSourcePtr = ShaderCode.c_str();

        *ShaderIdPtr = glCreateShader(ShaderType);
        glShaderSource(*ShaderIdPtr, 1, &ShaderSourcePtr, nullptr);
        glCompileShader(*ShaderIdPtr);

        return CheckCompileErrors(*ShaderIdPtr, ShaderErrorType);
    }

    bool Shader::Link()
    {
        if (IsValid())
        {
            // TODO: log shader already linked
            return false;
        }

        if (AttachedVertexShader == 0 || AttachedFragmentShader == 0)
        {
            // TODO: log error required shader are not attached
            return false;
        }

        ProgramId = glCreateProgram();
        glAttachShader(ProgramId, AttachedVertexShader);
        glAttachShader(ProgramId, AttachedFragmentShader);

        if (AttachedGeometryShader > 0)
            glAttachShader(ProgramId, AttachedGeometryShader);

        glLinkProgram(ProgramId);

        const bool bHasErrors = CheckCompileErrors(ProgramId, "PROGRAM");

        if (AttachedVertexShader > 0)
        {
            glDeleteShader(AttachedVertexShader);
            AttachedVertexShader = 0;
        }
        if (AttachedGeometryShader > 0)
        {
            glDeleteShader(AttachedGeometryShader);
            AttachedGeometryShader = 0;
        }
        if (AttachedFragmentShader > 0)
        {
            glDeleteShader(AttachedFragmentShader);
            AttachedFragmentShader = 0;
        }

        return bHasErrors;
    }

    bool Shader::AttachLinkAll(const std::span<ShaderToLink>& ShadersToLink)
    {
        for (ShaderToLink& ToLink : ShadersToLink)
        {
            if (!ToLink.Shader.Attach(ShaderType::Vertex, ToLink.VertexShader))
                return false;

            if (!ToLink.Shader.Attach(ShaderType::Fragment, ToLink.FragmentShader))
                return false;

            if (!ToLink.GeometryShader.empty())
            {
                if (!ToLink.Shader.Attach(ShaderType::Geometry, ToLink.GeometryShader))
                    return false;
            }

            if (!ToLink.Shader.Link())
                return false;
        }

        return true;
    }

    bool Shader::IsValid() const
    {
        return ProgramId != 0;
    }

    Shader::~Shader()
    {
        if(ProgramId > 0)
            glDeleteProgram(ProgramId);

        if (AttachedVertexShader > 0)
            glDeleteShader(AttachedVertexShader);

        if (AttachedFragmentShader > 0)
            glDeleteShader(AttachedFragmentShader);

        if (AttachedGeometryShader > 0)
            glDeleteShader(AttachedGeometryShader);
    }
}
