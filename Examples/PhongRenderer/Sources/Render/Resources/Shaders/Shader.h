#pragma once

#include <d3dcommon.h>
#include <string_view>
#include <d3dx12/d3dx12_core.h>
#include <wrl/client.h>
#include "Sources/Render/Resources/RenderResource.h"

namespace kRendrr
{
    class Shader : public RenderResource
    {
    public:

        struct CompilationParams
        {
            std::string_view Target {};
            std::string_view EntryPoint { "Main" };
            bool bCompileDebug { false };
        };

        void InitializeFromMemory(std::string_view ShaderSource, const CompilationParams& Params = {});

        void InitializeFromFile(std::string_view ShaderFileName, const CompilationParams& Params = {});

        CD3DX12_SHADER_BYTECODE GetShaderByteCode() const;

    private:

        Microsoft::WRL::ComPtr<ID3DBlob> ShaderBlob {};

    };
}
