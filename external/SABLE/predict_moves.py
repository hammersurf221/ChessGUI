#!/usr/bin/env python3
"""
Chess Move Prediction Interface for SABLE Release

This script provides a command-line interface to feed FEN positions to a trained chess move prediction model
and get multiple move predictions with proper context handling.

Usage:
    python predict_moves.py --model_path models/blitz_model --fen "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" --num_moves 5
"""

import argparse
import torch
import torch.nn.functional as F
import chess
import json
import pickle
from pathlib import Path
from typing import List, Dict, Tuple, Optional
import sys
import os

# Import the chess humanizer module
from chess_humanizer import ChessMovePredictor


def main():
    """Main function for command-line interface."""
    parser = argparse.ArgumentParser(description="Predict chess moves from FEN positions")
    parser.add_argument("--model_path", required=True, help="Path to trained model (without extension)")
    parser.add_argument("--fen", required=True, help="FEN string of the position")
    parser.add_argument("--num_moves", type=int, default=5, help="Number of moves to predict")
    parser.add_argument("--temperature", type=float, default=1.0, help="Sampling temperature")
    parser.add_argument("--context_file", help="JSON file with custom context")
    parser.add_argument("--analyze", action="store_true", help="Analyze position and show context")
    parser.add_argument("--output_file", help="Save predictions to JSON file")
    
    args = parser.parse_args()
    
    try:
        # Initialize predictor
        predictor = ChessMovePredictor(args.model_path)
        
        # Analyze position if requested
        if args.analyze:
            print("\n" + "="*60)
            print("POSITION ANALYSIS")
            print("="*60)
            analysis = predictor.analyze_position(args.fen)
            print(json.dumps(analysis, indent=2))
            print("="*60)
        
        # Load custom context if provided
        custom_context = None
        if args.context_file:
            with open(args.context_file, 'r') as f:
                custom_context = json.load(f)
            print(f"📄 Loaded custom context from {args.context_file}")
        
        # Predict moves
        print(f"\n🎯 Predicting {args.num_moves} moves with temperature {args.temperature}")
        if custom_context:
            predictions = predictor.predict_with_custom_context(
                args.fen, custom_context, args.num_moves, args.temperature
            )
        else:
            predictions = predictor.predict_moves(
                args.fen, args.num_moves, args.temperature
            )
        
        # Display results
        print("\n" + "="*60)
        print("MOVE PREDICTIONS")
        print("="*60)
        
        for i, pred in enumerate(predictions):
            print(f"{pred['rank']:2d}. {pred['move']:8s} (UCI: {pred['uci']:6s}) - {pred['probability']:.3f} ({pred['probability']*100:.1f}%)")
        
        print("="*60)
        
        # Save to file if requested
        if args.output_file:
            output_data = {
                "fen": args.fen,
                "model_path": args.model_path,
                "temperature": args.temperature,
                "predictions": predictions,
                "context_used": custom_context if custom_context else "auto-generated"
            }
            
            with open(args.output_file, 'w') as f:
                json.dump(output_data, f, indent=2)
            print(f"💾 Predictions saved to {args.output_file}")
    
    except Exception as e:
        print(f"❌ Error: {e}")
        sys.exit(1)


if __name__ == "__main__":
    main() 