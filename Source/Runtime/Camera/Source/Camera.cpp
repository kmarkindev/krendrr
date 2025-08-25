#include "Runtime/Camera/Camera.h"

#include <array>

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

        const bool bRemoveButton = KeyEvent.State == Application::Core::KeyEvent::KeyState::Released;
        constexpr static std::array PossibleButtons = {
            "Space",
            "Left Shift",
            "W",
            "A",
            "S",
            "D"
        };

        auto Iter = std::ranges::find(PossibleButtons, KeyEvent.Key);
        if (Iter != PossibleButtons.end())
        {
            // PS. We use string literals from array so it is safe to store pressed buttons as string_view

            if (bRemoveButton)
                PressedButtons.erase(*Iter);
            else
                PressedButtons.insert(*Iter);
        }
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

        // Gather input direction

        glm::vec3 InputMoveDirection {};

        if (PressedButtons.contains("Space"))
            InputMoveDirection.y += 1.f;
        if (PressedButtons.contains("Left Shift"))
            InputMoveDirection.y += -1.f;
        if (PressedButtons.contains("W"))
            InputMoveDirection.z += 1.f;
        if (PressedButtons.contains("A"))
            InputMoveDirection.x += -1.f;
        if (PressedButtons.contains("S"))
            InputMoveDirection.z += -1.f;
        if (PressedButtons.contains("D"))
            InputMoveDirection.x += 1.f;

        // Rotate

        // Multiply by DeltaTime so FPS drops don't affect mouse sensitivity
        CurrentPitch += InputMouseMove.y * CameraRotationScale * DeltaTime * 100;
        CurrentYaw += InputMouseMove.x * CameraRotationScale * DeltaTime * 100;

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
