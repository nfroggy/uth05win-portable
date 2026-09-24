#include "KeyInputDI.h"

namespace th5w {
unsigned char CKeyInputDI::GetKeyState(int scancode) const
{
    int count;
    const bool *keys = SDL_GetKeyboardState(&count);
    return scancode >= 0 && scancode < count && keys[scancode];
}
}
