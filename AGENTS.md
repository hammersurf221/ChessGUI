# ChessGUI Project Guide

## Overview
This repository contains a C++/Qt application to display a chessboard GUI and interact with a Stockfish engine. Additional Python scripts provide functionality for chessboard recognition and FEN string prediction using PyTorch models. The main folders are:

- **C++/Qt source**: `main.cpp`, `mainwindow.*`, `boardwidget.*`, `arrowoverlay.*`, `regionselector.*`, and related `.ui` files. Build configuration is in `CMakeLists.txt`.
- **Python scripts**: located in `python/` with modules for dataset handling, neural network inference, and FEN tracking.
- **Assets**: images, fonts, and style sheets used by the GUI under `assets/`.
- **Build directory**: `build/` (not tracked) stores CMake build output.

## Build Instructions
1. Install Qt 5 or Qt 6 along with CMake and a C++17 compatible compiler.
2. From the repository root:
   ```bash
   mkdir -p build
   cd build
   cmake ..
   make -j$(nproc)
   ```
3. The resulting executable `ChessGUI` will be found in the `build/` directory.

### Running Python Components
Python utilities require Python 3 with packages such as `torch`, `numpy`, `Pillow`, and `scikit-image`.
Example usage:
```bash
python3 python/fen_server-workingV2.py
```
Check individual scripts for their command line arguments.

## Runtime Settings
The application remembers your preferences between sessions. Open **File → Settings** (**Preferences** on macOS) to adjust options such as:

- Stockfish search depth and analysis interval
- Automatic board detection and manual region selection
- Paths to the Stockfish executable and FEN prediction model
- Whether to automatically play the best move and the delay before doing so
- A "stealth mode" toggle to hide overlays
- Default player color

Changes persist automatically thanks to `QSettings`. A **Reset to Defaults** button
is provided to restore the built‑in values at any time.

## Testing
The project does not currently include automated tests. If tests are added, they can be run after building with:
```bash
ctest
```

## Coding Conventions
- Use 4 spaces for indentation.
- Place opening braces on the same line as the control statement or function definition.
- Prefer modern C++17 features when possible.
- Keep headers and source files in sync with forward declarations.

## Contributing
1. Create a feature branch from `main`.
2. Follow the coding conventions above and provide clear commit messages.
3. Ensure the project builds without warnings before opening a pull request.
4. Include any new assets or scripts in the appropriate directories.
