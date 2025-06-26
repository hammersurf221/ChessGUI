# FENgineLive Project Structure

This document provides a comprehensive overview of the FENgineLive project structure, explaining the purpose and organization of each component.

## 📁 Root Directory Structure

```
ChessGUI/
├── 📄 CMakeLists.txt          # Main CMake configuration
├── 📄 main.cpp                # Application entry point
├── 📄 mainwindow.h/cpp        # Main GUI window implementation
├── 📄 boardwidget.h/cpp       # Chess board rendering widget
├── 📄 arrowoverlay.h/cpp      # Move arrow overlay widget
├── 📄 settingsdialog.h/cpp    # Settings configuration dialog
├── 📄 regionselector.h/cpp    # Board region selection widget
├── 📄 chessboard_detector.h/cpp # OpenCV-based board detection
├── 📄 globalhotkeymanager.h/cpp # System-wide hotkey management
├── 📄 telemetrymanager.h/cpp  # Move telemetry and statistics
├── 📄 telemetrydashboardv2.h/cpp # Telemetry visualization
├── 📄 stealthmoveselector.h/cpp # Stealth mode move selection
├── 📄 mainwindow.ui           # Qt Designer UI file
├── 📄 README.md               # Project documentation
├── 📄 .gitignore              # Git ignore rules
├── 📄 build.bat               # Windows build script
├── 📄 build.sh                # Unix build script
├── 📄 setup_dev.py            # Development environment setup
├── 📄 PROJECT_STRUCTURE.md    # This file
├── 📁 assets/                 # Application resources
├── 📁 external/               # External dependencies
├── 📁 readme/                 # Documentation assets
└── 📁 build/                  # Build output (generated)
```

## 🎨 Assets Directory

```
assets/
├── 📁 fonts/
│   └── 📄 Inter-Regular.ttf   # Custom font for UI
├── 📁 pieces/
│   ├── 📄 wK.svg              # White king piece
│   ├── 📄 wQ.svg              # White queen piece
│   ├── 📄 wR.svg              # White rook piece
│   ├── 📄 wB.svg              # White bishop piece
│   ├── 📄 wN.svg              # White knight piece
│   ├── 📄 wP.svg              # White pawn piece
│   ├── 📄 bK.svg              # Black king piece
│   ├── 📄 bQ.svg              # Black queen piece
│   ├── 📄 bR.svg              # Black rook piece
│   ├── 📄 bB.svg              # Black bishop piece
│   ├── 📄 bN.svg              # Black knight piece
│   └── 📄 bP.svg              # Black pawn piece
└── 📄 style.qss               # Qt stylesheet for dark theme
```

## 🔧 External Dependencies

```
external/
├── 📁 chess_mouse_mover/      # Machine learning models
│   ├── 📄 best_model.pth      # Trained transformer model
│   ├── 📁 checkpoints/        # Model checkpoints
│   ├── 📁 models/             # Model architecture definitions
│   │   ├── 📄 __init__.py
│   │   ├── 📄 losses.py       # Loss functions
│   │   └── 📄 transformer_model.py # Transformer architecture
│   ├── 📄 mouse_path_generator.py # Path generation utilities
│   └── 📄 requirements.txt    # Python dependencies
├── 📁 python/sable/          # SABLE transformer engine
│   ├── 📄 engine.py          # Best move predictor
│   └── 📄 README.md          # Model description
└── 📁 python/                 # Python runtime and scripts
    ├── 📄 python.exe         # Bundled Python interpreter
    ├── 📄 requirements.txt   # Python dependencies
    ├── 📁 fen_tracker/       # FEN prediction and automation
    │   ├── 📄 __init__.py
    │   ├── 📄 main.py        # FEN prediction server
    │   ├── 📄 play_move.py   # Mouse movement automation
    │   ├── 📄 human_mouse.py # Human-like mouse simulation
    │   ├── 📄 ccn_model.py   # Convolutional Chess Network
    │   ├── 📄 ccn_model_default.pth # Trained CCN model
    │   ├── 📁 core/          # Core FEN tracking logic
    │   ├── 📁 utils/         # Utility functions
    │   └── 📁 trajectories/  # Mouse trajectory data
    └── 📁 [Python runtime files] # Bundled Python installation
```

## 🏗️ Build System

### CMake Configuration (`CMakeLists.txt`)

The main CMake configuration handles:

- **Qt Integration**: Automatic MOC, UIC, and RCC processing
- **OpenCV Linking**: Computer vision library integration
- **Platform Configuration**: Windows, macOS, and Linux support
- **Installation**: CPack-based installer generation
- **Dependencies**: Runtime file bundling and deployment

### Build Scripts

- **`build.bat`**: Windows batch script for Visual Studio builds
- **`build.sh`**: Unix shell script for Linux/macOS builds
- **`setup_dev.py`**: Python script for development environment setup

## 🧠 Core Components

### Main Application (`main.cpp`)

- Application initialization and configuration
- High DPI support and OpenGL setup
- Logging system configuration
- Error handling and crash recovery
- Theme and font loading

### Main Window (`mainwindow.h/cpp`)

- Primary GUI container and layout
- Screenshot capture and processing
- Engine communication (UCI protocol)
- FEN prediction coordination
- Auto-move and stealth mode logic
- Telemetry and statistics tracking

### Board Widget (`boardwidget.h/cpp`)

- Chess position visualization
- Piece rendering with SVG graphics
- Board theme and colors
- Arrow overlay integration
- FEN parsing and board state management

### Computer Vision (`chessboard_detector.h/cpp`)

- OpenCV-based board detection
- Contour analysis and rectangle detection
- Line detection for board boundaries
- Debug image generation
- Auto-detection with manual fallback

### Automation (`play_move.py`)

- Human-like mouse movement simulation
- Transformer model integration
- Path generation and bezier curves
- Timing and delay simulation
- Error handling and logging

## 🔄 Data Flow

```
Screenshot Capture → FEN Prediction → Engine Analysis → Move Selection → Automation
       ↓                    ↓              ↓              ↓              ↓
   OpenCV Processing → CCN Model → SABLE Engine → Stealth Logic → PyAutoGUI
```

## 📊 Telemetry System

### Components

- **`telemetrymanager.h/cpp`**: Core telemetry collection and storage
- **`telemetrydashboardv2.h/cpp`**: Real-time statistics visualization
- **`stealthmoveselector.h/cpp`**: Stealth mode move selection logic

### Data Collection

- Move selection patterns
- Engine evaluation changes
- Timing statistics
- Error rates and performance metrics

## 🎯 Key Features Implementation

### Real-time Analysis

1. **Screenshot Capture**: Periodic screen capture using Qt
2. **Board Detection**: OpenCV-based region detection
3. **FEN Prediction**: CCN model inference
4. **Engine Analysis**: UCI protocol communication
5. **Result Display**: Real-time GUI updates

### Stealth Mode

1. **Temperature Sampling**: Configurable randomness
2. **Second-line Injection**: Percentage-based alternative moves
3. **Telemetry Logging**: Move selection tracking
4. **History Analysis**: Pattern detection and statistics

### Auto-Move

1. **Path Generation**: Transformer model for human-like paths
2. **Mouse Simulation**: PyAutoGUI-based automation
3. **Timing Control**: Realistic delays and pauses
4. **Error Recovery**: Retry logic and fallback mechanisms

## 🔧 Configuration

### Settings Dialog

- Analysis interval configuration
- Engine path and parameters
- Stealth mode settings
- Hotkey assignments
- Performance options

### File Locations

- **Settings**: Qt application settings (platform-specific)
- **Telemetry**: `telemetry_log.json` (rotated at 5MB)
- **Models**: Bundled with application
- **Logs**: Application logs and debug output

## 🚀 Deployment

### Build Process

1. **Configuration**: CMake generates build files
2. **Compilation**: C++ compilation with Qt and OpenCV
3. **Bundling**: Runtime dependencies copied to build directory
4. **Packaging**: CPack creates platform-specific installers

### Distribution

- **Windows**: NSIS installer with all dependencies
- **macOS**: DMG package with Qt framework bundling
- **Linux**: Tarball with system dependency instructions

## 🛠️ Development Guidelines

### Code Organization

- **Header Files**: Class declarations and interfaces
- **Source Files**: Implementation and logic
- **UI Files**: Qt Designer layouts
- **Resources**: Assets and configuration files

### Naming Conventions

- **Classes**: PascalCase (e.g., `MainWindow`)
- **Functions**: camelCase (e.g., `captureScreenshot`)
- **Variables**: camelCase (e.g., `analysisRunning`)
- **Constants**: UPPER_SNAKE_CASE (e.g., `DEFAULT_INTERVAL`)

### Error Handling

- **Logging**: Structured logging with categories
- **Exceptions**: C++ exception handling for critical errors
- **Recovery**: Automatic restart for crashed processes
- **User Feedback**: Status messages and error dialogs

This structure provides a solid foundation for a production-ready chess analysis application with clear separation of concerns, comprehensive error handling, and maintainable code organization. 