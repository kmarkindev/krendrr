#pragma once

#include "Event.h"

namespace krendrr::Runtime::Application::Core
{
    class MouseMoveEvent : public Event
    {
    public:

        glm::ivec2 Delta {};
        glm::ivec2 Position {};
    };
}
