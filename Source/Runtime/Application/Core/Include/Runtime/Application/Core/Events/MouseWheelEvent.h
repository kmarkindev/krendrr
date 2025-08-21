#pragma once

#include "Event.h"

namespace krendrr::Runtime::Application::Core
{
    class MouseWheelEvent : public Event
    {
    public:

        glm::ivec2 Delta {};
    };
}
