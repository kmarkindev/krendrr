#include "Examples/SimpleDeferredRendering/Application.h"
#include <array>
#include <iostream>
#include <memory>
#include "Runtime/Application/Core/EntryPoint.h"
#include "Runtime/Application/Core/StartupArgs.h"
#include "Runtime/Application/Core/Window.h"
#include "Runtime/ModelLoader/Loader.h"
#include "Runtime/RenderApi/Core/RenderApi.h"
#include "Runtime/Renderer/Core/Lights/PointLight.h"
#include "Runtime/Renderer/Core/Scene/Scene.h"

IMPLEMENT_ENTRY_POINT(krendrr::Examples::SimpleDeferredRendering::Application)

bool krendrr::Examples::SimpleDeferredRendering::Application::Initialize(const Runtime::Application::Core::StartupArgs& Args)
{
    std::cout << "Startup Args: ";
    for (const auto& Arg: Args.GetStartupArgs())
    {
        std::cout << Arg << " ";
    }
    std::cout << std::endl;

    bLoadFuturisticScene = Args.GetStartupArgs().size() > 1 && Args.GetStartupArgs()[1] == "futuristic";

    RenderApi = std::make_shared<Runtime::RenderApi::Core::RenderApi>();
    const bool bRenderApiInitResult = RenderApi->Initialize({
        .Debug = Runtime::RenderApi::Core::RenderApi::InitParams::Debug::DebugLayerWithGpuBasedValidation,
        .bEnableShadersDebug = true
    });

    if (!bRenderApiInitResult)
    {
        // TODO: log error
        return false;
    }

    Window = std::unique_ptr<Runtime::Application::Core::Window>{
        Runtime::Application::Core::Window::Create(RenderApi, {
            .Title = "Deferred Rendering Example",
            .Size = {1280, 720}
        })
    };

    Scene = std::make_shared<Runtime::Renderer::Core::Scene>();

    Runtime::ModelLoader::LoadResult MeshesLoadResult = Runtime::ModelLoader::LoadModel(
        bLoadFuturisticScene
            ?  "../Content/krendrr_examples_deferredrendering/FuturisticRoom/source/CyberPunkRoom.fbx"
            : "../Content/krendrr_examples_deferredrendering/Sponza/sponza.obj",
        *RenderApi,
        {
            .bFlipUVs = true,
            .bFlipNormals = bLoadFuturisticScene,
        }
    );

    if (!MeshesLoadResult.HasLoadedAtLeastOne())
    {
        // TODO: log error
        return false;
    }

    for (const auto& TexturedMesh : MeshesLoadResult.TexturedMeshes)
    {
        Scene->InsertTexturedMesh(TexturedMesh);
    }

    if (!InitializeSceneViewAndCamera())
    {
        // TODO: add error log
        return false;
    }

    if (bLoadFuturisticScene)
        SetupFuturisticScene();
    else
        SetupSponzaScene();

    Renderer = std::make_unique<Runtime::Renderer::Deferred::DeferredRenderer>();
    if (!Renderer->Initialize(RenderApi, Scene))
    {
        // TODO: log error
        return false;
    }

    return true;
}

bool krendrr::Examples::SimpleDeferredRendering::Application::Tick(float DeltaTime)
{
    // Show FPS in console
    {
        static std::size_t FpsDisplayCounter = 0;
        FpsDisplayCounter++;
        if (FpsDisplayCounter % 100 == 0)
            std::cout << "Delta: " << DeltaTime << " FPS: " << 1.f / DeltaTime << std::endl;
    }

    const glm::ivec2 WindowSize = Window->GetSize();
    Runtime::Application::Core::Window::WindowRenderData RenderData = Window->GetCurrentRenderTargetView();

    const bool bRenderDataSetSuccess = SceneView.SetRenderData(
        {
            RenderData.WindowRenderTarget,
            RenderData.Handle,
            D3D12_RESOURCE_STATE_PRESENT
        }, {
            0,
            0,
            WindowSize.x,
            WindowSize.y
        }
    );

    if (!bRenderDataSetSuccess)
        return false;

    Camera.Update(DeltaTime);

    std::array Views = {
        SceneView
    };
    if (!Renderer->Render(Views))
    {
        // TODO: add error log
        return false;
    }

    if (!Window->Swap())
    {
        // TODO: add error log
        return false;
    }

    return true;
}

bool krendrr::Examples::SimpleDeferredRendering::Application::Shutdown()
{
    Renderer->Shutdown();

    return true;
}

void krendrr::Examples::SimpleDeferredRendering::Application::HandleKeyEvent(const Runtime::Application::Core::KeyEvent& Event)
{
    Camera.ReceiveKeyInput(Event);
}

void krendrr::Examples::SimpleDeferredRendering::Application::HandleMouseMoveEvent(const Runtime::Application::Core::MouseMoveEvent& Event)
{
    Camera.ReceiveMouseMoveInput(Event);
}

void krendrr::Examples::SimpleDeferredRendering::Application::HandleMouseWheelEvent(const Runtime::Application::Core::MouseWheelEvent& Event)
{
    Camera.ReceiveMouseWheelInput(Event);
}

void krendrr::Examples::SimpleDeferredRendering::Application::SetupFuturisticScene()
{
    Camera.SetCameraPosition({-234.753769, 132.926086, 199.352264});
    Camera.SetCameraRotation({5, 40});

    Scene->ToggleAmbientLight(true);

    auto PointLight = Scene->SpawnPointLight();
    PointLight->SetPosition({-350, 130, -170});
}

void krendrr::Examples::SimpleDeferredRendering::Application::SetupSponzaScene()
{
    Camera.SetCameraPosition({0, 250, 0});
    Camera.SetCameraRotation({0, 0});

    Scene->ToggleAmbientLight(true);

    auto AmbientLight = Scene->GetAmbientLightData();
    AmbientLight.Intensity = 0.00125f;

    Scene->SetAmbientLightData(AmbientLight);

    auto PointLight1 = Scene->SpawnPointLight();
    PointLight1->SetPosition({1000, 250, 0});
    PointLight1->SetColor({1.f, 0.3f, 0.3f});
    PointLight1->SetCastsShadows(false);

    auto PointLight2 = Scene->SpawnPointLight();
    PointLight2->SetPosition({-1100, 250, 0});
    PointLight2->SetColor({0.3f, 1.0f, 0.3f});
}

bool krendrr::Examples::SimpleDeferredRendering::Application::InitializeSceneViewAndCamera()
{
    if (!SceneView.Initialize())
    {
        // TODO: add error log
        return false;
    }

    Camera.SetSceneView(&SceneView);

    return true;
}
