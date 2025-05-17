#pragma once

#include <cstdint>
#include <d3d12.h>
#include <wrl/client.h>
#include "Sources/Utils/Mixins/CheckInitializationMixin.h"

namespace kRendrr
{
    class RenderDevice : public CheckInitializationMixin
    {
    public:

        struct RenderDeviceInitParams
        {
            enum class DebugMode : uint8_t
            {
                Disabled,
                Enabled,
                EnabledWithGpuBasedValidation
            };

            DebugMode DebugMode { DebugMode::Disabled };
        };

        void Initialize(const RenderDeviceInitParams& InitParams);

        [[nodiscard]] uint8_t GetDxgiFlags() const;

        [[nodiscard]] Microsoft::WRL::ComPtr<ID3D12Device> GetDevice() const;

    private:

        Microsoft::WRL::ComPtr<ID3D12Device> Device {};
        uint8_t DxgiFlags {};

    };
}
