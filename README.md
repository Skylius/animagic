# Animagic

A Qt6 desktop app for assembling and disassembling GIF/WebP animations on Linux using ImageMagick.

## Features

- **Disassemble** GIF/WebP → still images (PNG/JPEG/BMP/WebP, etc.)
- **Assemble** still images → GIF or WebP with:
  - Loop count, per-frame delay or FPS
  - GIF: palette colors, optimize, dither
  - WebP: quality, method, near-lossless, alpha quality, lossless
- **Per-frame editor**: reorder, duplicate, delete, and edit timings
- **Preview**: play with loop/delay/FPS
- **Profiles**: preset encode settings (e.g., “Discord GIF”, “Low-size WebP”)
- **Metadata**: save and reload `animagic.meta.json` with source + assembly options
- **Background tasks**: cancellable, with unified progress + log and a global task bar
- **Settings**: default folders, backend mode (Magick++/CLI), theme (system/light/dark)
- **Platform helpers**: paths (XDG) and free-space checks
- **Embedded app icon**: `animagic.png` via Qt resource

## Dependencies

- **Qt 6** (Widgets, Gui, Core)
- **ImageMagick 7** with C++ headers (`Magick++`)
- CMake ≥ 3.16
- A C++17 compiler (g++/clang++)
- Linux (primary target)

On Debian/Ubuntu-like:
```bash
sudo apt install -y cmake g++ qt6-base-dev libmagick++-dev
```

## Build

```bash
git clone <your-repo-or-extract-zip> animagic
cd animagic
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . -j
./animagic
```

If Qt or ImageMagick are installed in non-standard paths, pass hints to CMake, e.g.:

```bash
cmake -DCMAKE_PREFIX_PATH=/opt/Qt/6.6.0/gcc_64 ..
```

## CLI Fallback

If Magick++ isn’t available at runtime, the backend falls back to invoking the `magick` (or legacy `convert`) CLI. You can force a mode from **Settings → Backend**.

## Project Layout

```
src/
  app/            # Application bootstrap + theme
  config/         # Defaults + AppSettings (JSON in ~/.config/animagic/settings.json)
  core/           # MagickBackend, Profiles, FrameOps, Timing, Task/TaskRunner, Cancellation
  platform/       # Paths (XDG-aware), SpaceCheck
  ui/
    Common/       # ProgressLogPane, FilePickers, CenteredListView, TaskBar
    Main/         # MainWindow (tabs + global task bar + theme toggle)
    ProjectBrowser/
    DisassemblePage/
    AssemblePage/
    MetadataPage/
    SettingsPage/
  resources/      # resources.qrc + icons/animagic.png
CMakeLists.txt
```

## Notes

- The preview runs on a timer using current per-frame delays; it is not a final fidelity encode preview.
- Free-space checks log a warning if low (won’t block you).

## Troubleshooting

- **Qt not found**: ensure `qt6-base-dev` (or your distro’s equivalent) is installed; set `CMAKE_PREFIX_PATH` if needed.
- **ImageMagick headers**: install `libmagick++-dev`. For CLI-only, you still need the runtime `imagemagick` package.
- **Wayland vs X11**: if you see rendering issues, try `QT_QPA_PLATFORM=xcb ./animagic`.

## License

MIT (placeholder).
# animagic
