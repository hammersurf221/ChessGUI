# **WARNING: THIS PROJECT IS NOT OPTIMIZED. USING IT MEANS YOU ACCEPT THAT YOU MAY BE BANNED ON ONLINE CHESS PLATFORMS REGARDLESS OF WHETHER YOU USE STEALTH MODE OR NOT. IF YOU WOULD LIKE TO KEEP YOUR ACCOUNT AND NOT BE FLAGGED FOR VIOLATING FAIR PLAY POLICIES AND TOS'S, DO NOT USE**





<!-- PROJECT HEADER -->
<div align="center">
  <img src="readme/assets/logos/FENgineLive_Logo.png" width="240" alt="FENgineLive Logo">
  <h1>FENgineLive</h1>
  <p><em>Real-time chess analysis with AI assistance</em></p>
  
  [![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
  [![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20macOS%20%7C%20Linux-blue.svg)]()
  [![Qt](https://img.shields.io/badge/Qt-5.15%2B%20%7C%206.x-blue.svg)]()
</div>

---

## ⚠️ Important Disclaimer

**This software is provided for educational and analytical purposes only.**

The authors do not condone, encourage, or support using FENgineLive in any manner that violates:
- Terms of Service of online chess platforms
- Fair-play policies
- Tournament rules
- Federation regulations

**Always respect the regulations of the site or event you are playing on.** Using Auto-Move, Stealth Mode, or any other feature to gain an unfair competitive advantage may result in account suspension or other penalties.

---

## 📋 Table of Contents

1. [Overview](#overview)
2. [Features](#features)
3. [System Requirements](#system-requirements)
4. [Installation](#installation)
5. [Usage](#usage)
6. [Configuration](#configuration)
7. [Troubleshooting](#troubleshooting)
8. [Development](#development)
9. [License](#license)

---

## 🎯 Overview

FENgineLive is a hybrid **Qt (C++17)** + **Python** application that provides real-time chess analysis using computer vision and AI engines. It captures screenshots of chess interfaces, converts positions to FEN notation using a Convolutional Chess Network (CCN), and analyzes them with the SABLE transformer engine.

### Key Capabilities

- **Real-time Vision**: CCN predicts 8×8 grid of piece classes from 256×256 RGB crops
- **AI Analysis**: Integration with the SABLE transformer engine for position evaluation
- **Human-like Automation**: Advanced mouse movement simulation with transformer models
- **Stealth Mode**: Configurable move selection with temperature-based randomization
- **Cross-platform**: Windows, macOS, and Linux support

---

## ✨ Features

| Category | Features |
|----------|----------|
| **Computer Vision** | • Real-time board detection<br>• FEN position recognition<br>• Multiple theme support<br>• Auto-detection with manual fallback |
| **Engine Integration** | • SABLE transformer backend<br>• UCI-like text protocol<br>• Multi-PV analysis |
| **Automation** | • Human-like mouse movement<br>• Transformer-based path generation<br>• Drag-and-drop simulation<br>• Configurable delays |
| **Stealth Features** | • Temperature-based move selection<br>• Second-line injection<br>• Telemetry logging<br>• Move history tracking |
| **User Interface** | • Modern Qt-based GUI<br>• Real-time evaluation display<br>• Move history visualization<br>• Global hotkey support |
| **Performance** | • Optimized screenshot capture<br>• Efficient FEN processing<br>• Memory management<br>• Crash recovery |

---

## 💻 System Requirements

### Minimum Requirements

| Component | Version |
|-----------|---------|
| **OS** | Windows 10, macOS 10.15+, Ubuntu 18.04+ |
| **CPU** | Intel i5 / AMD Ryzen 5 or equivalent |
| **RAM** | 4 GB |
| **GPU** | Integrated graphics (CPU mode) |
| **Storage** | 2 GB free space |

### Recommended Requirements

| Component | Version |
|-----------|---------|
| **OS** | Windows 11, macOS 12+, Ubuntu 20.04+ |
| **CPU** | Intel i7 / AMD Ryzen 7 or equivalent |
| **RAM** | 8 GB |
| **GPU** | NVIDIA GTX 1060+ / AMD RX 580+ (CUDA support) |
| **Storage** | 5 GB free space (SSD recommended) |

### Software Dependencies

| Component | Version | Notes |
|-----------|---------|-------|
| **Qt** | 5.15+ or 6.x | Core GUI framework |
| **OpenCV** | 4.x | Computer vision |
| **Python** | 3.8+ | ML and automation |
| **SABLE** | 1.0+ | Transformer engine |

---

## 🚀 Installation

### Prerequisites

1. **Install Qt Development Environment**
   ```bash
   # Windows: Download Qt installer from qt.io
   # macOS: brew install qt
   # Linux: sudo apt install qt6-base-dev
   ```

2. **Install OpenCV**
   ```bash
   # Windows: Download from opencv.org or use vcpkg
   # macOS: brew install opencv
   # Linux: sudo apt install libopencv-dev
   ```

3. **Install Python Dependencies**
   ```bash
   cd external/python
   pip install -r requirements.txt
   ```

### Building from Source

1. **Clone the Repository**
   ```bash
   git clone https://github.com/your-org/FENgineLive.git
   cd FENgineLive
   ```

2. **Configure Build**
   ```bash
   # Create build directory
   mkdir build && cd build
   
   # Configure with CMake
   cmake .. -DCMAKE_BUILD_TYPE=Release
   ```

3. **Build the Application**
   ```bash
   # Build with multiple cores
   cmake --build . --parallel $(nproc)
   
   # Or on Windows with Visual Studio
   cmake --build . --config Release
   ```

4. **Install Dependencies**
   ```bash
   # Copy SABLE engine scripts
   cp -r ../external/python/sable ./python/

   # Copy FEN tracker scripts
   cp -r ../external/python/fen_tracker/* ./python/
   ```

### Quick Start

1. **Launch the Application**
   ```bash
   # Linux/macOS
   ./ChessGUI
   
   # Windows
   ChessGUI.exe
   ```

2. **First Run Setup**
   - Accept board auto-detection or manually select region
   - Configure engine path in Settings
   - Set analysis interval and other preferences

---

## 🎮 Usage

### Basic Operation

1. **Start Analysis**
   - Press `Ctrl+A` (default) to toggle analysis
   - The app captures screenshots at configured intervals
   - FEN prediction and engine analysis run automatically

2. **View Results**
   - Best move arrow appears on the board
   - Evaluation bar shows position strength
   - Move history displays recent analysis

3. **Stealth Mode**
   - Press `Ctrl+S` to enable stealth mode
   - Moves are selected with configurable randomness
   - Telemetry logs track move selections

4. **Auto-Move**
   - Press `Ctrl+M` to enable auto-move
   - Human-like mouse movements execute moves
   - Works with most chess interfaces

### Advanced Features

- **Global Hotkeys**: Configure system-wide shortcuts
- **Telemetry Dashboard**: Monitor stealth mode statistics
- **Engine Strength**: Transformer-based move prediction
- **Board Detection**: Auto-detect or manual region selection

---

## ⚙️ Configuration

### Settings Dialog

Access via **Settings** menu or `Ctrl+,`:

| Setting | Description | Default |
|---------|-------------|---------|
| **Analysis Interval** | Screenshot capture frequency | 1000ms |
| **Engine Path** | Path to SABLE engine script | `./python/sable/engine.py` |
| **Engine Depth** | Analysis depth | 15 |
| **Stealth Temperature** | Move randomness (0.0-1.0) | 0.035 |
| **Second-line Injection** | Percentage of second-best moves | 10% |

### Configuration Files

- **Settings**: Stored in user preferences
- **Telemetry**: `telemetry_log.json` (rotated at 5MB)
- **Hotkeys**: Configurable via Settings dialog

---

## 🔧 Troubleshooting

### Common Issues

| Problem | Solution |
|---------|----------|
| **"Waiting for FEN" never disappears** | Check model path in Settings → Model Path |
| **Incorrect FEN prediction** | Use compatible chess theme (Icy Sea on Chess.com) |
| **Board not detected** | Manually set board region via Capture Region |
| **Auto-Move clicks wrong place** | Re-run Capture Region after window scaling changes |
| **No engine output** | Verify SABLE script path and model file |
| **Hotkeys not working** | Global hotkeys are Windows-only; use menu toggles |

### Debug Mode

Enable debug output:
```bash
# Set environment variable
export CHESSGUI_DEBUG=1

# Or on Windows
set CHESSGUI_DEBUG=1
```

### Performance Optimization

- **Reduce analysis interval** for faster response
- **Lower engine depth** for quicker analysis
- **Use GPU acceleration** if available
- **Close unnecessary applications** to free resources

---

## 🛠️ Development

### Project Structure

```
ChessGUI/
├── src/                    # C++ source files
│   ├── main.cpp           # Application entry point
│   ├── mainwindow.cpp     # Main GUI implementation
│   ├── boardwidget.cpp    # Chess board rendering
│   └── ...
├── external/              # External dependencies
│   ├── python/           # Python scripts
│   ├── lc0/              # Chess engine
│   └── chess_mouse_mover/ # ML models
├── assets/               # Resources
│   ├── pieces/           # Chess piece SVGs
│   ├── fonts/            # Custom fonts
│   └── style.qss         # Qt stylesheet
└── docs/                 # Documentation
```

### Building for Development

```bash
# Debug build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build . --parallel

# With sanitizers
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined"
```

### Code Style

- **C++**: Follow Qt coding conventions
- **Python**: PEP 8 with 4-space indentation
- **Comments**: Use Doxygen-style for C++ classes
- **Error Handling**: Comprehensive error checking and logging

### Testing

```bash
# Run unit tests (when implemented)
ctest --output-on-failure

# Run Python tests
cd external/python/fen_tracker
python -m pytest tests/
```

---

## 📄 License

FENgineLive is released under the **GNU General Public License v3.0**.

See [LICENSE](https://www.gnu.org/licenses/gpl-3.0.en.html) for details.

### Third-party Licenses

- **Qt**: LGPL v3 / GPL v3
- **OpenCV**: Apache 2.0
- **PyTorch**: BSD 3-Clause
 - **SABLE**: Proprietary

---

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

### Development Guidelines

- Follow existing code style and conventions
- Add tests for new features
- Update documentation as needed
- Ensure cross-platform compatibility

---

## 📞 Support

- **Issues**: [GitHub Issues](https://github.com/your-org/FENgineLive/issues)
- **Discussions**: [GitHub Discussions](https://github.com/your-org/FENgineLive/discussions)
- **Email**: williamsamiri011@gmail.com

---

<div align="center">
  <p><em>Built with ❤️ for the chess community</em></p>
</div>
