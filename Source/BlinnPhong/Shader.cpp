#include "Shader.h"
#include <glad/gl.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace krendrr::render
{
    Shader::Shader()
        : ProgramId{0}
    {

    }

    bool Shader::IsLoaded() const
    {
        return ProgramId != 0;
    }

    void Shader::CheckLoaded() const
    {
        if(!IsLoaded())
            throw std::runtime_error("Trying to use unloaded shader");
    }

    void Shader::Use() const
    {
        CheckLoaded();
        glUseProgram(ProgramId);
    }

    void Shader::SetBool(const std::string_view& Name, bool Value)
    {
        CheckLoaded();
        glUniform1i(glGetUniformLocation(ProgramId, Name.data()), static_cast<int>(Value));
    }

    void Shader::SetInt(const std::string_view& Name, int Value)
    {
        CheckLoaded();
        glUniform1i(glGetUniformLocation(ProgramId, Name.data()), Value);
    }

    void Shader::SetFloat(const std::string_view& Name, float Value)
    {
        CheckLoaded();
        glUniform1f(glGetUniformLocation(ProgramId, Name.data()), Value);
    }

    void Shader::SetMatrix4(const std::string_view& Name, const glm::mat4& Value)
    {
        glUniformMatrix4fv(glGetUniformLocation(ProgramId, Name.data()), 1, GL_FALSE, glm::value_ptr(Value));
    }

    void Shader::SetMatrix3(const std::string_view& Name, const glm::mat3& Value)
    {
        glUniformMatrix3fv(glGetUniformLocation(ProgramId, Name.data()), 1, GL_FALSE, glm::value_ptr(Value));
    }

    void Shader::SetVec3(const std::string_view& Name, const glm::vec3& Value)
    {
        glUniform3f(glGetUniformLocation(ProgramId, Name.data()), Value.x, Value.y, Value.z);
    }

    void Shader::CheckCompileErrors(GLuint Shader, std::string_view Type)
    {
        int Success;
        std::stringstream ErrorStream;

        constexpr int LOG_BUFFER_SIZE = 1024;
        char InfoLog[LOG_BUFFER_SIZE];

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
            throw std::runtime_error(ErrorStream.str());
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

    Shader::~Shader()
    {
        if(ProgramId > 0)
            glDeleteProgram(ProgramId);
    }

    void Shader::Load(const std::string_view& VertexShaderFileName, const std::string_view& FragmentShaderFileName)
    {
        if(IsLoaded())
            throw std::runtime_error("Shader already loaded");

        if(!VertexShaderFileName.ends_with(".vert"))
            throw std::runtime_error(std::string{"Invalid vertex shader provided, file not ending with .vert, got \""} + VertexShaderFileName.data() + "\"");

        if(!FragmentShaderFileName.ends_with(".frag"))
            throw std::runtime_error(std::string{"Invalid fragment shader provided, file not ending with .frag, got \""} + FragmentShaderFileName.data() + "\"");

        std::ifstream FileStream {};
        FileStream.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        GLuint VertexShader {};
        GLuint FragmentShader {};

        try
        {
            // Load and compile vertex shader
            {
                std::string ShaderCode {};
                std::stringstream StringStream {};
                FileStream.open(VertexShaderFileName.data());
                StringStream << FileStream.rdbuf();
                ShaderCode = StringStream.str();

                const char* VertexSource = ShaderCode.c_str();
                VertexShader = glCreateShader(GL_VERTEX_SHADER);
                glShaderSource(VertexShader, 1, &VertexSource, nullptr);
                glCompileShader(VertexShader);
                CheckCompileErrors(VertexShader, "VERTEX");

                FileStream.close();
            }

            // Load and compile fragment shader
            {
                std::string ShaderCode {};
                std::stringstream StringStream {};
                FileStream.open(FragmentShaderFileName.data());
                StringStream << FileStream.rdbuf();
                ShaderCode = StringStream.str();

                const char* FragmentSource = ShaderCode.c_str();
                FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
                glShaderSource(FragmentShader, 1, &FragmentSource, nullptr);
                glCompileShader(FragmentShader);
                CheckCompileErrors(FragmentShader, "FRAGMENT");

                FileStream.close();
            }

            // Link shaders into program
            {
                ProgramId = glCreateProgram();
                glAttachShader(ProgramId, VertexShader);
                glAttachShader(ProgramId, FragmentShader);
                glLinkProgram(ProgramId);
                CheckCompileErrors(ProgramId, "PROGRAM");
            }

            glDeleteShader(VertexShader);
            glDeleteShader(FragmentShader);
        }
        catch (...)
        {
            if(VertexShader)
                glDeleteShader(VertexShader);

            if(FragmentShader)
                glDeleteShader(FragmentShader);

            if(ProgramId)
                glDeleteProgram(ProgramId);

            throw;
        }
    }
}
