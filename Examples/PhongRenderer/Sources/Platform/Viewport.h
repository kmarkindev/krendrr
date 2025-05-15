#pragma once
#include <memory>

#include "Window/Window.h"
#include <glm/vec2.hpp>

class RenderTargetView;
class RenderDevice;

/**
 * Represents some window/widget/panel that should be treated as a viewport for drawing.
 */
class Viewport
{
public:

    explicit Viewport(std::shared_ptr<RenderDevice> RenderDevice);

    Viewport(std::shared_ptr<RenderDevice> RenderDevice, HWND Hwnd);

    void Initialize();

    void Uninitialize();

    /**
     * Get viewport pixel size used for drawing (exluding borders, shadows, etc.)
     */
    [[nodiscard]] glm::vec2 GetSize() const;

    [[nodiscard]] std::shared_ptr<RenderTargetView> GetCurrentRenderTargetView() const;

    void PresentAndSwapCurrentRenderTargetView();

private:

    std::shared_ptr<RenderDevice> RenderDevice {};

    HWND Hwnd {};

    static HWND CreateDefaultWindow();

};
