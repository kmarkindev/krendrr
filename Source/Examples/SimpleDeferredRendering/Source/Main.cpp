#include "DeferredShadingApp.h"

int main()
{
    krendrr::DeferredShading::DeferredShadingApp App {};

    App.Initialize("Deferred shading using SDL3 and OpengGL 4.6", {1600, 1200});
    App.ExecuteMainLoop();
    App.Uninitialize();
}