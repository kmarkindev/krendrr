#pragma once

#include <memory>
#include "Runtime/Application/Core/Application.h"
#include "Runtime/Camera/Camera.h"
#include "Runtime/RenderApi/Core/RenderApi.h"
#include "Runtime/Renderer/Core/Scene/SceneView.h"
#include "Runtime/Renderer/Deferred/DeferredRenderer.h"

namespace krendrr::Runtime::Application::Core
{
    class Window;
}

namespace krendrr::Examples::SimpleDeferredRendering
{
    class Application final : public Runtime::Application::Core::Application
    {
    public:

        bool Tick(float DeltaTime) override;

        bool Initialize(const Runtime::Application::Core::StartupArgs& Args) override;

        bool Shutdown() override;

        void HandleKeyEvent(const Runtime::Application::Core::KeyEvent& Event) override;

        void HandleMouseMoveEvent(const Runtime::Application::Core::MouseMoveEvent& Event) override;

        void HandleMouseWheelEvent(const Runtime::Application::Core::MouseWheelEvent& Event) override;

    private:

        std::shared_ptr<tf::Executor> TfExecutor {};

        std::shared_ptr<Runtime::RenderApi::Core::RenderApi> RenderApi {};

        bool bLoadFuturisticScene {false};

        void SetupFuturisticScene();
        void SetupSponzaScene();

        std::unique_ptr<Runtime::Application::Core::Window> Window {};
        std::shared_ptr<Runtime::Renderer::Core::Scene> Scene {};
        std::unique_ptr<Runtime::Renderer::Core::Renderer> Renderer {};

        Runtime::Renderer::Core::SceneView SceneView {};
        Runtime::Camera::Camera Camera {};

        bool InitializeSceneViewAndCamera();

        bool FillRenderTaskflow(tf::Taskflow& Taskflow, const std::function<void()>& ErrorStop);

    };
}


