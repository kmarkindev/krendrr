#include "SDLAppBase/Camera.h"

namespace krendrr::SDLAppBase
{
    glm::vec3 Camera::GetPosition() const
    {
        return CameraPosition;
    }

    void Camera::SetPosition(glm::vec3 NewPosition)
    {
        CameraPosition = NewPosition;
    }

    void Camera::SetRotation(glm::quat NewRotation)
    {
        CameraRotation = NewRotation;
    }

    void Camera::ReceiveMoveInput(const glm::vec3& NewDirection)
    {
        InputMoveDirection = glm::clamp(NewDirection, {-1, -1, -1}, {1, 1, 1});
    }

    void Camera::ReceiveMouseMove(const glm::vec2& NewMouseMove)
    {
        InputMouseMove = NewMouseMove;
    }

    void Camera::ReceiveRollInput(float NewDirection)
    {
        InputRoll = glm::clamp(NewDirection, -1.0f, 1.0f);
    }

    void Camera::ReceiveMouseMoveEvent(const SDL_Event& Event)
    {
        InputMouseMove = {Event.motion.xrel, Event.motion.yrel};
    }

    void Camera::ReceiveKeyDownEvent(const SDL_Event& Event)
    {
        switch (Event.key.key)
        {
            case SDLK_SPACE:
                InputMoveDirection.y += 1.0f;
            break;
            case SDLK_LSHIFT:
                InputMoveDirection.y += -1.0f;
            break;
            case SDLK_W:
                InputMoveDirection.z += 1.0f;
            break;
            case SDLK_A:
                InputMoveDirection.x += -1.0f;
            break;
            case SDLK_S:
                InputMoveDirection.z += -1.0f;
            break;
            case SDLK_D:
                InputMoveDirection.x += 1.0f;
            break;
            case SDLK_Q:
                InputRoll += -1.0f;
            break;
            case SDLK_E:
                InputRoll += 1.0f;
            break;
        }

        ReceiveMoveInput(InputMoveDirection);
        ReceiveRollInput(InputRoll);
    }

    void Camera::ReceiveKeyUpEvent(const SDL_Event& Event)
    {
        switch (Event.key.key)
        {
            case SDLK_SPACE:
                InputMoveDirection.y -= 1.0f;
            break;
            case SDLK_LSHIFT:
                InputMoveDirection.y -= -1.0f;
            break;
            case SDLK_W:
                InputMoveDirection.z -= 1.0f;
            break;
            case SDLK_A:
                InputMoveDirection.x -= -1.0f;
            break;
            case SDLK_S:
                InputMoveDirection.z -= -1.0f;
            break;
            case SDLK_D:
                InputMoveDirection.x -= 1.0f;
            break;
            case SDLK_Q:
                InputRoll -= -1.0f;
            break;
            case SDLK_E:
                InputRoll -= 1.0f;
            break;
        }

        ReceiveMoveInput(InputMoveDirection);
        ReceiveRollInput(InputRoll);
    }

    void Camera::Update(float DeltaTime)
    {
        glm::mat4 CameraRotMatrix = glm::mat4_cast(CameraRotation);
        glm::vec3 CameraUpVector = CameraRotMatrix * glm::vec4{0.f, 1.f, 0.f, 0.f};
        glm::vec3 CameraForwardVector = CameraRotMatrix * glm::vec4{0.f,0.f,1.f, 0.f};
        glm::vec3 CameraRightVector = CameraRotMatrix * glm::vec4{-1.f, 0.f, 0.f, 0.f};

        // Move
        CameraPosition += CameraRightVector * CameraMoveSpeed * InputMoveDirection.x * DeltaTime;
        CameraPosition += CameraUpVector * CameraMoveSpeed * InputMoveDirection.y * DeltaTime;
        CameraPosition += CameraForwardVector * CameraMoveSpeed * InputMoveDirection.z * DeltaTime;

        // Rotate
        glm::quat MouseXRotation = glm::angleAxis(-glm::radians(InputMouseMove.x * CameraRotationScale), CameraUpVector);
        glm::quat MouseYRotation = glm::angleAxis(-glm::radians(InputMouseMove.y * CameraRotationScale), CameraRightVector);
        CameraRotation = MouseYRotation * MouseXRotation * CameraRotation;
        InputMouseMove = {};

        // Roll
        CameraRotation = glm::angleAxis(glm::radians(InputRoll * CameraRollSpeed * DeltaTime), CameraForwardVector) * CameraRotation;
    }

    glm::mat4 Camera::GetViewMatrix() const
    {
        glm::mat4 CameraRotMatrix = glm::mat4_cast(CameraRotation);
        glm::vec3 CameraForwardVector = CameraRotMatrix * glm::vec4{0.f,0.f,1.f, 0.f};
        glm::vec3 CameraUpVector = CameraRotMatrix * glm::vec4{0.f, 1.f, 0.f, 0.f};

        return glm::lookAt(
            CameraPosition,
            CameraPosition + CameraForwardVector,
            CameraUpVector
        );
    }
}
