#pragma once

#include <d3dx12/d3dx12.h>

namespace krendrr::Runtime::RenderApi::Core
{
    class ContentFolderD3dInclude final : public ID3DInclude
    {
    public:

        HRESULT Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID* ppData, UINT* pBytes) override;

        HRESULT Close(LPCVOID pData) override;

    private:

        constexpr inline static const char* CONTENT_FOLDER_PATH = "../Content/";

        std::string LoadedShaderSource {};
    };
}
