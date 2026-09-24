#include "Game/Game.h"
#include "Game/TitleScreen/TitleScreen.h"
#include "Game/SelectReplayScreen/SelectReplayScreen.h"
#include "Game/Stage/Stage.h"
#include "gameLib/Graphic/PC98Font.h"
#include "gameLib/Misc/Random.h"
#include <pmdwin.h>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <vector>
#include <algorithm>

using namespace th5w;
struct TestGame : CGame { using CGame::Initialize; };
static void Require(bool condition, const char *message)
{
    if (!condition) throw std::runtime_error(message);
}
static std::vector<unsigned char> Capture(const std::filesystem::path &path)
{
    std::vector<unsigned char> pixels(640*480*3);
    glReadPixels(0, 0, 640, 480, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
    Require(glGetError() == GL_NO_ERROR, "OpenGL error");
    Require(std::count_if(pixels.begin(), pixels.end(), [](unsigned char p) { return p > 32; }) > 10000,
            "Rendered frame is blank");
    std::ofstream output(path, std::ios::binary);
    output << "P6\n640 480\n255\n";
    for (int y = 479; y >= 0; --y)
        output.write(reinterpret_cast<char *>(pixels.data() + y*640*3), 640*3);
    Require(bool(output), "Cannot save screenshot");
    return pixels;
}
int main(int argc, char **argv)
{
    if (argc != 3) { fprintf(stderr, "Usage: port_smoke DATA_DIR OUTPUT_DIR\n"); return 1; }
    const auto data = std::filesystem::absolute(argv[1]);
    const auto output = std::filesystem::absolute(argv[2]);
    std::filesystem::create_directories(output);
    // Every writable game file stays in a disposable test directory.
    for (const char *name : {"kaiki1.dat", "kaiki2.dat", "zun.com", "chs.dat", "config.ini",
                            "practice.bmp", "replay.bmp", "game.cfg", "score.dat"})
        if (std::filesystem::exists(data / name))
            std::filesystem::copy_file(data / name, output / name, std::filesystem::copy_options::overwrite_existing);
    std::filesystem::copy(data / "SE", output / "SE",
        std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing);
    std::filesystem::current_path(output);
    try {
        SeedGameRandom(1);
        for (int expected : {41, 18467, 6334, 26500, 19169})
            Require(GameRandom() == expected, "Windows replay RNG differs");
        Require(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO), SDL_GetError());
        TestGame game;
        CGame::s_pCurGame = &game;
        Require(game.Initialize(), "Game initialization failed");
        {
            PMDWIN decoder;
            char directory[] = "./";
            Require(decoder.init(directory), "PMD decoder init failed");
            decoder.setpcmrate(44100);
            const int track = game.m_th5Dat1.GetChildFileIndex("OP.M2");
            Require(track >= 0, "Opening music missing");
            Require(decoder.music_load2(game.m_th5Dat1.GetChildFileContentPointer(track),
                game.m_th5Dat1.GetChildFileLength(track)) == 0, "Music load failed");
            decoder.music_start();
            std::vector<short> samples(44100*2);
            decoder.getpcmdata(samples.data(), 44100);
            Require(std::any_of(samples.begin(), samples.end(), [](short s) { return s > 100 || s < -100; }),
                    "PMD decoder produced silence");
            puts("PMD music produces PCM audio");
        }
        {
            CTitleScreen title;
            title.Initialize(false);
            for (int i = 0; i < 180; ++i) title.Step();
            title.Draw();
            Capture(output / "title.ppm");
        }
        {
            CSelectReplayScreen replay;
            replay.Initialize(true, false);
            for (int i = 0; i < 120; ++i) replay.Step();
            replay.Draw();
            Capture(output / "replays.ppm");
        }
        for (int stageNumber = 0; stageNumber < 7; ++stageNumber) {
            auto &vars = CGame::GVar();
            vars.m_playStage = stageNumber;
            vars.m_playChara = stageNumber % 4;
            vars.m_playDifficulty = stageNumber == 6 ? 4 : 1;
            vars.m_bReplayMode = false;
            vars.m_bPracticeMode = true;
            vars.OnBeginGame();
            std::vector<unsigned char> recordedFrame;
            {
                CStage stage;
                stage.Initialize();
                int frames = 0;
                for (; frames < 900; ++frames) {
                    if (stage.Step()) break;
                    if (frames % 60 == 0) stage.Draw();
                    CPMDPlayer::FillSoftwareBuffer(1024);
                }
                stage.Draw();
                recordedFrame = Capture(output / ("stage" + std::to_string(stageNumber+1) + ".ppm"));
                printf("Stage %d: %d frames\n", stageNumber+1, frames);
                Require(frames == 900, "Stage ended unexpectedly");
            }
            vars.m_replay.CloseStage();
            Require(vars.m_replay.SaveFile("test.rpy"), "Replay save failed");
            CReplay restored;
            Require(restored.LoadFile("test.rpy"), "Replay reload failed");
            Require(restored.m_keyState[stageNumber] == vars.m_replay.m_keyState[stageNumber],
                    "Replay inputs differ after reload");
            vars.m_replay = restored;
            vars.m_bReplayMode = true;
            vars.OnBeginGame();
            {
                CStage playback;
                playback.Initialize();
                for (int frame = 0; frame < 900; ++frame) {
                    Require(playback.Step() == 0, "Replay ended early");
                    if (frame % 60 == 0) playback.Draw();
                }
                playback.Draw();
                const auto replayFrame = Capture(output / ("replay-stage" + std::to_string(stageNumber+1) + ".ppm"));
                // Compare the playfield; the HUD intentionally labels playback differently.
                for (int y = 56; y < 424; ++y)
                    for (int x = 32; x < 416; ++x)
                        for (int channel = 0; channel < 3; ++channel) {
                            const int index = (y*640+x)*3+channel;
                            Require(replayFrame[index] == recordedFrame[index], "Replay diverged from recorded gameplay");
                        }
            }
            vars.m_replay.CloseStage();
        }
        game.OnGameEnd();
        SDL_Quit();
        puts("Linux port smoke checks passed");
        return 0;
    } catch (const std::exception &error) {
        fprintf(stderr, "%s\n", error.what());
        return 1;
    }
}
