#include "GameWindow.h"
#include "../../Game/Game.h"
#include <algorithm>

namespace th5w {
CGameWindow::~CGameWindow() { Destroy(); }
bool CGameWindow::Create(bool fullscreen, int width, int height)
{
    m_width = width;
    m_height = height;
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    m_window = SDL_CreateWindow("Touhou 5: Mystic Square", width, height,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
    if (!m_window) return false;
    SDL_SetWindowMinimumSize(m_window, width, height);
    m_pGLContext = new gl::CGLContext;
    if (!m_pGLContext->Init(m_window)) return false;
    UpdateViewport();
    if (fullscreen) ToggleFullScreen();
    return true;
}
void CGameWindow::Destroy()
{
    delete m_pGLContext;
    m_pGLContext = nullptr;
    if (m_window) SDL_DestroyWindow(m_window);
    m_window = nullptr;
}
void CGameWindow::UpdateViewport()
{
    int width, height;
    if (!SDL_GetWindowSizeInPixels(m_window, &width, &height)) return;
    const double scale = std::min(double(width) / m_width, double(height) / m_height);
    const int w = int(m_width * scale), h = int(m_height * scale);
    glViewport((width - w) / 2, (height - h) / 2, w, h);
}
void CGameWindow::ToggleFullScreen()
{
    if (SDL_SetWindowFullscreen(m_window, !m_bFullScr)) {
        m_bFullScr = !m_bFullScr;
        if (m_bFullScr) SDL_HideCursor(); else SDL_ShowCursor();
    }
}
bool CGameWindow::ProcessEvents()
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_EVENT_QUIT:
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
            return false;
        case SDL_EVENT_KEY_DOWN:
            if (event.key.scancode == SDL_SCANCODE_RETURN &&
                (event.key.mod & SDL_KMOD_ALT) && !event.key.repeat)
                ToggleFullScreen();
            break;
        case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
            UpdateViewport();
            break;
        case SDL_EVENT_WINDOW_FOCUS_LOST:
            if (m_bActive) CGame::s_pCurGame->OnDeactivate();
            m_bActive = false;
            break;
        case SDL_EVENT_WINDOW_FOCUS_GAINED:
            if (!m_bActive) CGame::s_pCurGame->OnActivate();
            m_bActive = true;
            break;
        case SDL_EVENT_WINDOW_EXPOSED:
            if (!m_bActive) CGame::s_pCurGame->OnPaintInactivate();
            break;
        }
    }
    return true;
}
}
