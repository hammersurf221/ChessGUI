#!/usr/bin/env python3
"""
Basic Usage Example for Chess Humanizer - SABLE Release

This example demonstrates how to use the ChessMovePredictor class
to predict moves from FEN positions.
"""

import sys
import os
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from chess_humanizer import ChessMovePredictor


def main():
    """Basic usage example."""
    print("🎯 Chess Humanizer - Basic Usage Example")
    print("=" * 50)
    
    # Initialize the predictor with a trained model
    # Note: You need to have a trained model in the models/ directory
    model_path = "models/blitz_model"  # Adjust path as needed
    
    try:
        predictor = ChessMovePredictor(model_path)
        print(f"✅ Model loaded successfully!")
    except Exception as e:
        print(f"❌ Error loading model: {e}")
        print("💡 Make sure you have a trained model in the models/ directory")
        return
    
    # Example FEN positions
    test_positions = [
        {
            "name": "Starting Position",
            "fen": "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
        },
        {
            "name": "Sicilian Defense",
            "fen": "rnbqkbnr/pp1ppppp/2p5/8/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2"
        },
        {
            "name": "Ruy Lopez",
            "fen": "r1bqkbnr/pppp1ppp/2n5/1B2p3/4P3/5N2/PPPP1PPP/RNBQK2R b KQkq - 3 3"
        }
    ]
    
    # Test each position
    for i, position in enumerate(test_positions, 1):
        print(f"\n📊 Position {i}: {position['name']}")
        print(f"FEN: {position['fen']}")
        
        # Get move predictions
        predictions = predictor.predict_moves(
            position['fen'],
            num_moves=5,
            temperature=1.0
        )
        
        # Display predictions
        print("Top 5 predicted moves:")
        for j, pred in enumerate(predictions, 1):
            print(f"  {j}. {pred['move']:8s} (UCI: {pred['uci']:6s}) - {pred['probability']:.3f} ({pred['probability']*100:.1f}%)")
        
        print("-" * 50)
    
    # Example with custom context
    print("\n🎭 Example with Custom Context:")
    custom_context = {
        'eval_score': 150,  # Slight advantage for white
        'eval_delta': 50,   # Position improved by 50 centipawns
        'blunder_potential': 0.3,  # High blunder potential
        'complexity': 35,   # Complex position
        'material_balance': 0,  # Equal material
        'move_number': 15,  # Middlegame
        'game_phase': 'middlegame',
        'time_control': 'blitz',
        'white_elo': 1800,
        'black_elo': 1750,
        'per_move_time': 15.0,
        'center_control': 2,
        'king_safety': 0.6,
        'pawn_structure': 8,
        'is_capture': True,
        'is_check': False,
        'is_checkmate': False,
        'is_stalemate': False,
        'move_quality': 'tactical'
    }
    
    # Test position with custom context
    test_fen = "r1bqkbnr/pppp1ppp/2n5/1B2p3/4P3/5N2/PPPP1PPP/RNBQK2R b KQkq - 3 3"
    print(f"FEN: {test_fen}")
    print("Context: Blitz game, tactical position, high blunder potential")
    
    predictions = predictor.predict_with_custom_context(
        test_fen,
        custom_context,
        num_moves=3,
        temperature=0.8  # More conservative due to blunder potential
    )
    
    print("Top 3 predicted moves (with custom context):")
    for j, pred in enumerate(predictions, 1):
        print(f"  {j}. {pred['move']:8s} (UCI: {pred['uci']:6s}) - {pred['probability']:.3f} ({pred['probability']*100:.1f}%)")
    
    print("\n✅ Basic usage example completed!")


if __name__ == "__main__":
    main() 