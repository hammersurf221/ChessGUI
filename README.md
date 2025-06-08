# ChessGUI

**ChessGUI** is a desktop chessboard viewer and analyzer that combines a C++/Qt graphical interface with Python-based helpers for real-time board recognition and engine analysis. It uses OpenCV and a custom neural network model to detect chess positions from screen captures, then evaluates them using [Stockfish](https://stockfishchess.org/). It can optionally automate the best move with natural mouse gestures.

---

## 🔍 Features

- 📷 **Automatic board detection** using OpenCV and a PyTorch model  
- 🧠 **Real-time Stockfish analysis** with configurable depth and intervals  
- 🎯 **Move overlay arrows** for suggested moves  
- 🔁 **Auto-move** feature to play the best move with human-like motion  
- 💾 **Persistent settings** using Qt's `QSettings`  
- 🖥️ **Screen region selector** with optional global hotkeys  
- 🛠️ **Cross-platform** build with CMake and Qt 5/6  

---

## 🏗️ Building the GUI

### Requirements

- Qt 5 or Qt 6
- C++17-compatible compiler
- CMake 3.15 or later
- OpenCV (linked with the project)

### Build Instructions

```bash
git clone https://github.com/yourusername/ChessGUI.git
cd ChessGUI
mkdir -p build
cd build
cmake ..
make -j$(nproc)
```

Run the executable:

```bash
./ChessGUI
```

---

## 🐍 Python Utilities

### Requirements

Install required Python packages:

```bash
pip install torch numpy Pillow scikit-image pyautogui
```

### Run the FEN Tracker

```bash
python3 python/fen_tracker/main.py --color w
```

This starts the neural network server and prints FEN strings as board positions are detected and stabilized.

---

## 📁 Project Structure

```
ChessGUI/
├── assets/              # SVG piece images and GUI assets
├── build/               # CMake build output (not versioned)
├── python/
│   └── fen_tracker/     # Neural net model, FEN prediction scripts
├── main.cpp             # Application entry point
├── mainwindow.*         # Main GUI logic
├── boardwidget.*        # Chessboard display and FEN updates
├── regionselector.*     # Region selection overlay
└── CMakeLists.txt       # CMake build config
```

---

## ⚙️ Configuration

The GUI settings dialog includes:

- Stockfish path
- Neural network model path
- Analysis interval
- Engine depth
- Auto-move delay
- Auto-detection toggle

Settings are saved using `QSettings`. You can reset all options via the "Reset to Defaults" button.

Enable debug logging:

```bash
export CHESSGUI_DEBUG_DETECT=1
```

---

## 🧠 How It Works

1. Select (or auto-detect) a screen region containing the chessboard.
2. The GUI periodically captures screenshots.
3. The Python server uses a CNN to predict the board state.
4. A FEN string is sent back to the GUI.
5. Stockfish evaluates the position and suggests the best move.
6. If enabled, the move is played automatically using `pyautogui`.

---

## ✅ Example: End-to-End Flow

```bash
# Start the Python FEN tracker
python3 python/fen_tracker/main.py --color w

# Build and run the GUI
cd build
./ChessGUI
```

Then:
- Select a board region in the GUI.
- Click "Start Analysis".
- The application will display move suggestions and optionally play them.


---

## TROUBLESHOOTING:
- Status repeatedly showing "Stockfish Crashed" --> Stockfish.exe is incompatible with your processor. 
Visit Stockfish download page and download the correct version for your device. Extract folder, rename "XXXXXXXXX.exe" file to "stockfish.exe", move to FENgineLive folder.
- Start Analysis button not working (nothing happens) --> Writeable temp directories aren't working. Run app as administrator.

## 🙌 Acknowledgments

- [Stockfish](https://stockfishchess.org/) — world-class chess engine
- [PyTorch](https://pytorch.org/) — deep learning framework
- [OpenCV](https://opencv.org/) — computer vision library
- [Qt](https://www.qt.io/) — cross-platform GUI toolkit
