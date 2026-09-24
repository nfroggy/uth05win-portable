#include "Game/Game.h"
#include <SDL3/SDL_main.h>
#include <filesystem>
#include <cstdlib>
#include <cstring>

int main(int argc, char **argv)
{
    int frameLimit = 0;
    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "--data-dir") && i + 1 < argc) {
            std::error_code error;
            std::filesystem::current_path(argv[++i], error);
            if (error) { SDL_Log("Cannot open data directory: %s", error.message().c_str()); return 1; }
        } else if (!strcmp(argv[i], "--frames") && i + 1 < argc) {
            char *end;
            const long value = strtol(argv[++i], &end, 10);
            if (*end || value <= 0 || value > 10000000) { SDL_Log("Invalid frame count."); return 1; }
            frameLimit = int(value);
        } else {
            printf("Usage: %s [--data-dir PATH] [--frames N]\n", argv[0]);
            return strcmp(argv[i], "--help") ? 1 : 0;
        }
    }
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        SDL_Log("SDL initialization failed: %s", SDL_GetError());
        return 1;
    }
    bool success;
    {
        th5w::CGame game;
        success = game.Run(frameLimit);
    }
    SDL_Quit();
    return success ? 0 : 1;
}
