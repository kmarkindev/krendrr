#pragma once

#include <set>
#include <string_view>
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"

namespace krendrr::Runtime::Application::Core
{
    class MouseMoveEvent;
    class KeyEvent;
    class MouseWheelEvent;
}

namespace krendrr::Runtime::Renderer::Core
{
    class SceneView;
}

namespace krendrr::Runtime::Camera
{
    class Camera
    {
    public:

        void SetSceneView(Renderer::Core::SceneView* NewSceneView);

        void ReceiveKeyInput(const Application::Core::KeyEvent& KeyEvent);

        void ReceiveMouseMoveInput(const Application::Core::MouseMoveEvent& MouseEvent);

        void ReceiveMouseWheelInput(const Application::Core::MouseWheelEvent& WheelEvent);

        void Update(float DeltaTime);

        void SetCameraPosition(glm::vec3 NewPosition);

        /**
         * X - Pitch, Y - Yaw
         */
        void SetCameraRotation(glm::vec2 NewRotation);

    private:

        std::set<std::string_view> PressedButtons {};

        glm::vec2 InputMouseMove {};

        float CurrentYaw {};
        float CurrentPitch {};

        float CameraMoveSpeed = 100.f;
        float CameraRotationScale = 0.35f;

        float ScrollMagnifier = 15.f;
        float MinMoveSpeed = 10.f;
        float MaxMoveSpeed = 500.f;

        Renderer::Core::SceneView* ControlledSceneView {};

    };
}

