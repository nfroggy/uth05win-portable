#include "GLContext.h"

namespace th5w::gl {
CGLContext::~CGLContext()
{
    if (m_context) SDL_GL_DestroyContext(m_context);
}
bool CGLContext::Init(SDL_Window *window)
{
    m_window = window;
    m_context = SDL_GL_CreateContext(window);
    if (!m_context || !MakeCurrent()) return false;
    glShadeModel(GL_SMOOTH);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    SDL_GL_SetSwapInterval(0); // The game supplies its own frame rate (26, 56 or 60 Hz).
    return true;
}
bool CGLContext::MakeCurrent() { return SDL_GL_MakeCurrent(m_window, m_context); }
bool CGLContext::SwapBuffers()
{
    int width, height;
    SDL_GetWindowSizeInPixels(m_window, &width, &height);
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    glPushAttrib(GL_COLOR_BUFFER_BIT | GL_SCISSOR_BIT);
    glEnable(GL_SCISSOR_TEST);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glClearColor(0, 0, 0, 1);
    glScissor(0, 0, viewport[0], height);
    glClear(GL_COLOR_BUFFER_BIT);
    glScissor(viewport[0] + viewport[2], 0, width - viewport[0] - viewport[2], height);
    glClear(GL_COLOR_BUFFER_BIT);
    glScissor(0, 0, width, viewport[1]);
    glClear(GL_COLOR_BUFFER_BIT);
    glScissor(0, viewport[1] + viewport[3], width, height - viewport[1] - viewport[3]);
    glClear(GL_COLOR_BUFFER_BIT);
    glPopAttrib();
    return SDL_GL_SwapWindow(m_window);
}
}
