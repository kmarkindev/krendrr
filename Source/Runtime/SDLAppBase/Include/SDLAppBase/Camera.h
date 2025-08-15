#pragma once

#include <glm/fwd.hpp>
#include <glm/detail/type_quat.hpp>
#include <glm/gtc/quaternion.hpp>
#include <SDL3/SDL_events.h>

namespace krendrr::SDLAppBase
{

class Camera 
{
public:

    [[nodiscard]] glm::vec3 GetPosition() const;
    void SetPosition(glm::vec3 NewPosition);
    void SetRotation(glm::quat NewRotation);

    void ReceiveMoveInput(const glm::vec3& NewDirection);
    void ReceiveMouseMove(const glm::vec2& NewMouseMove);
    void ReceiveRollInput(float NewDirection);
    void ReceiveScrollInput(float ScrollDelta);

    void ReceiveMouseMoveEvent(const SDL_Event& Event);
    void ReceiveKeyDownEvent(const SDL_Event& Event);
    void ReceiveKeyUpEvent(const SDL_Event& Event);
    void ReceiveScrollEvent(const SDL_Event& Event);

    void Update(float DeltaTime);

    [[nodiscard]] glm::mat4 GetViewMatrix() const;

private:

    float CameraMoveSpeed = 100.f;
    float CameraRotationScale = 0.35f;
    float CameraRollSpeed = 45.f;

    float ScrollMagnifier = 5.f;
    float MinMoveSpeed = 10.f;
    float MaxMoveSpeed = 300.f;

    glm::vec3 CameraPosition {};
    glm::quat CameraRotation {1, {}};

    glm::vec3 InputMoveDirection {};
    glm::vec2 InputMouseMove {};
    float InputRoll {};
};

}
