#include "Shader.h"
#include <d3dcompiler.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include "Sources/Utils/HResultCheck.h"

namespace kRendrr
{
    void Shader::InitializeFromMemory(std::string_view ShaderSource, const CompilationParams& Params)
    {
        Microsoft::WRL::ComPtr<ID3DBlob> CompilationErrorBlob {};

        UINT Flags = 0;

        if(Params.bCompileDebug)
        {
            Flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
        }

        HRESULT Result = D3DCompile(
            ShaderSource.data(),
            ShaderSource.size(),
            nullptr,
            nullptr,
            nullptr,
            Params.EntryPoint.data(),
            Params.Target.data(),
            Flags,
            0,
            &ShaderBlob,
            &CompilationErrorBlob
        );

        std::string_view ErrorMsg {};

        if(CompilationErrorBlob != nullptr)
        {
            ErrorMsg = {
                static_cast<char*>(CompilationErrorBlob->GetBufferPointer()),
                CompilationErrorBlob->GetBufferSize()
            };
        }

        Result >> HResultCheck { ErrorMsg };

        // We have not failed compilation, but have some warnings/errors, so print them
        if(!ErrorMsg.empty())
        {
            std::cerr << ErrorMsg << '\n';
        }
    }

    void Shader::InitializeFromFile(std::string_view ShaderFileName, const CompilationParams& Params)
    {
        std::ifstream File(ShaderFileName.data());
        std::stringstream StringStream {};
        StringStream << File.rdbuf();

        InitializeFromMemory(StringStream.str(), Params);
    }

    CD3DX12_SHADER_BYTECODE Shader::GetShaderByteCode() const
    {
        return {ShaderBlob.Get()};
    }
}
