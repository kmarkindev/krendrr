#pragma once

#include "Event.h"

namespace krendrr::Runtime::Application::Core
{
    class KeyEvent : public Event
    {
    public:

        enum class KeyState
        {
            Pressed,
            Released,
        };

        std::string_view Key {};

        KeyState State {};
    };
}

