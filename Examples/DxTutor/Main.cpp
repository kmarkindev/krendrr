import std;

import <windows.h>;
import <wrl.h>;
import <D3d12.h>;
import <dxgi1_6.h>;
import <d3dcompiler.h>;

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
    WNDCLASSEX WindowClass = { 0 };
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

void InitRender()
{
    UINT DxgiFactoryFlags = 0;

    // 1. Включаем Debug Layer для получения сообщений об ошибках
    // Enable the debug layer (requires the Graphics Tools "optional feature").

    #if defined(_DEBUG)
    {
        Microsoft::WRL::ComPtr<ID3D12Debug> DebugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&DebugController))))
        {
            DebugController->EnableDebugLayer();

            // Enable additional debug layers.
            DxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
        }
        else
        {
            throw std::runtime_error("Failed to create debug layer");
        }
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
    for(UINT AdapterIndex = 0; SUCCEEDED(Factory->EnumAdapterByGpuPreference(
            AdapterIndex,
            DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
            IID_PPV_ARGS(&Adapter))
        ); ++AdapterIndex)
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
        Adapter->GetDesc(&Desc);
        std::wcout << "Selected adapter: " << Desc.Description << std::endl;
    }


    // 3. Создаем Command Queue через который в последствии и будем отдавать команды на GPU

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    if(FAILED(Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&CommandQueue))))
    {
        throw std::runtime_error("Failed to create command queue");
    }

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
    if(FAILED(Factory->CreateSwapChainForHwnd(
        CommandQueue.Get(),
        Window,
        &swapChainDesc,
        nullptr,
        nullptr,
        &SwapChain1
        )))
    {
        throw std::runtime_error("Failed to create swap chain");
    }

    SwapChain1.As(&SwapChain);
    SwapChainFrameIndex = SwapChain->GetCurrentBackBufferIndex();

    // 5. Создаем descriptor heap

    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = FrameCount;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    Device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&DescriptorHeap));

    // Размер шага внутри heap, шаг на два дескриптора это 2 * DescriptorHeapIncrementSize
    DescriptorHeapIncrementSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    // 6. Внутри descriptor heap создаем Render Target View

    D3D12_CPU_DESCRIPTOR_HANDLE CpuHandle = DescriptorHeap->GetCPUDescriptorHandleForHeapStart();

    // Create a RTV for each frame.
    for (UINT n = 0; n < FrameCount; n++)
    {
        // Получаем Render Target из Swap Chain
        if(FAILED(SwapChain->GetBuffer(n, IID_PPV_ARGS(&RenderTargets[n]))))
        {
            throw std::runtime_error("Failed to get buffer from swap chain");
        }

        // Создаем Render Target View в Descriptor Heap
        Device->CreateRenderTargetView(RenderTargets[n].Get(), nullptr, CpuHandle);

        // Сдвигаем указатель внутри Descriptor Heap на размер шага
        CpuHandle.ptr += DescriptorHeapIncrementSize;
    }

    // 7. Создаем аллокатор для команд
    if(FAILED(Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&CommandAllocator))))
    {
        throw std::runtime_error("Failed to create command allocator");
    }
}

Microsoft::WRL::ComPtr<ID3D12RootSignature> RootSignature {};
Microsoft::WRL::ComPtr<ID3D12PipelineState> PipelineState {};

void LoadAssets()
{
    // 1. Создаем Root Signature

    {
        D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {
            0,
            nullptr,
            0,
            nullptr,
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
        };

        Microsoft::WRL::ComPtr<ID3DBlob> blob;
        Microsoft::WRL::ComPtr<ID3DBlob> error;
        if(FAILED(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, &error)))
        {
            throw std::runtime_error("Failed to serialize root signature");
        }

        if(FAILED(Device->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&RootSignature))))
        {
            throw std::runtime_error("Failed to create root signature");
        }
    }

    // 2. Компиляция шейдеров, с включением отладки для Debug сборки

    Microsoft::WRL::ComPtr<ID3DBlob> VertexShader;
    Microsoft::WRL::ComPtr<ID3DBlob> PixelShader;

    {
        UINT compileFlags {};

        #if defined(_DEBUG)
        compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
        #endif

        if(FAILED(D3DCompileFromFile(L"S:/Dev/my/krendrr/Examples/DxTutor/shaders.hlsl",
            nullptr,
            nullptr,
            "VSMain",
            "vs_5_0",
            compileFlags,
            0,
            &VertexShader,
            nullptr
            )))
        {
            throw std::runtime_error("Failed to compile vertex shader");
        }

        if(FAILED(D3DCompileFromFile(L"S:/Dev/my/krendrr/Examples/DxTutor/shaders.hlsl",
            nullptr,
            nullptr,
            "PSMain",
            "ps_5_0",
            compileFlags,
            0,
            &PixelShader,
            nullptr
            )))
        {
            throw std::runtime_error("Failed to compile pixel shader");
        }
    }

    // 3. Определение разметки vertex buffer

    D3D12_INPUT_ELEMENT_DESC InputElementDescs[] =
        {
            {
            "POSITION",
            0,
            DXGI_FORMAT_R32G32B32_FLOAT,
            0,
            0,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
            0
            },
        {
            "COLOR",
            0,
            DXGI_FORMAT_R32G32B32A32_FLOAT,
            0,
            12,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
            0
        }
        };

    // 4. Создание PSO

    {
        D3D12_RASTERIZER_DESC RasterizerState {};
        RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
        RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
        RasterizerState.FrontCounterClockwise = FALSE;
        RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
        RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
        RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
        RasterizerState.DepthClipEnable = TRUE;
        RasterizerState.MultisampleEnable = FALSE;
        RasterizerState.AntialiasedLineEnable = FALSE;
        RasterizerState.ForcedSampleCount = 0;
        RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

        D3D12_BLEND_DESC BlendState {};
        {
            BlendState.AlphaToCoverageEnable = FALSE;
            BlendState.IndependentBlendEnable = FALSE;
            const D3D12_RENDER_TARGET_BLEND_DESC DefaultRenderTargetBlendDesc =
            {
                FALSE,FALSE,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP,
                D3D12_COLOR_WRITE_ENABLE_ALL,
            };
            for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
                BlendState.RenderTarget[i] = DefaultRenderTargetBlendDesc;
        }

        D3D12_SHADER_BYTECODE VertexShaderByteCode = {};
        VertexShaderByteCode.pShaderBytecode = VertexShader->GetBufferPointer();
        VertexShaderByteCode.BytecodeLength = VertexShader->GetBufferSize();

        D3D12_SHADER_BYTECODE PixelShaderByteCode = {};
        PixelShaderByteCode.pShaderBytecode = PixelShader->GetBufferPointer();
        PixelShaderByteCode.BytecodeLength = PixelShader->GetBufferSize();

        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.InputLayout = { InputElementDescs, _countof(InputElementDescs) };
        psoDesc.pRootSignature = RootSignature.Get();
        psoDesc.VS = VertexShaderByteCode;
        psoDesc.PS = PixelShaderByteCode;
        psoDesc.RasterizerState = RasterizerState;
        psoDesc.BlendState = BlendState;
        psoDesc.DepthStencilState.DepthEnable = FALSE;
        psoDesc.DepthStencilState.StencilEnable = FALSE;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psoDesc.SampleDesc.Count = 1;

        if(FAILED(Device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&PipelineState))))
        {
            throw std::runtime_error("Failed to create PSO");
        }
    }
}

void DestroyRender()
{

}

void Update()
{

}

void Render()
{

}

int main() try
{
    CreateRenderWindow();
    InitRender();
    LoadAssets();

    MSG Msg = {};
    while (Msg.message != WM_QUIT)
    {
        if (PeekMessage(&Msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&Msg);
            DispatchMessage(&Msg);

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
