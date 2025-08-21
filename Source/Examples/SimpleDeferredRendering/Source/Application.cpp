#include "Application.h"
#include "Runtime/Application/Core/EntryPoint.h"

IMPLEMENT_ENTRY_POINT(krendrr::Examples::SimpleDeferredRendering::Application)

krendrr::Examples::SimpleDeferredRendering::Application::Application(const Runtime::Application::Core::StartupArgs& Args)
    : Runtime::Application::Core::Application(Args)
{

}
