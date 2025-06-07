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
