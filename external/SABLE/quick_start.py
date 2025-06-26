#!/usr/bin/env python3
"""
Quick Start Script for Chess Humanizer SABLE Release

This script provides a simple way to test the system and get started quickly.
"""

import sys
import os
import json
from pathlib import Path

# Add current directory to path
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

def check_dependencies():
    """Check if all dependencies are installed."""
    print("🔍 Checking dependencies...")
    
    required_packages = [
        ('torch', 'PyTorch'),
        ('chess', 'python-chess'),
        ('numpy', 'NumPy'),
        ('pandas', 'Pandas'),
        ('sklearn', 'scikit-learn'),
        ('tqdm', 'tqdm'),
        ('rich', 'Rich (optional)'),
    ]
    
    missing = []
    for package, name in required_packages:
        try:
            __import__(package)
            print(f"✅ {name}")
        except ImportError:
            if package != 'rich':  # Rich is optional
                missing.append(name)
            else:
                print(f"⚠️  {name} (optional)")
    
    if missing:
        print(f"\n❌ Missing required packages: {', '.join(missing)}")
        print("💡 Install with: pip install -r requirements.txt")
        return False
    
    return True


def check_models():
    """Check if any trained models are available."""
    print("\n📁 Checking for trained models...")
    
    models_dir = Path("models")
    if not models_dir.exists():
        print("❌ models/ directory not found")
        print("💡 Create models/ directory and add trained model files")
        return False
    
    model_files = list(models_dir.glob("*_model.pth"))
    vocab_files = list(models_dir.glob("*_vocab.pkl"))
    
    if not model_files:
        print("❌ No trained model files found (*_model.pth)")
        print("💡 Add trained model files to models/ directory")
        return False
    
    if not vocab_files:
        print("❌ No vocabulary files found (*_vocab.pkl)")
        print("💡 Add vocabulary files to models/ directory")
        return False
    
    print(f"✅ Found {len(model_files)} model files and {len(vocab_files)} vocabulary files")
    
    # Show available models
    for model_file in model_files:
        model_name = model_file.stem.replace('_model', '')
        vocab_file = model_file.parent / f"{model_name}_vocab.pkl"
        if vocab_file.exists():
            print(f"   📦 {model_name}")
    
    return True


def demo_prediction():
    """Run a demo prediction if models are available."""
    print("\n🎯 Running demo prediction...")
    
    try:
        from chess_humanizer import ChessMovePredictor
        
        # Find first available model
        models_dir = Path("models")
        model_files = list(models_dir.glob("*_model.pth"))
        
        if not model_files:
            print("❌ No models available for demo")
            return False
        
        model_path = model_files[0].parent / model_files[0].stem.replace('_model', '')
        print(f"📦 Using model: {model_path.name}")
        
        # Initialize predictor
        predictor = ChessMovePredictor(str(model_path))
        
        # Test position
        test_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
        print(f"📊 Testing position: {test_fen}")
        
        # Get predictions
        predictions = predictor.predict_moves(test_fen, num_moves=3, temperature=1.0)
        
        print("\n🎯 Top 3 predicted moves:")
        for i, pred in enumerate(predictions, 1):
            print(f"  {i}. {pred['move']:8s} (UCI: {pred['uci']:6s}) - {pred['probability']:.3f} ({pred['probability']*100:.1f}%)")
        
        return True
        
    except Exception as e:
        print(f"❌ Demo failed: {e}")
        return False


def show_usage_examples():
    """Show usage examples."""
    print("\n📚 Usage Examples:")
    print("=" * 50)
    
    print("\n1. Basic Python usage:")
    print("""
from chess_humanizer import ChessMovePredictor

predictor = ChessMovePredictor("models/blitz_model")
predictions = predictor.predict_moves(fen, num_moves=5)
for pred in predictions:
    print(f"{pred['move']} - {pred['probability']:.3f}")
""")
    
    print("\n2. Command line usage:")
    print("""
python predict_moves.py --model_path models/blitz_model --fen "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" --num_moves 5
""")
    
    print("\n3. UCI engine usage:")
    print("""
python uci_engine.py models/blitz_model
""")
    
    print("\n4. Run examples:")
    print("""
python examples/basic_usage.py
python examples/advanced_usage.py
""")


def main():
    """Main quick start function."""
    print("🚀 Chess Humanizer SABLE Release - Quick Start")
    print("=" * 60)
    
    # Check dependencies
    if not check_dependencies():
        print("\n❌ Please install missing dependencies first.")
        return 1
    
    # Check models
    models_available = check_models()
    
    if models_available:
        # Run demo
        demo_success = demo_prediction()
        if demo_success:
            print("\n🎉 Demo completed successfully!")
        else:
            print("\n⚠️  Demo failed, but system is ready to use.")
    else:
        print("\n⚠️  No models available, but system is ready to use.")
    
    # Show usage examples
    show_usage_examples()
    
    print("\n" + "=" * 60)
    print("✅ Quick start completed!")
    print("\n💡 Next steps:")
    print("1. Add trained model files to models/ directory")
    print("2. Run: python examples/basic_usage.py")
    print("3. Try: python predict_moves.py --help")
    print("4. Read README.md for detailed documentation")
    
    return 0


if __name__ == "__main__":
    sys.exit(main()) 