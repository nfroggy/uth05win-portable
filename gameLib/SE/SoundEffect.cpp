#include "SoundEffect.h"
#include <cstdio>

namespace th5w {
CSoundEffect::~CSoundEffect() { Finalize(); }
void CSoundEffect::Finalize()
{
    for (auto &sound : m_sounds) {
        if (sound.stream) SDL_DestroyAudioStream(sound.stream);
        sound.stream = nullptr;
        sound.data.clear();
    }
}
bool CSoundEffect::Initialize()
{
    for (int i = 1; i < 16; ++i) {
        char filename[32];
        snprintf(filename, sizeof(filename), "SE/s%02d.wav", i);
        SDL_AudioSpec spec;
        Uint8 *data;
        Uint32 length;
        if (!SDL_LoadWAV(filename, &spec, &data, &length)) {
            SDL_Log("Cannot load %s: %s", filename, SDL_GetError());
            return false;
        }
        m_sounds[i].data.assign(data, data + length);
        SDL_free(data);
        m_sounds[i].stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, nullptr, nullptr);
        if (!m_sounds[i].stream || !SDL_ResumeAudioStreamDevice(m_sounds[i].stream)) return false;
    }
    return true;
}
void CSoundEffect::PlaySound(int index)
{
    if (index <= 0 || index >= int(m_sounds.size())) return;
    auto &sound = m_sounds[index];
    if (!sound.stream) return;
    SDL_ClearAudioStream(sound.stream);
    SDL_PutAudioStreamData(sound.stream, sound.data.data(), int(sound.data.size()));
    SDL_FlushAudioStream(sound.stream);
}
}
