# ChessGUI

ChessGUI is a cross-platform desktop application that displays a digital chessboard and integrates with the Stockfish engine to provide move analysis. Additional Python tools enable automatic board recognition and automation features.

## Features

- Interactive chessboard interface rendered with Qt
- Stockfish engine integration for real-time analysis
- Arrow overlays showing suggested moves
- Automatic board detection using OpenCV
- Optional automation to play the best move after a delay
- Persistent settings stored via QSettings
- Python-based FEN prediction and utilities

## Directory Overview

- **C++/Qt Source** – application code in `main.cpp`, `mainwindow.*`, `boardwidget.*`, `regionselector.*`, and related headers
- **Python Utilities** – board recognition and automation scripts in `python/`
- **Assets** – piece images, fonts and style sheet under `assets/`
- **Stockfish Engine** – a `stockfish.exe` binary is included, but you may specify a custom engine path in the settings
- **Build Output** – create a `build/` directory for CMake build files (not tracked)

## Building

1. Install Qt 5 or Qt 6, a C++17 capable compiler and CMake
2. From the repository root run:

```bash
mkdir -p build
cd build
cmake ..
make -j$(nproc)
```

The resulting executable will be located in the `build/` folder.

### Building on Windows

Use the Qt/MSVC environment or MinGW. Open a Qt command prompt and invoke the same CMake commands as above.

## Running

Launch the compiled binary from the build directory:

```bash
./ChessGUI
```

Upon first launch you can manually select a screen region for the chessboard or enable automatic detection. The main window shows Stockfish evaluations and suggested moves. Arrow overlays indicate the best line on the board.

## Configuration

Open **File → Settings** to configure runtime options:

- **Analysis Interval** – delay between Stockfish evaluations
- **Search Depth** – depth passed to the engine
- **Automatic Board Detection** – toggle OpenCV based detection
- **Force Manual Region** – always prompt for a region
- **Auto Play Best Move** – automatically execute the best move using the Python helper
- **Auto Move Delay** – wait time before executing the move
- **Stealth Mode** – hide overlays during automation
- **Stockfish Path** – location of the Stockfish binary
- **FEN Model Path** – path to the PyTorch model for board recognition
- **Default Player Color** – determines orientation and automation color

Settings persist between runs and can be restored to defaults from the dialog.

You can enable extra debug output for board detection by setting the environment variable `CHESSGUI_DEBUG_DETECT=1`. Intermediate images will be saved as `detect_debug_*.png`.

## Python Utilities

The `python/fen_tracker` package provides neural network based board recognition and automation scripts.

### Dependencies

Install Python 3 packages:

```bash
pip install torch torchvision pillow numpy scikit-image pyautogui
```

### FEN Prediction Server

Run the FEN server to stream board positions:

```bash
python3 python/fen_tracker/main.py --color w
```

It prints FEN strings to standard output when a new board state is detected.

### Automated Move Execution

`python/fen_tracker/play_move.py` moves pieces with realistic mouse drags. Usage:

```bash
python3 python/fen_tracker/play_move.py e2 e4 100 100 64 false
```

Arguments specify the origin square, destination square, top-left board coordinates, tile size and whether the board is flipped.

## Development Guidelines

- Use four spaces for indentation
- Place opening braces on the same line as the control statement or function
- Prefer modern C++17 features
- Keep headers and source files synchronized

Contributions are welcome. Please open pull requests from feature branches and ensure the project builds without warnings.
