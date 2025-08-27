#pragma once

#include "Event.h"
#include "glm/vec2.hpp"

namespace krendrr::Runtime::Application::Core
{
    class MouseWheelEvent : public Event
    {
    public:

        glm::ivec2 Delta {};
    };
}
