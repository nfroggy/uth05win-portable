#pragma once
#include "../Graphic/GLContext.h"

namespace th5w {
class CGameWindow {
public:
    bool m_bActive = true;
    bool m_bFullScr = false;
    SDL_Window *m_window = nullptr;
    int m_width = 640, m_height = 480; // Logical game coordinates.
    gl::CGLContext *m_pGLContext = nullptr;
    bool Create(bool fullscreen, int width, int height);
    void Destroy();
    void ToggleFullScreen();
    bool ProcessEvents();
    void UpdateViewport();
    ~CGameWindow();
};
}
