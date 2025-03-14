#define WINDOWS_LEAN_AND_MEAN

#include <array>
#include <stdexcept>
#include <vector>
#include <iostream>

#include "directx/d3dx12.h"
#include <dxgi1_6.h>
#include <windows.h>
#include <d3dcompiler.h>

#include <DirectXMath.h>

std::vector<UINT8> GenerateTextureData()
{
    UINT TextureWidth = 256;
    UINT TexturePixelSize = 4;
    UINT TextureHeight = 256;

    const UINT rowPitch = TextureWidth * TexturePixelSize;
    const UINT cellPitch = rowPitch >> 3;        // The width of a cell in the checkboard texture.
    const UINT cellHeight = TextureWidth >> 3;    // The height of a cell in the checkerboard texture.
    const UINT textureSize = rowPitch * TextureHeight;

    std::vector<UINT8> data(textureSize);
    UINT8* pData = &data[0];

    for (UINT n = 0; n < textureSize; n += TexturePixelSize)
    {
        UINT x = n % rowPitch;
        UINT y = n / rowPitch;
        UINT i = x / cellPitch;
        UINT j = y / cellHeight;

        if (i % 2 == j % 2)
        {
            pData[n] = 0x00;        // R
            pData[n + 1] = 0x00;    // G
            pData[n + 2] = 0x00;    // B
            pData[n + 3] = 0xff;    // A
        }
        else
        {
            pData[n] = 0xff;        // R
            pData[n + 1] = 0xff;    // G
            pData[n + 2] = 0xff;    // B
            pData[n + 3] = 0xff;    // A
        }
    }

    return data;
}

struct Check
{
    const char* outMsg = "HRESULT failed";
};

bool operator >> (HRESULT Result, const Check& ck)
{
    if(FAILED(Result))
    {
        throw std::runtime_error(ck.outMsg);
    }

    return true;
}

HWND Window {};

LRESULT CALLBACK WindowProc(HWND Hwnd, UINT Message, WPARAM WParam, LPARAM LParam)
{
    switch (Message)
    {
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
    }

    return DefWindowProc(Hwnd, Message, WParam, LParam);
}

void CreateRenderWindow()
{
    WNDCLASSEX WindowClass = {};
    WindowClass.cbSize = sizeof(WNDCLASSEX);
    WindowClass.style = CS_HREDRAW | CS_VREDRAW;
    WindowClass.lpfnWndProc = WindowProc;
    WindowClass.hInstance = GetModuleHandle(nullptr);
    WindowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    WindowClass.lpszClassName = "HelloTriangle";
    RegisterClassEx(&WindowClass);

    RECT WindowRect = { 0, 0, 800, 600 };
    AdjustWindowRect(&WindowRect, WS_OVERLAPPEDWINDOW, FALSE);

    Window = CreateWindow(
        WindowClass.lpszClassName,
        "Test Directx 12",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        WindowRect.right - WindowRect.left,
        WindowRect.bottom - WindowRect.top,
        nullptr,
        nullptr,
        GetModuleHandle(nullptr),
        nullptr);

    ShowWindow(Window, SW_SHOWNORMAL);
}

constexpr int FrameCount = 2;

Microsoft::WRL::ComPtr<ID3D12Device> Device {};
Microsoft::WRL::ComPtr<ID3D12CommandQueue> CommandQueue {};
Microsoft::WRL::ComPtr<IDXGISwapChain4> SwapChain {};
UINT SwapChainFrameIndex {};
Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> DescriptorHeap {};
UINT DescriptorHeapIncrementSize {};
std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, FrameCount> RenderTargets {};
Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CommandAllocator {};
Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> CommandList {};
int FenceValue {};
Microsoft::WRL::ComPtr<ID3D12Fence> Fence {};
HANDLE FenceEvent {};

Microsoft::WRL::ComPtr<ID3D12Resource> DepthBuffer {};
Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> DepthBufferDescriptorHeap {};
CD3DX12_CPU_DESCRIPTOR_HANDLE DepthViewBufferHandle {};

void InitRender()
{
    UINT DxgiFactoryFlags = 0;

    // 1. Включаем Debug Layer для получения сообщений об ошибках
    // Enable the debug layer (requires the Graphics Tools "optional feature").

    #if defined(_DEBUG)
    {
        Microsoft::WRL::ComPtr<ID3D12Debug1> DebugController;

        D3D12GetDebugInterface(IID_PPV_ARGS(&DebugController)) >> Check{"Failed to create debug layer"};

        DebugController->EnableDebugLayer();
        DebugController->SetEnableGPUBasedValidation(true);

        // Enable additional debug layers.
        DxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
    }
    #endif

    // 2. Создаем Device который будем использовать для рендеринга
    // NOTE: Enabling the debug layer after device creation will invalidate the active device.

    // Получем факторку для итерации по адаптерам
    Microsoft::WRL::ComPtr<IDXGIFactory6> Factory;
    CreateDXGIFactory2(DxgiFactoryFlags, IID_PPV_ARGS(&Factory));

    // Итерируемся по адаптерам и находим подходящий
    // Создаем Device объект, позволяющий работать с выбранным адаптером.

    Microsoft::WRL::ComPtr<IDXGIAdapter> Adapter {};
    for(UINT AdapterIndex = 0; Factory->EnumAdapterByGpuPreference(
            AdapterIndex,
            DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
            IID_PPV_ARGS(&Adapter)) >> Check{"Failed to enumerate adapters"}; ++AdapterIndex)
    {
        if(SUCCEEDED(D3D12CreateDevice(
            Adapter.Get(),
            D3D_FEATURE_LEVEL_12_2,
            IID_PPV_ARGS(&Device)
        )))
        {
            break;
        }
    }

    if(Device == nullptr)
    {
        throw std::runtime_error("Failed to create device");
    }

    {
        DXGI_ADAPTER_DESC Desc {};
        Adapter->GetDesc(&Desc) >> Check{"Failed to get adapter description"};
        std::wcout << "Selected adapter: " << Desc.Description << std::endl;
    }


    // 3. Создаем Command Queue через который в последствии и будем отдавать команды на GPU

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&CommandQueue)) >> Check {"Failed to create command queue"};

    // 4. Создаем Swap Chain

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = FrameCount;
    swapChainDesc.Width = 0;
    swapChainDesc.Height = 0;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;

    Microsoft::WRL::ComPtr<IDXGISwapChain1> SwapChain1 {};
    Factory->CreateSwapChainForHwnd(
        CommandQueue.Get(),
        Window,
        &swapChainDesc,
        nullptr,
        nullptr,
        &SwapChain1
        ) >> Check {"Failed to create swap chain"};

    SwapChain1.As(&SwapChain) >> Check{};
    SwapChainFrameIndex = SwapChain->GetCurrentBackBufferIndex();

    // 5. Создаем descriptor heap

    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = FrameCount;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    Device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&DescriptorHeap)) >> Check {"Failed to create rtv descriptor heap"};

    // Размер шага внутри heap, шаг на два дескриптора это 2 * DescriptorHeapIncrementSize
    DescriptorHeapIncrementSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    // 6. Внутри descriptor heap создаем Render Target View

    CD3DX12_CPU_DESCRIPTOR_HANDLE CpuHandle { DescriptorHeap->GetCPUDescriptorHandleForHeapStart() };

    // Create a RTV for each frame.
    for (UINT n = 0; n < FrameCount; n++)
    {
        // Получаем Render Target из Swap Chain
        SwapChain->GetBuffer(n, IID_PPV_ARGS(&RenderTargets[n])) >> Check {"Failed to get buffer from swap chain"};

        // Создаем Render Target View в Descriptor Heap
        Device->CreateRenderTargetView(RenderTargets[n].Get(), nullptr, CpuHandle);

        // Сдвигаем указатель внутри Descriptor Heap на размер шага
        CpuHandle.Offset(1, DescriptorHeapIncrementSize);
    }

    // 7. Создаем аллокатор для команд
    Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&CommandAllocator)) >> Check {"Failed to create command allocator"};

    // 8. Создаем Command List
    Device->CreateCommandList(0,
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        CommandAllocator.Get(),
        nullptr,
        IID_PPV_ARGS(&CommandList)) >> Check{"Failed to create command list"};

    CommandList->Close() >> Check{"Failed to close command list"};

    // 9. Создаем Fence

    Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&Fence)) >> Check {"Failed to create fence"};
    FenceValue = 1;

    // Create an event handle to use for frame synchronization.
    FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (Fence == nullptr)
    {
        throw std::runtime_error("Can't create fence event");
    }

    // 10. Создаем Depth Buffer

    {
        CD3DX12_HEAP_PROPERTIES HeapProperties {D3D12_HEAP_TYPE_DEFAULT};
        CD3DX12_RESOURCE_DESC DepthBufferDesc = CD3DX12_RESOURCE_DESC::Tex2D(
            DXGI_FORMAT_D32_FLOAT,
            800,
            600,
            1,
            0,
            1,
            0,
            D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
        );

        D3D12_CLEAR_VALUE ClearValue {
            .Format = DXGI_FORMAT_D32_FLOAT,
            .DepthStencil = {1.0f, 0}
        };

        Device->CreateCommittedResource(
            &HeapProperties,
            D3D12_HEAP_FLAG_NONE,
            &DepthBufferDesc,
            D3D12_RESOURCE_STATE_DEPTH_WRITE,
            &ClearValue,
            IID_PPV_ARGS(&DepthBuffer)
        ) >> Check{"Failed to create depth buffer"};

        D3D12_DESCRIPTOR_HEAP_DESC desc = {
            .Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
            .NumDescriptors = 1,
        };

        Device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&DepthBufferDescriptorHeap)) >> Check {"Failed to create descriptor heap"};

        DepthViewBufferHandle = {
            DepthBufferDescriptorHeap->GetCPUDescriptorHandleForHeapStart()
        };

        Device->CreateDepthStencilView(DepthBuffer.Get(), nullptr, DepthViewBufferHandle);
    }
}

void WaitFence()
{
    CommandQueue->Signal(Fence.Get(), ++FenceValue) >> Check{"Failed to signal fence"};
    Fence->SetEventOnCompletion(FenceValue, FenceEvent) >> Check{"Failed to set fence event"};
    ::WaitForSingleObject(FenceEvent, INFINITE);
}

void DestroyRender()
{
    WaitFence();
    CloseHandle(FenceEvent);
}

void Update()
{

}

Microsoft::WRL::ComPtr<ID3D12Resource> VertexBuffer {};
D3D12_VERTEX_BUFFER_VIEW VertexBufferView {};
Microsoft::WRL::ComPtr<ID3D12RootSignature> RootSignature {};
Microsoft::WRL::ComPtr<ID3D12PipelineState> PipelineState {};
Microsoft::WRL::ComPtr<ID3DBlob> VertexShader {};
Microsoft::WRL::ComPtr<ID3DBlob> PixelShader {};
D3D12_RECT ScissorRect = CD3DX12_RECT(0, 0, LONG_MAX, LONG_MAX);
D3D12_VIEWPORT Viewport = CD3DX12_VIEWPORT(0.f, 0.f, 800.f, 600.f);

Microsoft::WRL::ComPtr<ID3D12Resource> IndexBuffer {};
D3D12_INDEX_BUFFER_VIEW IndexBufferView {};
int numIndices {};

Microsoft::WRL::ComPtr<ID3D12Resource> CubeFaceTexture;
Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvHeap {};

CD3DX12_CPU_DESCRIPTOR_HANDLE srvHeapHandle {};

CD3DX12_STATIC_SAMPLER_DESC StaticSampler { };

void LoadAssets()
{
    // 1. Определяем данные для загрузки

    struct Vertex
    {
        DirectX::XMFLOAT3 Position {};
        DirectX::XMFLOAT2 UV {};
    };

    const WORD IndexData[] = {
        0, 1, 2, 0, 2, 3,
        4, 6, 5, 4, 7, 6,
        4, 5, 1, 4, 1, 0,
        3, 2, 6, 3, 6, 7,
        1, 5, 6, 1, 6, 2,
        4, 0, 3, 4, 3, 7
    };

    numIndices = std::size(IndexData);

    Vertex vertexData[] = {
        { {-1.0f, -1.0f, -1.0f}, { 0.f, 0.f } }, // 0
        { {-1.0f,  1.0f, -1.0f}, { 0.f, 1.f } }, // 1
        { {1.0f,  1.0f, -1.0f}, { 1.f, 1.f } }, // 2
        { {1.0f, -1.0f, -1.0f}, { 1.f, 0.f } }, // 3
        { {-1.0f, -1.0f,  1.0f}, { 0.f, 1.f } }, // 4
        { {-1.0f,  1.0f,  1.0f}, { 0.f, 0.f } }, // 5
        { {1.0f,  1.0f,  1.0f}, { 1.f, 0.f } }, // 6
        { {1.0f, -1.0f,  1.0f}, { 1.f, 1.f } }  // 7
    };

    // 2. Создаем ресурс для хранения данных на GPU
    {
        auto HeapProps =  CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        auto ResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(vertexData));

        Device->CreateCommittedResource(
            &HeapProps,
            D3D12_HEAP_FLAG_NONE,
            &ResourceDesc,
            D3D12_RESOURCE_STATE_COMMON,
            nullptr,
            IID_PPV_ARGS(&VertexBuffer)
        ) >> Check{"Failed to create vertex buffer"};
    }

    {
        auto HeapProps =  CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        auto ResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(IndexData));

        Device->CreateCommittedResource(
            &HeapProps,
            D3D12_HEAP_FLAG_NONE,
            &ResourceDesc,
            D3D12_RESOURCE_STATE_COMMON,
            nullptr,
            IID_PPV_ARGS(&IndexBuffer)
        ) >> Check{"Failed to create index buffer"};
    }

    // 3. Создаем ресурс для загрузки данных в созданный ранее Vertex Buffer
    // Т.к. мы указали D3D12_HEAP_TYPE_DEFAULT, у нас нет возможности писать в буфер из CPU
    // Так что мы создаем еще один, и записываем данные через него.
    // PS. Можно было бы использовать один буфер с другим типом, но тогда страдала бы
    // производительность. Note: upload buffer удаляется, т.к. он не нужен после загрузки.

    Microsoft::WRL::ComPtr<ID3D12Resource> VertexUploadBuffer {};
    Microsoft::WRL::ComPtr<ID3D12Resource> IndexUploadBuffer {};

    {
        auto HeapProps =  CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        auto ResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(vertexData));

        Device->CreateCommittedResource(
                &HeapProps,
                D3D12_HEAP_FLAG_NONE,
                &ResourceDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&VertexUploadBuffer)
            ) >> Check{"Failed to create vertex upload buffer"};
    }

    {
        auto HeapProps =  CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        auto ResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(IndexData));

        Device->CreateCommittedResource(
                &HeapProps,
                D3D12_HEAP_FLAG_NONE,
                &ResourceDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&IndexUploadBuffer)
            ) >> Check{"Failed to create index upload buffer"};
    }

    // 4. Мапим GPU память на CPU память, пишем наши данные в буфер, не забываем Unmap.

    {
        void* PtrToLoad {};
        VertexUploadBuffer->Map(0, nullptr, &PtrToLoad) >> Check{"Failed to map vertex buffer to memory"};
        std::memcpy(PtrToLoad, vertexData, sizeof(vertexData));
        VertexUploadBuffer->Unmap(0, nullptr);
    }

    {
        void* PtrToLoad {};
        IndexUploadBuffer->Map(0, nullptr, &PtrToLoad) >> Check{"Failed to map index buffer to memory"};
        std::memcpy(PtrToLoad, IndexData, sizeof(IndexData));
        IndexUploadBuffer->Unmap(0, nullptr);
    }

    // 5. Как и обещали, копируем данные с upload buffer в обычный buffer.

    CommandAllocator->Reset() >> Check{"Failed to reset command allocator"};
    CommandList->Reset(CommandAllocator.Get(), nullptr) >> Check{"Failed to reset command list"};
    CommandList->CopyResource(VertexBuffer.Get(), VertexUploadBuffer.Get());
    CommandList->CopyResource(IndexBuffer.Get(), IndexUploadBuffer.Get());

    {
        const auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            VertexBuffer.Get(),
            D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
        CommandList->ResourceBarrier(1, &barrier);
    }

    {
        const auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            IndexBuffer.Get(),
            D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);
        CommandList->ResourceBarrier(1, &barrier);
    }

    // 6. Создаем View чтобы использовать буфер в будущем.

    VertexBufferView = {
        .BufferLocation = VertexBuffer->GetGPUVirtualAddress(),
        .SizeInBytes = sizeof(vertexData),
        .StrideInBytes = sizeof(Vertex),
    };

    IndexBufferView = {
        .BufferLocation = IndexBuffer->GetGPUVirtualAddress(),
        .SizeInBytes = sizeof(IndexData),
        .Format = DXGI_FORMAT_R16_UINT
    };

    Microsoft::WRL::ComPtr<ID3D12Resource> CubeFaceTextureUploadBuffer {};
    {
        auto textureData = GenerateTextureData();

        // Генерируем Mip Maps если нужны...

        CD3DX12_RESOURCE_DESC TexDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM,
            256,
            256,
            1,
            1
            );

        CD3DX12_HEAP_PROPERTIES HeapProps {D3D12_HEAP_TYPE_DEFAULT};

        Device->CreateCommittedResource(
            &HeapProps,
            D3D12_HEAP_FLAG_NONE,
            &TexDesc,
            D3D12_RESOURCE_STATE_COMMON,
            nullptr,
            IID_PPV_ARGS(&CubeFaceTexture)
        ) >> Check{"Failed to create texture"};

        CD3DX12_HEAP_PROPERTIES UploadProps {D3D12_HEAP_TYPE_UPLOAD};
        auto uploadBufferSize = GetRequiredIntermediateSize(CubeFaceTexture.Get(), 0, 1);
        auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);

        Device->CreateCommittedResource(
            &UploadProps,
            D3D12_HEAP_FLAG_NONE,
            &resDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&CubeFaceTextureUploadBuffer)
        ) >> Check{"Failed to create texture"};

        D3D12_SUBRESOURCE_DATA texturesubresData = {};
        texturesubresData.pData = &textureData[0];
        texturesubresData.RowPitch = 256 * 4;
        texturesubresData.SlicePitch = texturesubresData.RowPitch * 256;

        UpdateSubresources(CommandList.Get(), CubeFaceTexture.Get(), CubeFaceTextureUploadBuffer.Get(), 0, 0, 1, &texturesubresData);

        D3D12_DESCRIPTOR_HEAP_DESC HeapDesc = {
            .Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
            .NumDescriptors = 1,
            .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
        };

        Device->CreateDescriptorHeap(&HeapDesc, IID_PPV_ARGS(&srvHeap)) >> Check{"Failed to create srv descriptor heap"};

        srvHeapHandle = {srvHeap->GetCPUDescriptorHandleForHeapStart()};

        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Format = resDesc.Format;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;

        Device->CreateShaderResourceView(CubeFaceTexture.Get(), &srvDesc, srvHeapHandle);

        const auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            CubeFaceTexture.Get(),
            D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        CommandList->ResourceBarrier(1, &barrier);
    }

    // 7. Ожидаем выполнение копирования (завершения выполнения Command List)

    CommandList->Close() >> Check{"Failed to close command list"};

    ID3D12CommandList* CommandLists[] = { CommandList.Get() };
    CommandQueue->ExecuteCommandLists(std::size(CommandLists), CommandLists);

    WaitFence();

    // 8. Создаем Root Signature и PSO для отрисовки треугольника (радужного бурито, лол)

    {
        StaticSampler = {0, D3D12_FILTER_MIN_MAG_MIP_LINEAR};
    }

    {
        CD3DX12_ROOT_PARAMETER RootParameters[2]{};
        RootParameters[0].InitAsConstants(sizeof(DirectX::XMMATRIX) / sizeof(DWORD32), 0, 0, D3D12_SHADER_VISIBILITY_ALL);

        CD3DX12_DESCRIPTOR_RANGE descRange = {D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0};
        RootParameters[1].InitAsDescriptorTable(1, &descRange);

        CD3DX12_ROOT_SIGNATURE_DESC RootSignatureDesc = {};
        RootSignatureDesc.Init(std::size(RootParameters), RootParameters, 1, &StaticSampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        Microsoft::WRL::ComPtr<ID3DBlob> RootSignatureBlob {};
        Microsoft::WRL::ComPtr<ID3DBlob> RootSignatureErrorBlob {};
        D3D12SerializeRootSignature(&RootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0,
            &RootSignatureBlob, &RootSignatureErrorBlob) >> Check{"Failed to serialize root signature"};
        Device->CreateRootSignature(0, RootSignatureBlob->GetBufferPointer(),
            RootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&RootSignature)) >> Check{"Failed to create root signature"};
    }

    {
        D3D12_INPUT_ELEMENT_DESC InputLayout[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
        };

        #if defined(_DEBUG)
        // Enable better shader debugging with the graphics debugging tools.
        UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
        #else
        UINT compileFlags = 0;
        #endif

        // Note: вместо компиляции на лету, можно скомпилировать их заранее и получить .cso файлы,
        // которые можно загрузить используя D3DReadFileBlob

        {
            Microsoft::WRL::ComPtr<ID3DBlob> CompilationErrorBlob {};

            D3DCompileFromFile(L"S:/Dev/my/krendrr/Examples/DxTutor/shaders.hlsl",
                nullptr, nullptr, "VSMain", "vs_5_1",
                compileFlags, 0, &VertexShader, &CompilationErrorBlob);

            if(CompilationErrorBlob != nullptr)
            {
                std::string error( static_cast<char*>(CompilationErrorBlob->GetBufferPointer()), CompilationErrorBlob->GetBufferSize());
                std::cout << error << std::endl;
                throw std::runtime_error("Failed to compile vertex shader");
            }

            D3DCompileFromFile(L"S:/Dev/my/krendrr/Examples/DxTutor/shaders.hlsl",
                nullptr, nullptr, "PSMain", "ps_5_1",
                compileFlags, 0, &PixelShader, &CompilationErrorBlob);

            if(CompilationErrorBlob != nullptr)
            {
                std::string error( static_cast<char*>(CompilationErrorBlob->GetBufferPointer()), CompilationErrorBlob->GetBufferSize());
                std::cout << error << std::endl;
                throw std::runtime_error("Failed to compile pixel shader");
            }
        }

        D3D12_GRAPHICS_PIPELINE_STATE_DESC PsoDesc = {};
        PsoDesc.InputLayout = { InputLayout, std::size(InputLayout) };
        PsoDesc.pRootSignature = RootSignature.Get();
        PsoDesc.VS = CD3DX12_SHADER_BYTECODE(VertexShader.Get());
        PsoDesc.PS = CD3DX12_SHADER_BYTECODE(PixelShader.Get());
        PsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        PsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        PsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
        PsoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
        PsoDesc.SampleMask = UINT_MAX;
        PsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        PsoDesc.NumRenderTargets = 1;
        PsoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        PsoDesc.SampleDesc.Count = 1;

        Device->CreateGraphicsPipelineState(&PsoDesc, IID_PPV_ARGS(&PipelineState)) >> Check{"Failed to create graphics pipeline"};
    }
}

void Render()
{
    int CurrentSwapIndex = SwapChain->GetCurrentBackBufferIndex();
    auto& Rtv = RenderTargets[CurrentSwapIndex];

    CommandAllocator->Reset() >> Check{"Failed to reset command allocator"};
    CommandList->Reset(CommandAllocator.Get(), nullptr) >> Check{"Failed to reset command list"};

    auto BarrierToRender = CD3DX12_RESOURCE_BARRIER::Transition(Rtv.Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    CommandList->ResourceBarrier(1, &BarrierToRender);

    float color = std::sin(FenceValue * 0.01) + 0.5f;

    FLOAT ClearColor[4] = { color, color, color, 1.0f };
    CD3DX12_CPU_DESCRIPTOR_HANDLE RtvHandle { DescriptorHeap->GetCPUDescriptorHandleForHeapStart(), CurrentSwapIndex, DescriptorHeapIncrementSize };
    CommandList->ClearRenderTargetView(RtvHandle, ClearColor, 0, nullptr);
    CommandList->ClearDepthStencilView(DepthViewBufferHandle, D3D12_CLEAR_FLAG_DEPTH, 1.f, 0.f, 0, nullptr);

    CommandList->SetPipelineState(PipelineState.Get());
    CommandList->SetGraphicsRootSignature(RootSignature.Get());

    CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    CommandList->IASetVertexBuffers(0, 1, &VertexBufferView);
    CommandList->IASetIndexBuffer(&IndexBufferView);

    CommandList->RSSetViewports(1, &Viewport);
    CommandList->RSSetScissorRects(1, &ScissorRect);

    CommandList->SetDescriptorHeaps(1, srvHeap.GetAddressOf());
    CommandList->SetGraphicsRootDescriptorTable(1, srvHeap->GetGPUDescriptorHandleForHeapStart());

    CommandList->OMSetRenderTargets(1, &RtvHandle, true, &DepthViewBufferHandle);

    DirectX::XMMATRIX ViewMatrix = DirectX::XMMatrixLookAtLH(
        DirectX::XMVectorSet(0, 0, -5, 1),
        DirectX::XMVectorSet(0, 0, 0, 1),
        DirectX::XMVectorSet(0, 1, 0, 0));
    DirectX::XMMATRIX ProjectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(90.f), 800.f / 600.f, 0.1f, 10.f);

    {
        DirectX::XMMATRIX ModelMatrix = DirectX::XMMatrixRotationZ(FenceValue * 0.005f)
            * DirectX::XMMatrixRotationY(FenceValue * 0.007f)
            * DirectX::XMMatrixRotationX(FenceValue * 0.01f);
        DirectX::XMMATRIX MVP = DirectX::XMMatrixTranspose(ModelMatrix * ViewMatrix * ProjectionMatrix);

        CommandList->SetGraphicsRoot32BitConstants(0, sizeof(DirectX::XMMATRIX) / sizeof(DWORD32), &MVP, 0);

        CommandList->DrawIndexedInstanced(numIndices, 1, 0, 0, 0);
    }

    {
        DirectX::XMMATRIX ModelMatrix = DirectX::XMMatrixRotationZ(FenceValue * -0.005f)
            * DirectX::XMMatrixRotationY(FenceValue * -0.007f)
            * DirectX::XMMatrixRotationX(FenceValue * -0.01f);
        DirectX::XMMATRIX MVP = DirectX::XMMatrixTranspose(ModelMatrix * ViewMatrix * ProjectionMatrix);

        CommandList->SetGraphicsRoot32BitConstants(0, sizeof(DirectX::XMMATRIX) / sizeof(DWORD32), &MVP, 0);

        CommandList->DrawIndexedInstanced(numIndices, 1, 0, 0, 0);
    }

    auto BarrierToPresent = CD3DX12_RESOURCE_BARRIER::Transition(Rtv.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    CommandList->ResourceBarrier(1, &BarrierToPresent);

    CommandList->Close() >> Check{"Failed to close command list"};

    ID3D12CommandList* CommandLists[] = { CommandList.Get() };
    CommandQueue->ExecuteCommandLists(std::size(CommandLists), CommandLists);

    SwapChain->Present(0, 0) >> Check{"Failed to present swap chain"};

    WaitFence();
}

int main() try
{
    CreateRenderWindow();
    InitRender();
    LoadAssets();

    MSG Msg = {};
    bool Exit = false;
    while (!Exit)
    {
        if (PeekMessage(&Msg, nullptr, 0, 0, PM_REMOVE))
        {
            Exit = Msg.message == WM_QUIT;

            TranslateMessage(&Msg);
            DispatchMessage(&Msg);
        }
        else
        {
            Update();
            Render();
        }
    }

    DestroyRender();
}
catch(std::exception& ex)
{
    std::cerr << "Exception: " << ex.what() << "\n";
    return -1;
}
