#!/usr/bin/env python3
"""
Chess Humanizer - Human-like Chess Move Prediction

A neural network-based chess move prediction system that plays like a human player
rather than a perfect engine. Uses context-aware predictions to generate moves that
reflect human decision-making patterns.

This is the standalone release version that can be used outside of the main project.
"""

import torch
import torch.nn as nn
import torch.nn.functional as F
import numpy as np
import pandas as pd
import chess
import json
import pickle
import os
import sys
from typing import List, Dict, Tuple, Optional
from pathlib import Path
import time

# Suppress TensorBoard warnings
os.environ['TF_CPP_MIN_LOG_LEVEL'] = '2'

# Try to import Rich for enhanced output
try:
    from rich.console import Console
    from rich.progress import Progress
    RICH_AVAILABLE = True
except ImportError:
    RICH_AVAILABLE = False

# Check for torch.compile
TORCH_COMPILE_AVAILABLE = hasattr(torch, 'compile')


class LabelSmoothingCrossEntropy(nn.Module):
    """Label smoothing cross entropy loss."""
    def __init__(self, epsilon: float = 0.05, reduction: str = 'mean'):
        super().__init__()
        self.epsilon = epsilon
        self.reduction = reduction
    
    def forward(self, logits, targets):
        num_classes = logits.size(-1)
        log_probs = F.log_softmax(logits, dim=-1)
        targets_one_hot = F.one_hot(targets, num_classes).float()
        
        # Apply label smoothing
        targets_one_hot = targets_one_hot * (1 - self.epsilon) + self.epsilon / num_classes
        
        loss = -(targets_one_hot * log_probs).sum(dim=-1)
        
        if self.reduction == 'mean':
            return loss.mean()
        elif self.reduction == 'sum':
            return loss.sum()
        else:
            return loss


class ChessTransformer(nn.Module):
    """Transformer-based model for chess move prediction."""
    
    def __init__(self, 
                 board_size: int = 8,
                 num_channels: int = 16,
                 context_dim: int = 16,
                 d_model: int = 256,
                 num_heads: int = 8,
                 num_layers: int = 6,
                 num_moves: int = 218,
                 dropout: float = 0.1):
        super().__init__()
        
        self.board_size = board_size
        self.num_channels = num_channels
        self.context_dim = context_dim
        self.d_model = d_model
        self.num_moves = num_moves
        
        # Board projection to token space
        self.board_projection = nn.Linear(board_size * board_size * num_channels, board_size * board_size * d_model)
        
        # Positional embeddings (rank and file)
        self.rank_embedding = nn.Parameter(torch.randn(board_size, d_model))
        self.file_embedding = nn.Parameter(torch.randn(board_size, d_model))
        
        # CLS token
        self.cls_token = nn.Parameter(torch.randn(1, 1, d_model))
        
        # Context projection to d_model
        self.context_projection = nn.Linear(context_dim, d_model)
        
        # Transformer encoder
        encoder_layer = nn.TransformerEncoderLayer(
            d_model=d_model,
            nhead=num_heads,
            dim_feedforward=4 * d_model,
            dropout=dropout,
            batch_first=True,
            norm_first=True
        )
        
        self.transformer = nn.TransformerEncoder(encoder_layer, num_layers=num_layers)
        
        # Output projection (MLP head)
        self.output_projection = nn.Sequential(
            nn.Linear(d_model, d_model // 2),
            nn.ReLU(),
            nn.Dropout(dropout),
            nn.Linear(d_model // 2, d_model // 4),
            nn.ReLU(),
            nn.Dropout(dropout),
            nn.Linear(d_model // 4, num_moves)
        )
    
    def forward(self, position: torch.Tensor, context: torch.Tensor) -> torch.Tensor:
        """Forward pass with optimized architecture."""
        batch_size = position.shape[0]
        
        # 1. Project board to token space: (B, 8, 8, 16) -> (B, 64, d_model)
        position_flat = position.view(batch_size, -1)  # (B, 8*8*16)
        board_tokens = self.board_projection(position_flat)  # (B, 64 * d_model)
        board_tokens = board_tokens.view(batch_size, self.board_size * self.board_size, self.d_model)  # (B, 64, d_model)
        
        # 2. Add positional embeddings
        # Create rank and file indices for each square
        ranks = torch.arange(self.board_size, device=position.device).repeat_interleave(self.board_size)  # (64,)
        files = torch.arange(self.board_size, device=position.device).repeat(self.board_size)  # (64,)
        
        # Get embeddings for each square
        rank_embeddings = self.rank_embedding[ranks]  # (64, d_model)
        file_embeddings = self.file_embedding[files]  # (64, d_model)
        
        # Add positional embeddings to board tokens
        board_tokens = board_tokens + rank_embeddings.unsqueeze(0) + file_embeddings.unsqueeze(0)  # (B, 64, d_model)
        
        # 3. Process context and create CLS token
        context_proj = self.context_projection(context)  # (B, d_model)
        cls_tokens = self.cls_token.expand(batch_size, -1, -1)  # (B, 1, d_model)
        cls_tokens = cls_tokens + context_proj.unsqueeze(1)  # Add context to CLS token
        
        # 4. Concatenate CLS token with board tokens
        tokens = torch.cat([cls_tokens, board_tokens], dim=1)  # (B, 65, d_model)
        
        # 5. Apply transformer
        transformed = self.transformer(tokens)
        
        # 6. Pool using CLS token (first token)
        cls_output = transformed[:, 0]  # (B, d_model)
        
        # 7. Output projection
        logits = self.output_projection(cls_output)  # (B, num_moves)
        
        return logits


class ChessPositionEncoder:
    """Encoder for chess positions and context features."""
    
    def __init__(self):
        # Game phase encoder
        self.phase_encoder = {
            'opening': 0,
            'middlegame': 1,
            'endgame': 2
        }
        
        # Time control encoder
        self.time_control_encoder = {
            'blitz': 0,
            'rapid': 1,
            'classical': 2
        }
    
    def encode_fen(self, fen: str) -> torch.Tensor:
        """Encode FEN string to tensor representation."""
        board = chess.Board(fen)
        
        # Create 8x8x12 tensor (6 piece types x 2 colors)
        tensor = torch.zeros(8, 8, 12)
        
        piece_to_channel = {
            (chess.PAWN, chess.WHITE): 0,
            (chess.KNIGHT, chess.WHITE): 1,
            (chess.BISHOP, chess.WHITE): 2,
            (chess.ROOK, chess.WHITE): 3,
            (chess.QUEEN, chess.WHITE): 4,
            (chess.KING, chess.WHITE): 5,
            (chess.PAWN, chess.BLACK): 6,
            (chess.KNIGHT, chess.BLACK): 7,
            (chess.BISHOP, chess.BLACK): 8,
            (chess.ROOK, chess.BLACK): 9,
            (chess.QUEEN, chess.BLACK): 10,
            (chess.KING, chess.BLACK): 11,
        }
        
        for square in chess.SQUARES:
            piece = board.piece_at(square)
            if piece:
                rank = square // 8
                file = square % 8
                channel = piece_to_channel[(piece.piece_type, piece.color)]
                tensor[rank, file, channel] = 1.0
        
        # Add additional features
        additional_features = torch.zeros(8, 8, 4)
        
        # Castling rights
        if board.has_kingside_castling_rights(chess.WHITE):
            additional_features[7, 6, 0] = 1.0  # g1
        if board.has_queenside_castling_rights(chess.WHITE):
            additional_features[7, 2, 0] = 1.0  # c1
        if board.has_kingside_castling_rights(chess.BLACK):
            additional_features[0, 6, 1] = 1.0  # g8
        if board.has_queenside_castling_rights(chess.BLACK):
            additional_features[0, 2, 1] = 1.0  # c8
        
        # En passant square
        if board.ep_square:
            rank = board.ep_square // 8
            file = board.ep_square % 8
            additional_features[rank, file, 2] = 1.0
        
        # Side to move
        if board.turn == chess.WHITE:
            additional_features[:, :, 3] = 1.0
        
        # Combine tensors
        combined = torch.cat([tensor, additional_features], dim=2)
        return combined
    
    def encode_context_features(self, context: Dict) -> torch.Tensor:
        """Encode context features to tensor."""
        features = []
        
        # Numerical features
        features.extend([
            context.get('eval_score', 0.0) / 1000.0,  # Normalize
            context.get('eval_delta', 0.0) / 1000.0,
            context.get('blunder_potential', 0.0) / 1000.0,
            context.get('complexity', 20) / 50.0,  # Normalize
            context.get('material_balance', 0.0) / 1000.0,
            context.get('move_number', 1) / 100.0,  # Normalize
        ])
        
        # Categorical features (one-hot encoded)
        # Game phase
        phase = context.get('game_phase', 'opening')
        phase_idx = self.phase_encoder.get(phase, 0)
        phase_onehot = [0, 0, 0]
        phase_onehot[phase_idx] = 1
        features.extend(phase_onehot)
        
        # Time control
        time_control = context.get('time_control', 'rapid')
        tc_idx = self.time_control_encoder.get(time_control, 1)
        tc_onehot = [0, 0, 0]
        tc_onehot[tc_idx] = 1
        features.extend(tc_onehot)
        
        # Boolean features
        features.extend([
            1.0 if context.get('is_check', False) else 0.0,
            1.0 if context.get('is_capture', False) else 0.0,
            1.0 if context.get('is_checkmate', False) else 0.0,
            1.0 if context.get('is_stalemate', False) else 0.0,
        ])
        
        return torch.tensor(features, dtype=torch.float32)
    
    def get_legal_moves_mask(self, fen: str, move_to_idx: Dict) -> torch.Tensor:
        """Get mask for legal moves."""
        board = chess.Board(fen)
        legal_moves = list(board.legal_moves)
        
        mask = torch.zeros(len(move_to_idx))
        for move in legal_moves:
            move_uci = move.uci()
            if move_uci in move_to_idx:
                mask[move_to_idx[move_uci]] = 1.0
        
        return mask


class ChessMovePredictor:
    """High-level interface for predicting chess moves from FEN positions."""
    
    def __init__(self, model_path: str, vocab_path: Optional[str] = None):
        """
        Initialize the move predictor with a trained model.
        
        Args:
            model_path: Path to the trained model (without extension)
            vocab_path: Optional path to vocabulary file (if None, will use model_path_vocab.pkl)
        """
        self.model_path = model_path
        self.vocab_path = vocab_path
        self.model = None
        self.encoder = None
        self.move_to_idx = {}
        self.idx_to_move = {}
        self.device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
        
        # Initialize Rich console if available
        if RICH_AVAILABLE:
            self.console = Console()
        else:
            self.console = None
        
        # Load the model
        self._load_model()
        
        print(f"✅ Model loaded successfully from {model_path}")
        print(f"🔧 Device: {self.device}")
        print(f"📊 Vocabulary size: {len(self.move_to_idx):,} moves")
    
    def _load_model(self):
        """Load the trained model and vocabulary."""
        try:
            # Load vocabulary
            vocab_path = self.vocab_path if self.vocab_path else f"{self.model_path}_vocab.pkl"
            if not os.path.exists(vocab_path):
                raise FileNotFoundError(f"Vocabulary file not found: {vocab_path}")
            
            with open(vocab_path, 'rb') as f:
                vocab_data = pickle.load(f)
            
            self.move_to_idx = vocab_data['move_to_idx']
            self.idx_to_move = vocab_data['idx_to_move']
            
            # Create encoder
            self.encoder = ChessPositionEncoder()
            
            # Create and load model
            self.model = ChessTransformer(num_moves=len(self.move_to_idx))
            
            # Look for model file with different naming patterns
            model_file = None
            possible_names = [
                f"{self.model_path}_model.pth",
                f"{self.model_path}.pth",
                os.path.join(self.model_path, "model.pth"),
                os.path.join(self.model_path, "sablev1-5m.pth")
            ]
            
            for name in possible_names:
                if os.path.exists(name):
                    model_file = name
                    break
            
            if model_file is None:
                raise FileNotFoundError(f"Model file not found. Tried: {possible_names}")
            
            # Load model state dict and patch _orig_mod. prefix if present
            state_dict = torch.load(model_file, map_location=self.device)
            if any(k.startswith('_orig_mod.') for k in state_dict.keys()):
                state_dict = {k.replace('_orig_mod.', ''): v for k, v in state_dict.items()}
            self.model.load_state_dict(state_dict)
            self.model.to(self.device)
            self.model.eval()
            
        except Exception as e:
            raise RuntimeError(f"Failed to load model from {self.model_path}: {e}")
    
    def _create_default_context(self, fen: str, move_number: int = 1) -> Dict:
        """Create default context features for a position."""
        board = chess.Board(fen)
        
        # Basic game state analysis
        is_check = board.is_check()
        is_checkmate = board.is_checkmate()
        is_stalemate = board.is_stalemate()
        
        # Material balance (simplified)
        material_balance = 0
        piece_values = {
            chess.PAWN: 100,
            chess.KNIGHT: 320,
            chess.BISHOP: 330,
            chess.ROOK: 500,
            chess.QUEEN: 900,
            chess.KING: 20000
        }
        
        for square in chess.SQUARES:
            piece = board.piece_at(square)
            if piece:
                value = piece_values[piece.piece_type]
                if piece.color == chess.WHITE:
                    material_balance += value
                else:
                    material_balance -= value
        
        # Game phase estimation
        total_pieces = len(board.piece_map())
        if total_pieces > 20:
            game_phase = "opening"
        elif total_pieces > 10:
            game_phase = "middlegame"
        else:
            game_phase = "endgame"
        
        # Default context
        context = {
            'eval_score': 0,  # Neutral evaluation
            'eval_delta': 0,  # No change
            'blunder_potential': 0.0,
            'complexity': 20,  # Medium complexity
            'material_balance': material_balance,
            'move_number': move_number,
            'game_phase': game_phase,
            'time_control': 'rapid',  # Default time control
            'white_elo': 1500,  # Default ELO
            'black_elo': 1500,
            'per_move_time': 30.0,  # Default time per move
            'center_control': 0,
            'king_safety': 0.0,
            'pawn_structure': 0,
            'is_capture': False,
            'is_check': is_check,
            'is_checkmate': is_checkmate,
            'is_stalemate': is_stalemate,
            'move_quality': 'normal'
        }
        
        return context
    
    def _analyze_position_for_context(self, fen: str) -> Dict:
        """Analyze a position to extract context features."""
        board = chess.Board(fen)
        context = self._create_default_context(fen)
        
        # Enhanced position analysis
        legal_moves = list(board.legal_moves)
        
        # Check for captures
        capture_moves = [move for move in legal_moves if board.is_capture(move)]
        context['is_capture'] = len(capture_moves) > 0
        
        # Center control (simplified)
        center_squares = [chess.E4, chess.E5, chess.D4, chess.D5]
        center_control = 0
        for square in center_squares:
            if board.piece_at(square):
                piece = board.piece_at(square)
                if piece.color == board.turn:
                    center_control += 1
        context['center_control'] = center_control
        
        # King safety (distance from center)
        white_king = board.king(chess.WHITE)
        black_king = board.king(chess.BLACK)
        
        if white_king is not None:
            rank, file = divmod(white_king, 8)
            center_distance = abs(rank - 3.5) + abs(file - 3.5)
            context['king_safety'] = max(0, 7 - center_distance) / 7.0
        
        # Pawn structure (simplified)
        pawn_structure = 0
        for square in chess.SQUARES:
            piece = board.piece_at(square)
            if piece and piece.piece_type == chess.PAWN:
                pawn_structure += 1
        context['pawn_structure'] = pawn_structure
        
        return context
    
    def predict_moves(self, fen: str, num_moves: int = 5, temperature: float = 1.0, 
                     context: Optional[Dict] = None, include_probabilities: bool = True) -> List[Dict]:
        """
        Predict multiple moves for a given FEN position.
        
        Args:
            fen: FEN string of the position
            num_moves: Number of moves to predict
            temperature: Sampling temperature (higher = more random)
            context: Optional context dictionary (if None, will be auto-generated)
            include_probabilities: Whether to include move probabilities
            
        Returns:
            List of dictionaries containing move predictions
        """
        if self.model is None:
            raise RuntimeError("Model not loaded")
        
        # Validate FEN
        try:
            board = chess.Board(fen)
        except ValueError as e:
            raise ValueError(f"Invalid FEN string: {e}")
        
        # Check if position is terminal
        if board.is_checkmate():
            return [{"move": "checkmate", "probability": 1.0, "uci": "checkmate", "san": "checkmate"}]
        if board.is_stalemate():
            return [{"move": "stalemate", "probability": 1.0, "uci": "stalemate", "san": "stalemate"}]
        if board.is_insufficient_material():
            return [{"move": "draw", "probability": 1.0, "uci": "draw", "san": "draw"}]
        
        # Generate context if not provided
        if context is None:
            context = self._analyze_position_for_context(fen)
        
        # Get legal moves
        legal_moves = list(board.legal_moves)
        if not legal_moves:
            return []
        
        # Prepare model inputs
        with torch.no_grad():
            # Encode position
            position_tensor = self.encoder.encode_fen(fen).unsqueeze(0).to(self.device)
            
            # Encode context
            context_tensor = self.encoder.encode_context_features(context).unsqueeze(0).to(self.device)
            
            # Get legal moves mask
            legal_mask = self.encoder.get_legal_moves_mask(fen, self.move_to_idx).to(self.device)
            
            # Forward pass
            logits = self.model(position_tensor, context_tensor)
            
            # Apply legal moves mask
            logits = logits + (legal_mask + 1e-45).log()
            
            # Apply temperature
            logits = logits / temperature
            
            # Get probabilities
            probs = F.softmax(logits, dim=1)
            
            # Get top-k moves
            top_probs, top_indices = torch.topk(probs, k=min(num_moves, len(legal_moves)), dim=1)
            
            # Convert to move predictions
            predictions = []
            for i in range(top_probs.shape[1]):
                move_idx = top_indices[0, i].item()
                probability = top_probs[0, i].item()
                uci_move = self.idx_to_move[move_idx]
                
                # Convert UCI to SAN if possible
                try:
                    san_move = board.san(chess.Move.from_uci(uci_move))
                except:
                    san_move = uci_move
                
                prediction = {
                    "move": san_move,
                    "uci": uci_move,
                    "probability": probability,
                    "rank": i + 1
                }
                
                if include_probabilities:
                    prediction["log_probability"] = torch.log(probs[0, move_idx]).item()
                
                predictions.append(prediction)
        
        return predictions
    
    def predict_with_custom_context(self, fen: str, context: Dict, num_moves: int = 5, 
                                   temperature: float = 1.0) -> List[Dict]:
        """Predict moves with custom context features."""
        return self.predict_moves(fen, num_moves, temperature, context)
    
    def analyze_position(self, fen: str) -> Dict:
        """Analyze a position and return detailed context information."""
        board = chess.Board(fen)
        context = self._analyze_position_for_context(fen)
        
        # Additional analysis
        legal_moves = list(board.legal_moves)
        
        analysis = {
            "fen": fen,
            "turn": "white" if board.turn == chess.WHITE else "black",
            "legal_moves_count": len(legal_moves),
            "is_check": board.is_check(),
            "is_checkmate": board.is_checkmate(),
            "is_stalemate": board.is_stalemate(),
            "castling_rights": {
                "white_kingside": board.has_kingside_castling_rights(chess.WHITE),
                "white_queenside": board.has_queenside_castling_rights(chess.WHITE),
                "black_kingside": board.has_kingside_castling_rights(chess.BLACK),
                "black_queenside": board.has_queenside_castling_rights(chess.BLACK)
            },
            "en_passant_square": str(board.ep_square) if board.ep_square else None,
            "context_features": context,
            "legal_moves": [move.uci() for move in legal_moves[:10]]  # First 10 moves
        }
        
        return analysis


# Version information
__version__ = "1.0.0"
__author__ = "Chess Humanizer Team"
__description__ = "Human-like chess move prediction system" 