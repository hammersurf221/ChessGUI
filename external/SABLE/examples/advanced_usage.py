#!/usr/bin/env python3
"""
Advanced Usage Example for Chess Humanizer - SABLE Release

This example demonstrates advanced features including:
- Position analysis
- Custom context creation
- Temperature control
- Batch predictions
"""

import sys
import os
import json
import time
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from chess_humanizer import ChessMovePredictor


def create_context_for_scenario(scenario: str, move_number: int = 1) -> dict:
    """Create context for different game scenarios."""
    base_context = {
        'eval_score': 0,
        'eval_delta': 0,
        'blunder_potential': 0.0,
        'complexity': 20,
        'material_balance': 0.0,
        'move_number': move_number,
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
    
    if scenario == "blitz_aggressive":
        base_context.update({
            'time_control': 'blitz',
            'per_move_time': 10.0,
            'white_elo': 2000,
            'black_elo': 2000,
            'blunder_potential': 0.4,
            'complexity': 30,
            'move_quality': 'aggressive'
        })
    elif scenario == "classical_cautious":
        base_context.update({
            'time_control': 'classical',
            'per_move_time': 120.0,
            'white_elo': 1800,
            'black_elo': 1800,
            'blunder_potential': 0.1,
            'complexity': 15,
            'move_quality': 'cautious'
        })
    elif scenario == "tactical_position":
        base_context.update({
            'eval_score': 200,
            'eval_delta': 100,
            'blunder_potential': 0.6,
            'complexity': 40,
            'is_capture': True,
            'move_quality': 'tactical'
        })
    elif scenario == "endgame":
        base_context.update({
            'game_phase': 'endgame',
            'move_number': 40,
            'complexity': 25,
            'blunder_potential': 0.2,
            'move_quality': 'endgame'
        })
    
    return base_context


def analyze_position_detailed(predictor, fen: str):
    """Perform detailed position analysis."""
    print(f"\n🔍 Detailed Position Analysis")
    print("=" * 60)
    
    analysis = predictor.analyze_position(fen)
    
    print(f"Position: {analysis['fen']}")
    print(f"Turn: {analysis['turn']}")
    print(f"Legal moves: {analysis['legal_moves_count']}")
    print(f"Check: {analysis['is_check']}")
    print(f"Checkmate: {analysis['is_checkmate']}")
    print(f"Stalemate: {analysis['is_stalemate']}")
    
    print(f"\nCastling Rights:")
    for color, rights in analysis['castling_rights'].items():
        print(f"  {color}: {rights}")
    
    if analysis['en_passant_square']:
        print(f"En passant square: {analysis['en_passant_square']}")
    
    print(f"\nContext Features:")
    context = analysis['context_features']
    for key, value in context.items():
        print(f"  {key}: {value}")
    
    print(f"\nFirst 10 legal moves:")
    for i, move in enumerate(analysis['legal_moves'][:10], 1):
        print(f"  {i:2d}. {move}")


def compare_temperatures(predictor, fen: str, context: dict):
    """Compare predictions at different temperatures."""
    print(f"\n🌡️  Temperature Comparison")
    print("=" * 60)
    
    temperatures = [0.5, 1.0, 1.5, 2.0]
    
    for temp in temperatures:
        print(f"\nTemperature: {temp}")
        predictions = predictor.predict_with_custom_context(
            fen, context, num_moves=3, temperature=temp
        )
        
        for i, pred in enumerate(predictions, 1):
            print(f"  {i}. {pred['move']:8s} - {pred['probability']:.3f} ({pred['probability']*100:.1f}%)")


def batch_predictions(predictor, positions: list, context: dict):
    """Perform batch predictions on multiple positions."""
    print(f"\n📦 Batch Predictions")
    print("=" * 60)
    
    start_time = time.time()
    
    for i, position in enumerate(positions, 1):
        print(f"\nPosition {i}: {position['name']}")
        print(f"FEN: {position['fen']}")
        
        predictions = predictor.predict_with_custom_context(
            position['fen'], context, num_moves=2, temperature=1.0
        )
        
        for j, pred in enumerate(predictions, 1):
            print(f"  {j}. {pred['move']:8s} - {pred['probability']:.3f}")
    
    elapsed_time = time.time() - start_time
    print(f"\n⏱️  Batch processing time: {elapsed_time:.2f} seconds")
    print(f"⏱️  Average time per position: {elapsed_time/len(positions):.2f} seconds")


def main():
    """Advanced usage example."""
    print("🎯 Chess Humanizer - Advanced Usage Example")
    print("=" * 60)
    
    # Initialize the predictor
    model_path = "models/blitz_model"  # Adjust path as needed
    
    try:
        predictor = ChessMovePredictor(model_path)
        print(f"✅ Model loaded successfully!")
    except Exception as e:
        print(f"❌ Error loading model: {e}")
        print("💡 Make sure you have a trained model in the models/ directory")
        return
    
    # Test position
    test_fen = "r1bqkbnr/pppp1ppp/2n5/1B2p3/4P3/5N2/PPPP1PPP/RNBQK2R b KQkq - 3 3"
    
    # 1. Detailed position analysis
    analyze_position_detailed(predictor, test_fen)
    
    # 2. Different scenarios
    scenarios = ["blitz_aggressive", "classical_cautious", "tactical_position", "endgame"]
    
    for scenario in scenarios:
        print(f"\n🎭 Scenario: {scenario}")
        print("-" * 40)
        
        context = create_context_for_scenario(scenario)
        predictions = predictor.predict_with_custom_context(
            test_fen, context, num_moves=3, temperature=1.0
        )
        
        for i, pred in enumerate(predictions, 1):
            print(f"  {i}. {pred['move']:8s} - {pred['probability']:.3f} ({pred['probability']*100:.1f}%)")
    
    # 3. Temperature comparison
    context = create_context_for_scenario("blitz_aggressive")
    compare_temperatures(predictor, test_fen, context)
    
    # 4. Batch predictions
    test_positions = [
        {"name": "Starting Position", "fen": "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"},
        {"name": "Sicilian Defense", "fen": "rnbqkbnr/pp1ppppp/2p5/8/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2"},
        {"name": "Ruy Lopez", "fen": "r1bqkbnr/pppp1ppp/2n5/1B2p3/4P3/5N2/PPPP1PPP/RNBQK2R b KQkq - 3 3"},
        {"name": "French Defense", "fen": "rnbqkbnr/pppp1ppp/4p3/8/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2"}
    ]
    
    batch_context = create_context_for_scenario("rapid")
    batch_predictions(predictor, test_positions, batch_context)
    
    print("\n✅ Advanced usage example completed!")


if __name__ == "__main__":
    main() 