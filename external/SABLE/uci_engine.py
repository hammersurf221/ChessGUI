#!/usr/bin/env python3
"""
UCI Protocol Wrapper for Chess Move Prediction Model - SABLE Release

This module provides a UCI-compatible interface for the chess move prediction model,
allowing it to be used with chess applications that support UCI engines.
"""

import sys
import time
import threading
from typing import Dict, List, Optional
from chess_humanizer import ChessMovePredictor


class UCIChessEngine:
    """UCI-compatible chess engine wrapper for the move prediction model."""
    
    def __init__(self, model_path: str, vocab_path: Optional[str] = None):
        """Initialize the UCI engine with a trained model."""
        self.model_path = model_path
        self.vocab_path = vocab_path
        self.predictor = None
        self.is_ready = False
        self.is_thinking = False
        self.current_position = None
        self.search_time = 1000  # Default search time in ms
        self.max_depth = 1  # Always depth 1 for prediction model
        self.multipv = 1
        
        # Initialize the predictor
        self._load_model()
        
        # UCI engine info
        self.engine_name = "ChessHumanizer"
        self.engine_author = "Move Prediction Model"
        self.engine_version = "1.0"
    
    def _load_model(self):
        """Load the prediction model."""
        try:
            self.predictor = ChessMovePredictor(self.model_path, self.vocab_path)
            self.is_ready = True
            print(f"info string Model loaded: {self.model_path}", file=sys.stderr)
        except Exception as e:
            print(f"info string Error loading model: {e}", file=sys.stderr)
            self.is_ready = False
    
    def _format_score(self, probability: float) -> int:
        """Convert probability to centipawn score for UCI compatibility."""
        # Convert probability (0-1) to centipawn score (-10000 to 10000)
        # This is a rough approximation since we don't have true evaluation
        if probability > 0.5:
            # Positive score for high probability moves
            return int((probability - 0.5) * 20000)
        else:
            # Negative score for low probability moves
            return int((probability - 0.5) * 20000)
    
    def _search_position(self, fen: str, search_time_ms: int = 1000) -> List[Dict]:
        """Search the position and return move predictions."""
        if not self.is_ready or not self.predictor:
            return []
        
        try:
            # Get predictions with temperature based on search time
            # Longer search time = more conservative (lower temperature)
            temperature = max(0.5, min(1.5, 1000 / search_time_ms))
            
            # Get multiple variations if multipv > 1
            num_moves = max(1, min(10, self.multipv))
            
            predictions = self.predictor.predict_moves(
                fen, 
                num_moves=num_moves, 
                temperature=temperature
            )
            
            return predictions
            
        except Exception as e:
            print(f"info string Search error: {e}", file=sys.stderr)
            return []
    
    def _send_best_move(self, predictions: List[Dict]):
        """Send best move in UCI format."""
        if predictions:
            best_move = predictions[0]['uci']
            print(f"bestmove {best_move}")
        else:
            print("bestmove 0000")  # No move available
    
    def _send_info(self, predictions: List[Dict], depth: int = 1, time_ms: int = 0):
        """Send search info in UCI format."""
        for i, pred in enumerate(predictions):
            if i >= self.multipv:
                break
                
            score = self._format_score(pred['probability'])
            move = pred['uci']
            
            # Format UCI info string
            info_parts = [
                f"info depth {depth}",
                f"multipv {i + 1}",
                f"score cp {score}",
                f"time {time_ms}",
                f"pv {move}"
            ]
            
            print(" ".join(info_parts))
    
    def handle_uci_command(self, command: str):
        """Handle UCI commands."""
        parts = command.strip().split()
        if not parts:
            return
        
        cmd = parts[0].lower()
        
        if cmd == "uci":
            self._handle_uci()
        elif cmd == "isready":
            self._handle_isready()
        elif cmd == "setoption":
            self._handle_setoption(parts[1:])
        elif cmd == "ucinewgame":
            self._handle_ucinewgame()
        elif cmd == "position":
            self._handle_position(parts[1:])
        elif cmd == "go":
            self._handle_go(parts[1:])
        elif cmd == "stop":
            self._handle_stop()
        elif cmd == "quit":
            self._handle_quit()
        else:
            print(f"info string Unknown command: {command}", file=sys.stderr)
    
    def _handle_uci(self):
        """Handle 'uci' command."""
        print("id name ChessHumanizer")
        print("id author Move Prediction Model")
        print("option name ModelPath type string default models/blitz_model")
        print("option name SearchTime type spin default 1000 min 100 max 10000")
        print("option name MultiPV type spin default 1 min 1 max 10")
        print("option name Temperature type spin default 100 min 50 max 150")
        print("uciok")
    
    def _handle_isready(self):
        """Handle 'isready' command."""
        if self.is_ready:
            print("readyok")
        else:
            print("info string Engine not ready", file=sys.stderr)
            print("readyok")
    
    def _handle_setoption(self, args: List[str]):
        """Handle 'setoption' command."""
        if len(args) >= 2 and args[0] == "name":
            option_name = args[1]
            if len(args) >= 4 and args[2] == "value":
                option_value = args[3]
                
                if option_name == "SearchTime":
                    try:
                        self.search_time = int(option_value)
                    except ValueError:
                        pass
                elif option_name == "MultiPV":
                    try:
                        self.multipv = max(1, min(10, int(option_value)))
                    except ValueError:
                        pass
                elif option_name == "Temperature":
                    try:
                        # Temperature is stored as percentage (50-150)
                        temp_percent = max(50, min(150, int(option_value)))
                        self.temperature = temp_percent / 100.0
                    except ValueError:
                        pass
    
    def _handle_ucinewgame(self):
        """Handle 'ucinewgame' command."""
        # Reset any game state if needed
        pass
    
    def _handle_position(self, args: List[str]):
        """Handle 'position' command."""
        if not args:
            return
        
        if args[0] == "startpos":
            self.current_position = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
            # Apply moves if provided
            if len(args) > 1 and args[1] == "moves":
                self._apply_moves(args[2:])
        elif args[0] == "fen":
            # Parse FEN position
            fen_parts = args[1:7]  # Take first 6 parts of FEN
            self.current_position = " ".join(fen_parts)
            # Apply moves if provided
            if len(args) > 7 and args[7] == "moves":
                self._apply_moves(args[8:])
    
    def _apply_moves(self, moves: List[str]):
        """Apply moves to the current position."""
        if not self.current_position:
            return
        
        try:
            import chess
            board = chess.Board(self.current_position)
            
            for move_uci in moves:
                move = chess.Move.from_uci(move_uci)
                if move in board.legal_moves:
                    board.push(move)
                else:
                    print(f"info string Illegal move: {move_uci}", file=sys.stderr)
                    break
            
            self.current_position = board.fen()
            
        except Exception as e:
            print(f"info string Error applying moves: {e}", file=sys.stderr)
    
    def _handle_go(self, args: List[str]):
        """Handle 'go' command."""
        if not self.current_position:
            print("info string No position set", file=sys.stderr)
            print("bestmove 0000")
            return
        
        # Parse go parameters
        search_time = self.search_time
        depth = 1  # Always depth 1 for prediction model
        
        i = 0
        while i < len(args):
            if args[i] == "movetime" and i + 1 < len(args):
                try:
                    search_time = int(args[i + 1])
                except ValueError:
                    pass
                i += 2
            elif args[i] == "depth" and i + 1 < len(args):
                try:
                    depth = int(args[i + 1])
                except ValueError:
                    pass
                i += 2
            else:
                i += 1
        
        # Start search in a separate thread
        self.is_thinking = True
        search_thread = threading.Thread(
            target=self._search_thread,
            args=(self.current_position, search_time, depth)
        )
        search_thread.daemon = True
        search_thread.start()
    
    def _search_thread(self, fen: str, search_time_ms: int, depth: int):
        """Search thread to simulate thinking time."""
        start_time = time.time()
        
        # Perform the search
        predictions = self._search_position(fen, search_time_ms)
        
        # Calculate elapsed time
        elapsed_time = int((time.time() - start_time) * 1000)
        
        # Send info during search (simulated)
        if self.is_thinking:
            self._send_info(predictions, depth, elapsed_time)
        
        # Send best move
        if self.is_thinking:
            self._send_best_move(predictions)
        
        self.is_thinking = False
    
    def _handle_stop(self):
        """Handle 'stop' command."""
        self.is_thinking = False
        # Send best move immediately if we have one
        if self.current_position:
            predictions = self._search_position(self.current_position, 100)
            self._send_best_move(predictions)
    
    def _handle_quit(self):
        """Handle 'quit' command."""
        self.is_thinking = False
        sys.exit(0)


def main():
    """Main function for UCI engine."""
    if len(sys.argv) < 2:
        print("Usage: python uci_engine.py <model_path> [vocab_path]", file=sys.stderr)
        sys.exit(1)
    
    model_path = sys.argv[1]
    vocab_path = sys.argv[2] if len(sys.argv) > 2 else None
    
    # Create UCI engine
    engine = UCIChessEngine(model_path, vocab_path)
    
    # Main command loop
    try:
        for line in sys.stdin:
            engine.handle_uci_command(line.strip())
    except KeyboardInterrupt:
        print("info string Engine stopped by user", file=sys.stderr)
    except Exception as e:
        print(f"info string Engine error: {e}", file=sys.stderr)


if __name__ == "__main__":
    main() 