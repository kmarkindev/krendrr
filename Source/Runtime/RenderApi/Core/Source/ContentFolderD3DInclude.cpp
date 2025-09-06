#include "Runtime/RenderApi/Core/ContentFolderD3DInclude.h"
#include <fstream>
#include <sstream>

namespace krendrr::Runtime::RenderApi::Core
{
    HRESULT ContentFolderD3dInclude::Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID* ppData, UINT* pBytes)
    {
        std::string FilePath = CONTENT_FOLDER_PATH + std::string(pFileName);

        std::ifstream Stream(FilePath);

        if (!Stream.is_open())
            return S_FALSE;

        std::stringstream StringStream {};
        StringStream << Stream.rdbuf();

        LoadedShaderSource = StringStream.str();

        *ppData = LoadedShaderSource.c_str();
        *pBytes = LoadedShaderSource.size();

        return S_OK;
    }

    HRESULT ContentFolderD3dInclude::Close(LPCVOID pData)
    {
        return S_OK;
    }
}
