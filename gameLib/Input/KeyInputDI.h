#pragma once
#include <SDL3/SDL.h>

namespace th5w {
// The name is retained for the existing game-facing input interface.
class CKeyInputDI {
public:
    bool Initialize() { return true; }
    bool UpdateKeyState() { return true; } // Events are pumped by the main loop.
    unsigned char GetKeyState(int scancode) const;
};
}
