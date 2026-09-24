#pragma once
#include <cstdint>

namespace th5w {
// Keep the original Windows CRT sequence so recorded inputs remain reproducible.
inline std::uint32_t gameRandomState = 1;
inline void SeedGameRandom(unsigned int seed) { gameRandomState = seed; }
inline int GameRandom()
{
    gameRandomState = gameRandomState * 214013u + 2531011u;
    return (gameRandomState >> 16) & 0x7fff;
}
}
