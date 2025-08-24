#include "Runtime/Camera/Camera.h"
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
        if (!ControlledSceneView)
            return;

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

        InputMoveDirection = glm::clamp(InputMoveDirection, {-1, -1, -1}, {1, 1, 1});
    }

    void Camera::ReceiveMouseMoveInput(const Application::Core::MouseMoveEvent& MouseEvent)
    {
        if (!ControlledSceneView)
            return;

        InputMouseMove = {static_cast<float>(MouseEvent.Delta.x), static_cast<float>(MouseEvent.Delta.y)};
    }

    void Camera::ReceiveMouseWheelInput(const Application::Core::MouseWheelEvent& WheelEvent)
    {
        if (!ControlledSceneView)
            return;

        CameraMoveSpeed = glm::clamp(CameraMoveSpeed + WheelEvent.Delta.y * ScrollMagnifier, MinMoveSpeed, MaxMoveSpeed);
    }

    void Camera::Update(float DeltaTime)
    {
        if (!ControlledSceneView)
            return;

        // Rotate

        CurrentPitch += InputMouseMove.y * CameraRotationScale;
        CurrentYaw += InputMouseMove.x * CameraRotationScale;

        CurrentPitch = glm::clamp(CurrentPitch, -80.f, 80.f);

        glm::quat YawRotation = glm::angleAxis(-glm::radians(CurrentYaw), glm::vec3{0, 1, 0});
        glm::quat PitchRotation = glm::angleAxis(-glm::radians(CurrentPitch), glm::vec3{1, 0, 0});

        glm::quat CameraRotation = YawRotation * PitchRotation;

        ControlledSceneView->SetRotation(CameraRotation);
        InputMouseMove = {};

        // Move

        constexpr glm::vec3 UpVector = glm::vec4{0.f, 1.f, 0.f, 0.f};
        const glm::vec3 CameraForwardVector = CameraRotation * glm::vec4{0.f,0.f,-1.f, 0.f};
        const glm::vec3 CameraRightVector = CameraRotation * glm::vec4{1.f, 0.f, 0.f, 0.f};

        glm::vec3 CameraPosition = ControlledSceneView->GetPosition();

        glm::vec3 MoveDirection {};
        if (glm::length2(InputMoveDirection) > 0.f)
            MoveDirection = glm::normalize(InputMoveDirection);

        CameraPosition += CameraRightVector * CameraMoveSpeed * MoveDirection.x * DeltaTime;
        CameraPosition += UpVector * CameraMoveSpeed * MoveDirection.y * DeltaTime;
        CameraPosition += CameraForwardVector * CameraMoveSpeed * MoveDirection.z * DeltaTime;

        ControlledSceneView->SetPosition(CameraPosition);
    }

    void Camera::SetCameraPosition(glm::vec3 NewPosition)
    {
        if (!ControlledSceneView)
            return;

        ControlledSceneView->SetPosition(NewPosition);
    }

    void Camera::SetCameraRotation(glm::vec2 NewRotation)
    {
        if (!ControlledSceneView)
            return;

        CurrentPitch = NewRotation.x;
        CurrentYaw = NewRotation.y;
    }
}
