#pragma once
#include <SDL3/SDL.h>
#include <array>
#include <vector>

namespace th5w {
class CSoundEffect {
    struct Sound {
        SDL_AudioStream *stream = nullptr;
        std::vector<unsigned char> data;
    };
    std::array<Sound, 16> m_sounds;
public:
    bool Initialize();
    void PlaySound(int soundIdx);
    void Finalize();
    ~CSoundEffect();
};
}
