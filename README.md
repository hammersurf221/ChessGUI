# **WARNING: THIS PROJECT IS NOT OPTIMIZED. USING IT MEANS YOU ACCEPT THAT YOU MAY BE BANNED ON ONLINE CHESS PLATFORMS REGARDLESS OF WHETHER YOU USE STEALTH MODE OR NOT. IF YOU WOULD LIKE TO KEEP YOUR ACCOUNT AND NOT BE FLAGGED FOR VIOLATING FAIR PLAY POLICIES AND TOS'S, DO NOT USE**





<!-- PROJECT HEADER -->
<p align="center">
  <img src="readme/assets/logos/FENgineLive_Logo.png" width="240" alt="FENgineLive Logo">
</p>

<h1 align="center">FENgineLive</h1>
<p align="center"><em>Real-time chess-board recognition · Maia (Lc0) analysis · On-screen & in-game assistance</em></p>

---

## Table of Contents
1. [Overview](#overview)  
2. [Features](#features)  
3. [Requirements](#requirements)  
4. [Installation](#installation)  
5. [Usage](#usage)  
6. [Troubleshooting & Debugging](#troubleshooting--debugging)  
7. [Screenshots & GIFs](#screenshots--gifs)  
8. [Learning Resources](#learning-resources)  
9. [License](#license)  

---

## Overview
**FENgineLive** is a hybrid **Qt (C++17)** + **Python** application that captures periodic screenshots of any chess interface, converts the current position to **FEN** with a custom [*Convolutional Chess Network*](https://github.com/hammersurf221/FENgine) (CCN), and feeds it to **Maia** using the **Lc0** backend.

The GUI then:

* Renders the position in its own board widget  
* Draws an arrow for Maia’s best move
* Shows evaluation, move history, and the raw FEN
* Offers **Stealth Mode** that limits search depth and chooses moves with human-like randomness (optional)
* Can **Auto-Move** the chosen move directly on the board (optional)  

Use it for real-time tactics training, stream overlays, or hands-free auto-playing—completely client-side.

---

## Features

| Category | Highlights |
|----------|------------|
| **Real-time Vision** | CCN predicts an 8 × 8 grid of piece classes from 256 × 256 RGB crops. Currently only works with specific themes. You can train your own weights at  https://github.com/hammersurf221/FENgine|
| **Maia/Lc0 Integration** | UCI handshake with backend options and evaluation parsing. |
| **Stealth Mode** | Uses shallow depth (e.g., 6 plies) then picks from the top line via softmax with human delays. |
| **Auto-Move** | Uses `pyautogui` to click the recommended move on your chess site—works with Lichess/Chess.com & most GUI boards. Toggle on/off any time. |
| **Telemetry Dashboard** | Records stealth moves to `telemetry_log.json` and shows real-time stats in a dockable widget. |
| **Region Auto-Detect + Manual Fallback** | Detects the chessboard rectangle via OpenCV; cancel to draw region manually. |
| **Global Hotkeys** | Toggle analysis, stealth, auto-move, overlays without leaving your game. |
| **Cross-Platform** | Builds on Windows, macOS, and Linux with Qt 5/6 + CMake; Python 3.8 + runtime bundled or system-wide. |

---

## Requirements

### Runtime

| Component | Minimum Version |
|-----------|-----------------|
| **Lc0** | 0.29 + |
| **Qt** (Widgets, GUI, Core) | 5.15 or 6.x |
| **OpenCV** | 4.x |
| **Python** | 3.8 + |
| Python libs | `torch >= 2.0`, `torchvision`, `numpy`, `Pillow`, `scikit-image`, `pyautogui` |
| **GPU** | *Optional* – CCN runs on CPU |

> **Security note for Auto-Move:**  
> Auto-Move uses `pyautogui` to control your mouse and keyboard. Only enable it when you trust the app’s window focus and are comfortable with automated clicks.

### Build-time

* CMake ≥ 3.16  
* C++17-compliant compiler (MSVC 2022 / Clang 15 / GCC 11 +)  
* Ninja (optional but faster)

---

## Installation
~~~bash
# 1. Clone
git clone https://github.com/<your-org>/FENgineLive.git
cd FENgineLive

# 2. Build the Qt/C++ binary
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel

# 3. Install Python deps
python -m pip install -r python/requirements.txt

# 4. Copy or symlink your Lc0 executable and Maia network,
#    then launch the GUI:
./build/FENgineLive            # Linux/macOS
build\Release\FENgineLive.exe  # Windows
~~~

> **First run:** FENgineLive prompts to auto-detect the board.  
> Accept, or cancel to draw a region manually.  
> You can re-run detection any time via **Capture Region**.
>
> **Disclaimer:** Requirements.txt is in the process of being made. For now, the project uses the following dependencies:
> torch
> Pillow
> numpy
> 

---

## Usage
1. **Open a chess site or GUI** and start a game.  
2. Launch **FENgineLive** – a translucent overlay appears.  
3. Press **`Ctrl + A`** (default) to start/stop analysis.  
4. The app captures a screenshot every *N ms* (set in **Settings → Analysis Interval**), predicts the FEN, and queries Maia via Lc0.
5. Watch the best-move arrow, evaluation bar, and PGN history update in real-time.  
6. Toggle **Stealth Mode** (*`Ctrl + S`*) to enable the humanized move picker.
7. Toggle **Auto-Move** (*`Ctrl + M`*) if you’d like the app to physically play the move on your board.  
8. Use **Reset Game** when starting a new game.

---

## Troubleshooting & Debugging

| Symptom | Fix |
|---------|-----|
| **“Waiting for FEN” never disappears** | Verify `ccn_model_default.pth` is present and its correct path is set in **Settings → Model Path** |
| **Predicted FEN is incorrect** | You may be using a model weight trained on a different theme than the one you are currently using. Simply use a basic chess.com board and the "Icy Sea" theme on Chess.com. |
| **Board not detected or wrong size** | For now, manually set your board region. Board autodetection is in the process of being optimized. |
| **Auto-Move clicks in the wrong place** | Ensure your browser window is the same scale when you captured the region; re-run **Capture Region**. |
| **No engine output** | Check **Settings → Engine Path** and make sure you are using the correct path to **lc0.exe** and that the Maia network file exists. |
| **Hotkeys do nothing on macOS/Linux** | Global hotkeys are Windows-only for now; use menu toggles instead. |

A fuller FAQ lives in **docs/TROUBLESHOOTING.md** -> ***STILL BEING MADE***.

---

## Screenshots & GIFs
Currently placeholders.

| Auto-Detected Board | Best-Move Arrow | Stealth Randomisation | Auto-Move Click | Move History & Eval |
|---------------------|-----------------|-----------------------|-----------------|---------------------|
| `assets/screenshots/auto_detect.png` | `assets/gifs/arrow.gif` | `assets/gifs/stealth.gif` | `assets/gifs/automove.gif` | `assets/screenshots/history_eval.png` |

---

## Learning Resources
* **CCN architecture** – see <https://github.com/hammersurf221/FENgine> for model code and residual enhancements.    
* **Lc0 UCI options** – <https://github.com/LeelaChessZero/lc0/wiki/Parameters>
* **Qt Widgets** – <https://doc.qt.io/>  
* **OpenCV 4.x tutorials** – <https://docs.opencv.org/>  

---

## Disclaimer  
This project is provided **for educational and analytical purposes only**.  
The author(s) **do not condone, encourage, or support** using FENgineLive in any manner that violates the Terms of Service, fair-play policies, or rules of any online chess platform, tournament, or federation.  
**Always respect the regulations of the site or event you are playing on.**  
Using Auto-Move, Stealth Mode, or any other feature to gain an unfair competitive advantage is strictly discouraged and may result in account suspension or other penalties.

---


## License
FENgineLive is released under the **GNU General Public License v3.0**.  
See [LICENSE](https://www.gnu.org/licenses/gpl-3.0.en.html) for details.

---
