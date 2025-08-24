#include "Camera.h"
#include "glm/common.hpp"
#include "glm/ext/quaternion_trigonometric.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/norm.inl"
#include "Runtime/Application/Core/Events/KeyEvent.h"
#include "Runtime/Application/Core/Events/MouseMoveEvent.h"
#include "Runtime/Application/Core/Events/MouseWheelEvent.h"
#include "Runtime/Renderer/Core/Scene/SceneView.h"

namespace krendrr::Runtime::Camera
{
    void Camera::SetSceneView(Renderer::Core::SceneView* NewSceneView)
    {
        ControlledSceneView = NewSceneView;
    }

    void Camera::ReceiveKeyInput(const Application::Core::KeyEvent& KeyEvent)
    {
        const float KeyStateScale = KeyEvent.State == Application::Core::KeyEvent::KeyState::Pressed ? 1.f : -1.f;

        if (KeyEvent.Key == "Space")
            InputMoveDirection.y += KeyStateScale;
        else if (KeyEvent.Key == "Left Shift")
            InputMoveDirection.y += -KeyStateScale;
        else if (KeyEvent.Key == "W")
            InputMoveDirection.z += KeyStateScale;
        else if (KeyEvent.Key == "A")
            InputMoveDirection.x += -KeyStateScale;
        else if (KeyEvent.Key == "S")
            InputMoveDirection.z += -KeyStateScale;
        else if (KeyEvent.Key == "D")
            InputMoveDirection.x += KeyStateScale;
        else if (KeyEvent.Key == "Q")
            InputRoll += -KeyStateScale;
        else if (KeyEvent.Key == "E")
            InputRoll += KeyStateScale;

        InputMoveDirection = glm::clamp(InputMoveDirection, {-1, -1, -1}, {1, 1, 1});
        InputRoll = glm::clamp(InputRoll, -1.0f, 1.0f);
    }

    void Camera::ReceiveMouseMoveInput(const Application::Core::MouseMoveEvent& MouseEvent)
    {
        InputMouseMove = {static_cast<float>(MouseEvent.Delta.x), static_cast<float>(MouseEvent.Delta.y)};
    }

    void Camera::ReceiveMouseWheelInput(const Application::Core::MouseWheelEvent& WheelEvent)
    {
        CameraMoveSpeed = glm::clamp(CameraMoveSpeed + WheelEvent.Delta.y * ScrollMagnifier, MinMoveSpeed, MaxMoveSpeed);
    }

    void Camera::Update(float DeltaTime)
    {
        if (!ControlledSceneView)
            return;

        glm::quat CameraRotation = ControlledSceneView->GetRotation();
        glm::vec3 CameraPosition = ControlledSceneView->GetPosition();

        const glm::vec3 CameraUpVector = CameraRotation * glm::vec4{0.f, 1.f, 0.f, 0.f};
        const glm::vec3 CameraForwardVector = CameraRotation * glm::vec4{0.f,0.f,-1.f, 0.f};
        const glm::vec3 CameraRightVector = CameraRotation * glm::vec4{1.f, 0.f, 0.f, 0.f};

        // Move
        glm::vec3 MoveDirection {};
        if (glm::length2(InputMoveDirection) > 0.f)
            MoveDirection = glm::normalize(InputMoveDirection);

        CameraPosition += CameraRightVector * CameraMoveSpeed * MoveDirection.x * DeltaTime;
        CameraPosition += CameraUpVector * CameraMoveSpeed * MoveDirection.y * DeltaTime;
        CameraPosition += CameraForwardVector * CameraMoveSpeed * MoveDirection.z * DeltaTime;
        ControlledSceneView->SetPosition(CameraPosition);

        // Rotate
        glm::quat MouseXRotation = glm::angleAxis(-glm::radians(InputMouseMove.x * CameraRotationScale), CameraUpVector);
        glm::quat MouseYRotation = glm::angleAxis(-glm::radians(InputMouseMove.y * CameraRotationScale), CameraRightVector);
        CameraRotation = MouseYRotation * MouseXRotation * CameraRotation;
        InputMouseMove = {};

        // Roll
        CameraRotation = glm::angleAxis(glm::radians(InputRoll * CameraRollSpeed * DeltaTime), CameraForwardVector) * CameraRotation;

        ControlledSceneView->SetRotation(CameraRotation);
    }
}
