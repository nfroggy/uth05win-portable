# Touhou 5: Mystic Square — SDL3 port

I wanted to play Pegasy's Windows port of Touhou 5 on Linux, so I had Chatgpt Codex port it to SDL3. It seems to work well so I decided to put it on Github in case someone else finds this useful.

## How to build

On Fedora:

```sh
sudo dnf install gcc-c++ cmake ninja-build git SDL3-devel libglvnd-devel mesa-libGLU-devel freetype-devel fontconfig-devel
```

From the repository root:

```sh
git submodule update --init --recursive
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

## Game data

From the PC98 game:

- `kaiki1.dat` (renamed from 怪綺談1.dat)
- `kaiki2.dat` (renamed from 怪綺談2.dat)
- `zun.com` (lowercase filename on Linux)

The `SE/`, `practice.bmp`, `replay.bmp`, and configuration files from this repo should be in the same directory as the data files. `anex86.bmp` is optional; when absent, FreeType and Fontconfig provide system fonts.

To run from another directory:

```sh
/path/to/build/uth05win --data-dir /path/to/game-data
```

Configuration, high scores, and replays are saved in the game data directory,
which must be writable. The included Chinese translation is selected by
`MOD_FILE = chs.dat` in `config.ini`. Comment out that line with `//` to use the
original Japanese text. Startup errors are printed to the terminal.

## Controls

| Key | Action |
| --- | --- |
| Arrow keys | Move / navigate menus |
| Z | Shoot / confirm |
| X | Bomb / cancel |
| Left Shift | Move slowly |
| Left Ctrl | Skip dialogue / fast-forward replays |
| Esc | Pause menu |
| Alt + Enter | Toggle fullscreen |
