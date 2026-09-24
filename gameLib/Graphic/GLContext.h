#pragma once
#include "mygl.h"
#include <SDL3/SDL.h>

namespace th5w::gl {
class CGLContext {
    SDL_Window *m_window = nullptr;
    SDL_GLContext m_context = nullptr;
public:
    bool Init(SDL_Window *window);
    bool MakeCurrent();
    bool SwapBuffers();
    ~CGLContext();
};
}
