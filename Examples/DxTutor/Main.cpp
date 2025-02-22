import std;

import <windows.h>;
import <wrl.h>;
import <D3d12.h>;
import <dxgi1_6.h>;

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
        DXGI_ADAPTER_DESC Desc {};
        Adapter->GetDesc(&Desc);

        std::wcout << "Adapter Index = " << AdapterIndex << std::endl;
        std::wcout << Desc.Description << std::endl;
        std::wcout << Desc.DedicatedVideoMemory << std::endl;
        std::wcout << Desc.DedicatedSystemMemory << std::endl;
        std::wcout << Desc.SharedSystemMemory << std::endl;

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
        SwapChain->GetBuffer(n, IID_PPV_ARGS(&RenderTargets[n]));

        // Создаем Render Target View в Descriptor Heap
        Device->CreateRenderTargetView(RenderTargets[n].Get(), nullptr, CpuHandle);

        // Сдвигаем указатель внутри Descriptor Heap на размер шага
        CpuHandle.ptr += DescriptorHeapIncrementSize;
    }

    // 7. Создаем аллокатор для команд
    Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&CommandAllocator));
}

void LoadAssets()
{

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

int main()
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
