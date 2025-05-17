#include "SwapChain.h"
#include <dxgi1_5.h>
#include <dxgi1_6.h>
#include <format>
#include <d3dx12/d3dx12_root_signature.h>
#include "Sources/Render/RenderDevice.h"
#include "Sources/Render/Resources/Commands/CommandQueue.h"
#include "Sources/Utils/HResultCheck.h"

namespace kRendrr
{
    SwapChain::SwapChain(HWND Hwnd)
        : Hwnd(Hwnd), BuffersCount(2)
    {

    }

    void SwapChain::Initialize(const RenderDevice& RenderDevice, const CommandQueue& CommandQueue)
    {
        CheckInitialization(false);

        // Create Swap Chain COM

        Microsoft::WRL::ComPtr<IDXGIFactory6> Factory;
        CreateDXGIFactory2(RenderDevice.GetDxgiFlags(), IID_PPV_ARGS(&Factory))
            >> HResultCheck{};

        DXGI_SWAP_CHAIN_DESC1 SwapChainDesc = {};
        SwapChainDesc.BufferCount = BuffersCount;
        SwapChainDesc.Width = 0;
        SwapChainDesc.Height = 0;
        SwapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        SwapChainDesc.SampleDesc.Count = 1;

        Microsoft::WRL::ComPtr<IDXGISwapChain1> SwapChain1 {};
        Factory->CreateSwapChainForHwnd(
            CommandQueue.GetQueue().Get(),
            Hwnd,
            &SwapChainDesc,
            nullptr,
            nullptr,
            &SwapChain1
            ) >> HResultCheck {};

        SwapChain1.As(&DxgiSwapChain)
            >> HResultCheck {};

        // Get Swap Chain's RTVs

        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
        rtvHeapDesc.NumDescriptors = BuffersCount;
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

        RenderDevice.GetDevice()
            ->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&CpuRenderTargetsHeap))
            >> HResultCheck {};

        const UINT DescriptorIncrement = RenderDevice.GetDevice()
            ->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        CD3DX12_CPU_DESCRIPTOR_HANDLE DescriptorHandle { CpuRenderTargetsHeap->GetCPUDescriptorHandleForHeapStart() };

        RenderTargets.reserve(BuffersCount);
        for(int i = 0; i < BuffersCount; i++)
        {
            // Get allocated RTV from Swap Chain
            Microsoft::WRL::ComPtr<ID3D12Resource> Resource;
            DxgiSwapChain->GetBuffer(i, IID_PPV_ARGS(&Resource)) >> HResultCheck {};

            // Create CPU Handle to use RTV later
            RenderDevice.GetDevice()
                ->CreateRenderTargetView(Resource.Get(), nullptr, DescriptorHandle);

            // Initialize RTV using already existing data
            RenderTargets
                .emplace_back()
                .Initialize(Resource, DescriptorHandle);

            std::wstring name = std::format(L"Swap Chain RT {}", i);
            Resource->SetName(name.c_str())
                >> HResultCheck {};

            DescriptorHandle.Offset(1, DescriptorIncrement);
        }

        MarkAsInitialized();
    }

    const RenderTarget& SwapChain::GetCurrentRenderTargetView() const
    {
        CheckInitialization();

        return RenderTargets[DxgiSwapChain->GetCurrentBackBufferIndex()];
    }

    void SwapChain::Present()
    {
        CheckInitialization();

        DxgiSwapChain->Present(0, 0)
            >> HResultCheck {};
    }
}
