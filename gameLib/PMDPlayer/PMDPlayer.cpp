#include "PMDPlayer.h"
#include <SDL3/SDL.h>
#include <pmdwin.h>
#include <algorithm>
#include <memory>
#include <vector>

namespace th5w {
namespace {
std::unique_ptr<PMDWIN> decoder;
SDL_AudioStream *stream = nullptr;
bool loaded = false, playing = false, resumeOnActivate = false;
constexpr int sampleRate = 44100;
}
bool CPMDPlayer::Initialize()
{
    decoder = std::make_unique<PMDWIN>();
    char directory[] = "./";
    if (!decoder->init(directory)) return false;
    decoder->setpcmrate(sampleRate);
    SDL_AudioSpec spec{SDL_AUDIO_S16, 2, sampleRate};
    stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, nullptr, nullptr);
    return stream && SDL_ResumeAudioStreamDevice(stream);
}
bool CPMDPlayer::LoadPMDData(unsigned char *data, int size)
{
    UnloadPMDData();
    if (!decoder || !data || size <= 0) return false;
    loaded = decoder->music_load2(data, size) == 0;
    return loaded;
}
bool CPMDPlayer::UnloadPMDData()
{
    playing = loaded = resumeOnActivate = false;
    if (decoder) decoder->music_stop();
    if (stream) SDL_ClearAudioStream(stream);
    return true;
}
void CPMDPlayer::Play()
{
    if (!loaded || !stream) return;
    SDL_ClearAudioStream(stream);
    decoder->music_start();
    playing = true;
    SetFrequency(sampleRate);
    SetVolume(100);
    FillSoftwareBuffer(4096);
    SDL_ResumeAudioStreamDevice(stream);
}
void CPMDPlayer::Pause()
{
    playing = false;
    if (stream) SDL_PauseAudioStreamDevice(stream);
}
void CPMDPlayer::Resume()
{
    if (!loaded || !stream) return;
    playing = true;
    SDL_ResumeAudioStreamDevice(stream);
}
void CPMDPlayer::Finalize()
{
    UnloadPMDData();
    if (stream) SDL_DestroyAudioStream(stream);
    stream = nullptr;
    decoder.reset();
}
void CPMDPlayer::SetFrequency(int frequency)
{
    if (stream && frequency > 0)
        SDL_SetAudioStreamFrequencyRatio(stream, float(frequency) / sampleRate);
}
void CPMDPlayer::SetVolume(float volume)
{
    if (stream) SDL_SetAudioStreamGain(stream, std::clamp(volume / 100.f, 0.f, 1.f));
}
void CPMDPlayer::OnDeactivate() { resumeOnActivate = playing; Pause(); }
void CPMDPlayer::OnActivate() { if (resumeOnActivate) Resume(); resumeOnActivate = false; }
bool CPMDPlayer::FillSoftwareBuffer(int samples)
{
    if (!playing || !stream || samples <= 0) return false;
    // Keep about 100 ms queued without advancing the decoder while paused.
    if (SDL_GetAudioStreamQueued(stream) >= sampleRate / 10 * 4) return false;
    samples = std::min(samples, 4096);
    std::vector<short> buffer(samples * 2);
    decoder->getpcmdata(buffer.data(), samples);
    return SDL_PutAudioStreamData(stream, buffer.data(), samples * 4);
}
}
