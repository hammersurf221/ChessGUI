#!/usr/bin/env python3
"""
FENgineLive Development Environment Setup Script

This script helps set up the development environment by:
1. Installing Python dependencies
2. Setting up pre-commit hooks
3. Creating necessary directories
4. Validating the environment
"""

import os
import sys
import subprocess
import platform
import shutil
from pathlib import Path

def run_command(cmd, check=True, capture_output=False):
    """Run a command and return the result."""
    try:
        result = subprocess.run(cmd, shell=True, check=check, 
                              capture_output=capture_output, text=True)
        return result
    except subprocess.CalledProcessError as e:
        print(f"Command failed: {cmd}")
        print(f"Error: {e}")
        return None

def check_python_version():
    """Check if Python version is compatible."""
    version = sys.version_info
    if version.major < 3 or (version.major == 3 and version.minor < 8):
        print("Error: Python 3.8 or higher is required")
        return False
    print(f"✓ Python {version.major}.{version.minor}.{version.micro} detected")
    return True

def install_python_dependencies():
    """Install Python dependencies."""
    print("\n📦 Installing Python dependencies...")
    
    requirements_file = Path("external/python/requirements.txt")
    if not requirements_file.exists():
        print("Error: requirements.txt not found")
        return False
    
    # Install dependencies
    cmd = f"{sys.executable} -m pip install -r {requirements_file}"
    result = run_command(cmd)
    if result is None:
        return False
    
    print("✓ Python dependencies installed")
    return True

def setup_pre_commit():
    """Set up pre-commit hooks if requested."""
    response = input("\n🔧 Set up pre-commit hooks? (y/N): ").strip().lower()
    if response in ['y', 'yes']:
        print("Installing pre-commit...")
        run_command(f"{sys.executable} -m pip install pre-commit")
        run_command("pre-commit install")
        print("✓ Pre-commit hooks installed")

def create_directories():
    """Create necessary directories if they don't exist."""
    print("\n📁 Creating necessary directories...")
    
    directories = [
        "build",
        "logs",
        "external/python/fen_tracker/__pycache__",
        "external/chess_mouse_mover/checkpoints"
    ]
    
    for directory in directories:
        Path(directory).mkdir(parents=True, exist_ok=True)
    
    print("✓ Directories created")

def validate_environment():
    """Validate the development environment."""
    print("\n🔍 Validating environment...")
    
    # Check for required files
    required_files = [
        "CMakeLists.txt",
        "main.cpp",
        "external/python/fen_tracker/play_move.py",
        "assets/pieces/wK.svg"
    ]
    
    missing_files = []
    for file_path in required_files:
        if not Path(file_path).exists():
            missing_files.append(file_path)
    
    if missing_files:
        print("❌ Missing required files:")
        for file_path in missing_files:
            print(f"  - {file_path}")
        return False
    
    # Check for external dependencies
    external_deps = [
        "external/lc0/lc0.exe",
        "external/python/python.exe"
    ]
    
    missing_deps = []
    for dep_path in external_deps:
        if not Path(dep_path).exists():
            missing_deps.append(dep_path)
    
    if missing_deps:
        print("⚠️  Missing external dependencies (these are optional for development):")
        for dep_path in missing_deps:
            print(f"  - {dep_path}")
    
    print("✓ Environment validation completed")
    return True

def check_build_tools():
    """Check for build tools."""
    print("\n🔨 Checking build tools...")
    
    # Check for CMake
    cmake_result = run_command("cmake --version", check=False, capture_output=True)
    if cmake_result and cmake_result.returncode == 0:
        version_line = cmake_result.stdout.split('\n')[0]
        print(f"✓ {version_line}")
    else:
        print("❌ CMake not found. Please install CMake 3.16 or higher")
        return False
    
    # Check for Qt
    qt_result = run_command("qmake --version", check=False, capture_output=True)
    if qt_result and qt_result.returncode == 0:
        version_line = qt_result.stdout.split('\n')[0]
        print(f"✓ {version_line}")
    else:
        print("⚠️  Qt not found. Please install Qt 5.15+ or 6.x")
    
    # Check for OpenCV
    try:
        import cv2
        print(f"✓ OpenCV {cv2.__version__}")
    except ImportError:
        print("⚠️  OpenCV not found. Please install OpenCV 4.x")
    
    return True

def main():
    """Main setup function."""
    print("🚀 FENgineLive Development Environment Setup")
    print("=" * 50)
    
    # Check Python version
    if not check_python_version():
        sys.exit(1)
    
    # Install Python dependencies
    if not install_python_dependencies():
        sys.exit(1)
    
    # Set up pre-commit hooks
    setup_pre_commit()
    
    # Create directories
    create_directories()
    
    # Check build tools
    if not check_build_tools():
        print("\n⚠️  Some build tools are missing. Please install them to build the project.")
    
    # Validate environment
    if not validate_environment():
        print("\n❌ Environment validation failed. Please check the missing files.")
        sys.exit(1)
    
    print("\n🎉 Development environment setup completed!")
    print("\nNext steps:")
    print("1. Install Qt development environment")
    print("2. Install OpenCV")
    print("3. Run build script:")
    if platform.system() == "Windows":
        print("   build.bat")
    else:
        print("   ./build.sh")
    print("\nHappy coding! 🎯")

if __name__ == "__main__":
    main() 