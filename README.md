# ChessGUI

ChessGUI is a desktop application written in C++ using Qt that displays a chessboard and communicates with the Stockfish engine for move analysis.  Python helper scripts perform chessboard image recognition and FEN prediction with PyTorch models.

## Prerequisites
- **Qt 5 or Qt 6**, a C++17 compiler and CMake for building the GUI
- **Python 3** with packages such as `torch`, `numpy`, `Pillow`, and `scikit-image` for the optional recognition utilities

## Building the GUI
Follow the steps from `AGENTS.md`:
```bash
mkdir -p build
cd build
cmake ..
make -j$(nproc)
```
The resulting executable `ChessGUI` will appear in the `build/` directory.

## Running the GUI
After building, launch the application:
```bash
./build/ChessGUI
```
The GUI will start with options to select a board region, toggle automated move analysis, and display Stockfish suggestions.

## Settings
Open **File → Settings** (**Preferences** on macOS) to customize runtime options.
The dialog lets you configure Stockfish depth, analysis interval, automatic board
detection, manual region selection, and paths to the Stockfish executable and FEN
prediction model. You can also enable stealth mode, change the default player
color, and set an auto‑move delay. All choices persist between launches, and a
**Reset to Defaults** button restores the original values.

### Debugging Chessboard Detection
Set the environment variable `CHESSGUI_DEBUG_DETECT=1` before launching the
application to save intermediate images from the automatic board detection
routine.  The images will appear in the working directory as
`detect_debug_*.png` files showing the grayscale input, edge map, detected lines
and final rectangle.

## Python Utilities
The `python/` folder contains scripts used for board recognition and automation:
- `fen_server-workingV2.py` – standalone FEN prediction server
- `fen_server-working.py` and `fen_server-slow.py` – alternative server variants
- `fen_predictor.py` – command‑line FEN predictor for a single image
- `fen_tracker/` – real‑time tracker and automation helpers such as `main.py` and `play_move.py`

Example invocation from `AGENTS.md`:
```bash
python3 python/fen_server-workingV2.py
```
Check each script for its specific arguments.

## Testing
The project does not include automated tests, but if any are added they can be run with `ctest` after building.
