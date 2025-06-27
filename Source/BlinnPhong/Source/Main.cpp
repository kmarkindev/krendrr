#include "BlinnPhongApp.h"

int main()
{
    krendrr::BlinnPhong::BlinnPhongApp App {};

    App.Initialize("BlinnPhong model renderer using OpenGL 4.6 and SDL3", {1280, 720});
    App.ExecuteMainLoop();
    App.Uninitialize();

    return 0;
}