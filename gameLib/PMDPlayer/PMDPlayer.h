#pragma once

namespace th5w {
// All decoder access happens on the game thread; SDL consumes the queued PCM.
class CPMDPlayer {
public:
    static bool Initialize();
    static bool LoadPMDData(unsigned char *data, int size);
    static bool UnloadPMDData();
    static void Play();
    static void Pause();
    static void Resume();
    static void Finalize();
    static void SetFrequency(int frequency);
    static void SetVolume(float volume);
    static void OnDeactivate();
    static void OnActivate();
    static bool FillSoftwareBuffer(int samples);
};
}
