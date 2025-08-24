#pragma once

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

    private:

        glm::vec3 InputMoveDirection {};
        glm::vec2 InputMouseMove {};
        float InputRoll {};

        float CameraMoveSpeed = 100.f;
        float CameraRotationScale = 0.35f;
        float CameraRollSpeed = 45.f;

        float ScrollMagnifier = 15.f;
        float MinMoveSpeed = 10.f;
        float MaxMoveSpeed = 500.f;

        Renderer::Core::SceneView* ControlledSceneView {};

    };
}

