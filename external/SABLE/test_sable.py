#!/usr/bin/env python3
"""
Test script for Chess Humanizer SABLE Release

This script tests the basic functionality of the SABLE release
to ensure everything is working correctly.
"""

import sys
import os
import json
import tempfile
from pathlib import Path

# Add current directory to path
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

def test_imports():
    """Test that all required modules can be imported."""
    print("🔍 Testing imports...")
    
    try:
        import torch
        print(f"✅ PyTorch {torch.__version__}")
    except ImportError as e:
        print(f"❌ PyTorch import failed: {e}")
        return False
    
    try:
        import chess
        print(f"✅ python-chess {chess.__version__}")
    except ImportError as e:
        print(f"❌ python-chess import failed: {e}")
        return False
    
    try:
        import numpy as np
        print(f"✅ NumPy {np.__version__}")
    except ImportError as e:
        print(f"❌ NumPy import failed: {e}")
        return False
    
    try:
        import pandas as pd
        print(f"✅ Pandas {pd.__version__}")
    except ImportError as e:
        print(f"❌ Pandas import failed: {e}")
        return False
    
    try:
        from chess_humanizer import ChessMovePredictor, ChessTransformer, ChessPositionEncoder
        print("✅ chess_humanizer module imported successfully")
    except ImportError as e:
        print(f"❌ chess_humanizer import failed: {e}")
        return False
    
    return True


def test_model_creation():
    """Test that the model can be created (without loading weights)."""
    print("\n🔧 Testing model creation...")
    
    try:
        from chess_humanizer import ChessTransformer
        
        # Create a small test model
        model = ChessTransformer(num_moves=100)  # Small vocabulary for testing
        print("✅ ChessTransformer created successfully")
        
        # Test forward pass with dummy data
        import torch
        dummy_position = torch.randn(1, 8, 8, 16)  # Batch size 1, 8x8 board, 16 channels
        dummy_context = torch.randn(1, 16)  # Batch size 1, 16 context features
        
        with torch.no_grad():
            output = model(dummy_position, dummy_context)
        
        print(f"✅ Forward pass successful, output shape: {output.shape}")
        return True
        
    except Exception as e:
        print(f"❌ Model creation failed: {e}")
        return False


def test_encoder():
    """Test the position encoder."""
    print("\n🔤 Testing position encoder...")
    
    try:
        from chess_humanizer import ChessPositionEncoder
        
        encoder = ChessPositionEncoder()
        
        # Test FEN encoding
        test_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
        position_tensor = encoder.encode_fen(test_fen)
        
        print(f"✅ FEN encoding successful, tensor shape: {position_tensor.shape}")
        
        # Test context encoding
        test_context = {
            'eval_score': 0,
            'eval_delta': 0,
            'blunder_potential': 0.0,
            'complexity': 20,
            'material_balance': 0.0,
            'move_number': 1,
            'game_phase': 'opening',
            'time_control': 'rapid',
            'white_elo': 1500,
            'black_elo': 1500,
            'per_move_time': 30.0,
            'center_control': 0,
            'king_safety': 0.0,
            'pawn_structure': 0,
            'is_capture': False,
            'is_check': False,
            'is_checkmate': False,
            'is_stalemate': False,
            'move_quality': 'normal'
        }
        
        context_tensor = encoder.encode_context_features(test_context)
        print(f"✅ Context encoding successful, tensor shape: {context_tensor.shape}")
        
        return True
        
    except Exception as e:
        print(f"❌ Encoder test failed: {e}")
        return False


def test_predictor_initialization():
    """Test predictor initialization (without actual model files)."""
    print("\n🎯 Testing predictor initialization...")
    
    try:
        from chess_humanizer import ChessMovePredictor
        
        # This should fail gracefully if no model files exist
        try:
            predictor = ChessMovePredictor("nonexistent_model")
            print("❌ Should have failed with nonexistent model")
            return False
        except Exception as e:
            print(f"✅ Correctly failed with nonexistent model: {type(e).__name__}")
        
        return True
        
    except Exception as e:
        print(f"❌ Predictor initialization test failed: {e}")
        return False


def test_command_line_tools():
    """Test that command line tools can be imported."""
    print("\n🖥️  Testing command line tools...")
    
    try:
        import predict_moves
        print("✅ predict_moves.py imported successfully")
    except Exception as e:
        print(f"❌ predict_moves.py import failed: {e}")
        return False
    
    try:
        import uci_engine
        print("✅ uci_engine.py imported successfully")
    except Exception as e:
        print(f"❌ uci_engine.py import failed: {e}")
        return False
    
    return True


def test_examples():
    """Test that example files can be imported."""
    print("\n📝 Testing example files...")
    
    try:
        sys.path.append(os.path.join(os.path.dirname(__file__), 'examples'))
        import basic_usage
        print("✅ basic_usage.py imported successfully")
    except Exception as e:
        print(f"❌ basic_usage.py import failed: {e}")
        return False
    
    try:
        import advanced_usage
        print("✅ advanced_usage.py imported successfully")
    except Exception as e:
        print(f"❌ advanced_usage.py import failed: {e}")
        return False
    
    # Test context template
    try:
        context_template_path = os.path.join(os.path.dirname(__file__), 'examples', 'context_template.json')
        with open(context_template_path, 'r') as f:
            context = json.load(f)
        print("✅ context_template.json loaded successfully")
    except Exception as e:
        print(f"❌ context_template.json loading failed: {e}")
        return False
    
    return True


def test_file_structure():
    """Test that all required files exist."""
    print("\n📁 Testing file structure...")
    
    required_files = [
        'chess_humanizer.py',
        'predict_moves.py',
        'uci_engine.py',
        'requirements.txt',
        'README.md',
        'setup.py',
        'examples/basic_usage.py',
        'examples/advanced_usage.py',
        'examples/context_template.json'
    ]
    
    missing_files = []
    for file_path in required_files:
        full_path = os.path.join(os.path.dirname(__file__), file_path)
        if not os.path.exists(full_path):
            missing_files.append(file_path)
        else:
            print(f"✅ {file_path}")
    
    if missing_files:
        print(f"❌ Missing files: {missing_files}")
        return False
    
    return True


def main():
    """Run all tests."""
    print("🧪 Chess Humanizer SABLE Release - Test Suite")
    print("=" * 60)
    
    tests = [
        ("File Structure", test_file_structure),
        ("Imports", test_imports),
        ("Model Creation", test_model_creation),
        ("Encoder", test_encoder),
        ("Predictor Initialization", test_predictor_initialization),
        ("Command Line Tools", test_command_line_tools),
        ("Examples", test_examples),
    ]
    
    passed = 0
    total = len(tests)
    
    for test_name, test_func in tests:
        print(f"\n{'='*20} {test_name} {'='*20}")
        try:
            if test_func():
                passed += 1
                print(f"✅ {test_name} PASSED")
            else:
                print(f"❌ {test_name} FAILED")
        except Exception as e:
            print(f"❌ {test_name} FAILED with exception: {e}")
    
    print(f"\n{'='*60}")
    print(f"📊 Test Results: {passed}/{total} tests passed")
    
    if passed == total:
        print("🎉 All tests passed! SABLE release is ready to use.")
        print("\n💡 To use the system:")
        print("1. Add trained model files to the models/ directory")
        print("2. Run: python examples/basic_usage.py")
        print("3. Or use: python predict_moves.py --help")
    else:
        print("⚠️  Some tests failed. Please check the errors above.")
        return 1
    
    return 0


if __name__ == "__main__":
    sys.exit(main()) 