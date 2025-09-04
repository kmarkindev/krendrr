#include "Runtime/Application/Windows/WindowsWindow.h"
#include <dxgi1_2.h>
#include "Runtime/Application/Core/Internal/WindowAllocator.h"
#include "SDL3/SDL_stdinc.h"
#include "Runtime/RenderApi/Core/ApiCallCheck.h"
#include "SDL3/SDL_mouse.h"

IMPLEMENT_WINDOW_ALLOCATOR(krendrr::Runtime::Application::Windows::WindowsWindow)

namespace krendrr::Runtime::Application::Windows
{

    bool WindowsWindow::Initialize(const std::shared_ptr<RenderApi::Core::RenderApi>& NewRenderApi, const InitializeParams& Params)
    {
        if (IsValid())
        {
            // TODO: add error log
            return false;
        }

        RenderApi = NewRenderApi;

        Window = SDL_CreateWindow(
            Params.Title.data(),
            Params.Size.x,
            Params.Size.y,
            SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_INPUT_FOCUS
        );

        SDL_PropertiesID WindowProps = SDL_GetWindowProperties(Window);
        SDL_SetPointerProperty(WindowProps, SLD_WINDOW_OBJECT_PROPERTY, this);

        WindowHandle = static_cast<HWND>(SDL_GetPointerProperty(WindowProps, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr));

        if (Window == nullptr || WindowHandle == nullptr)
        {
            // TODO: add error log
            return false;
        }

        SDL_HideCursor();
        SDL_SetWindowRelativeMouseMode(Window, true);

        if (!CreateUpdateSwapChain())
        {
            // TODO: log error
            return false;
        }

        if (!CreateUpdateRenderTargets())
        {
            // TODO: log error
            return false;
        }

        // TODO: add success log

        return true;
    }

    bool WindowsWindow::Destroy()
    {
        if (!IsValid())
        {
            // TODO: add error log
            return false;
        }

        SDL_DestroyWindow(Window);
        Window = nullptr;

        // TODO: add success log

        return true;
    }

    bool WindowsWindow::IsValid() const
    {
        return Window != nullptr;
    }

    WindowsWindow::~WindowsWindow()
    {
        // TODO: add log about closing the window in destructor

        // It is ok to call virtual here, we are final class
        Destroy();
    }

    glm::ivec2 WindowsWindow::GetSize() const
    {
        glm::ivec2 Size {};
        SDL_GetWindowSize(Window, &Size.x, &Size.y);

        return Size;
    }

    bool WindowsWindow::Swap()
    {
        CHECKED(SwapChain->Present(0, DXGI_PRESENT_ALLOW_TEARING), "Failed to present swap chain")

        return true;
    }

    void WindowsWindow::HandleWindowSizeChanged()
    {
        if (!CreateUpdateSwapChain())
        {
            // TODO: log error
            return;
        }

        if (!CreateUpdateRenderTargets())
        {
            // TODO: log error
            return;
        }
    }

    Core::Window::WindowRenderData WindowsWindow::GetCurrentRenderTargetView() const
    {
        const int Index = SwapChain->GetCurrentBackBufferIndex();

        return {
            .WindowRenderTarget = RenderTargets[Index].Get(),
            .Handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(RtvCpuDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), Index, DescriptorIncrementSize)
        };
    }

    bool WindowsWindow::CreateUpdateSwapChain()
    {
        RenderApi->WaitForQueue(RenderApi->GetDirectQueue().Get());

        if (SwapChain)
        {
            for (auto& RenderTarget: RenderTargets)
            {
                RenderTarget.Reset();
            }

            CHECKED(
                SwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING),
                "Failed to resize Swap Chain Buffers. Most likely you forgot to remove render data from a scene view, connected to this window"
            )
        }
        else
        {
            DXGI_SWAP_CHAIN_DESC1 SwapChainDesc = {};
            SwapChainDesc.BufferCount = SWAP_CHAIN_BUFFER_COUNT;
            SwapChainDesc.Width = 0;
            SwapChainDesc.Height = 0;
            SwapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
            SwapChainDesc.SampleDesc.Count = 1;
            SwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

            Microsoft::WRL::ComPtr<IDXGISwapChain1> SwapChain1 {};

            CHECKED(
                RenderApi->GetDXGIFactory()
                    ->CreateSwapChainForHwnd(
                        RenderApi->GetDirectQueue().Get(),
                        WindowHandle,
                        &SwapChainDesc,
                        nullptr,
                        nullptr,
                        &SwapChain1
                    ),
                "Failed to create Swap Chain"
            )

            CHECKED_S(SwapChain1.As(&SwapChain))
        }

        SwapChainBufferIndex = SwapChain->GetCurrentBackBufferIndex();

        return true;
    }

    bool WindowsWindow::CreateUpdateRenderTargets()
    {
        if (RtvCpuDescriptorHeap != nullptr)
        {
            for (auto& RenderTarget: RenderTargets)
            {
                RenderTarget.Reset();
            }
        }
        else
        {
            D3D12_DESCRIPTOR_HEAP_DESC RtvHeapDesc = {};
            RtvHeapDesc.NumDescriptors = SWAP_CHAIN_BUFFER_COUNT;
            RtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

            CHECKED(
                RenderApi->GetDevice()->CreateDescriptorHeap(&RtvHeapDesc, IID_PPV_ARGS(&RtvCpuDescriptorHeap)),
                "Could not create Rtv Descriptor Heap for Swap Chain"
            )
        }

        if (DescriptorIncrementSize < 0)
            DescriptorIncrementSize = RenderApi->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        CD3DX12_CPU_DESCRIPTOR_HANDLE CpuHandle { RtvCpuDescriptorHeap->GetCPUDescriptorHandleForHeapStart() };

        for (int i = 0; i < SWAP_CHAIN_BUFFER_COUNT; i++)
        {
            CHECKED(
                SwapChain->GetBuffer(i, IID_PPV_ARGS(&RenderTargets[i])),
                "Failed to get swap chain buffer from Swap Chain"
            )

            RenderApi->GetDevice()->CreateRenderTargetView(RenderTargets[i].Get(), nullptr, CpuHandle);

            CpuHandle.Offset(1, DescriptorIncrementSize);
        }

        return true;
    }
}
