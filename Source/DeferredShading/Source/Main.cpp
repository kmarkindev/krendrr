#include "DeferredShadingApp.h"

int main()
{
    krendrr::DeferredShading::DeferredShadingApp App {};

    App.Initialize("Deferred shading using SDL3 and OpengGL 4.6", {1600, 900});
    App.ExecuteMainLoop();
    App.Uninitialize();
}