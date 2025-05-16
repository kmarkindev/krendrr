#pragma once

#include "Sources/Utils/Mixins/CheckInitializationMixin.h"

namespace kRendrr
{
    class RenderResource : protected CheckInitializationMixin
    {

    public:

        RenderResource() = default;

        RenderResource(const RenderResource&) = delete;
        RenderResource& operator=(const RenderResource&) = delete;

        RenderResource(RenderResource&&) = default;
        RenderResource& operator=(RenderResource&&) = default;

        ~RenderResource() = default;

    };
}
