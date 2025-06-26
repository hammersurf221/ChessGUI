# Chess Humanizer - SABLE Release

A standalone release of the Chess Humanizer system that provides human-like chess move prediction using neural networks. This package can be used independently of the main project.

## 🎯 Overview

Chess Humanizer predicts chess moves that reflect human decision-making patterns rather than perfect engine play. It uses context-aware predictions to generate moves that consider:

- Game phase (opening, middlegame, endgame)
- Time control (blitz, rapid, classical)
- Position complexity and blunder potential
- Player skill levels
- Tactical vs positional considerations

## 📦 Installation

1. **Clone or download this SABLE folder**
2. **Install dependencies:**
   ```bash
   pip install -r requirements.txt
   ```

3. **Add a trained model:**
   - Place your trained model files in the `models/` directory
   - You need both `*_model.pth` and `*_vocab.pkl` files
   - Example: `models/blitz_model_model.pth` and `models/blitz_model_vocab.pkl`

## 🚀 Quick Start

### Basic Usage

```python
from chess_humanizer import ChessMovePredictor

# Initialize predictor
predictor = ChessMovePredictor("models/blitz_model")

# Predict moves for a position
fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
predictions = predictor.predict_moves(fen, num_moves=5, temperature=1.0)

# Display results
for pred in predictions:
    print(f"{pred['move']} - {pred['probability']:.3f}")
```

### Command Line Interface

```bash
# Basic prediction
python predict_moves.py --model_path models/blitz_model --fen "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" --num_moves 5

# With custom context
python predict_moves.py --model_path models/blitz_model --fen "..." --context_file examples/context_template.json

# Position analysis
python predict_moves.py --model_path models/blitz_model --fen "..." --analyze
```

### UCI Engine Interface

```bash
# Run as UCI engine
python uci_engine.py models/blitz_model

# Use with chess applications that support UCI engines
# (Chess.com, Lichess, ChessBase, etc.)
```

## 📁 File Structure

```
SABLE/
├── chess_humanizer.py      # Main module with all classes
├── predict_moves.py        # Command-line interface
├── uci_engine.py          # UCI protocol wrapper
├── requirements.txt       # Python dependencies
├── README.md             # This file
├── models/               # Place trained models here
│   ├── blitz_model_model.pth
│   └── blitz_model_vocab.pkl
└── examples/             # Usage examples
    ├── basic_usage.py
    ├── advanced_usage.py
    └── context_template.json
```

## 🎮 Usage Examples

### 1. Basic Move Prediction

```python
from chess_humanizer import ChessMovePredictor

predictor = ChessMovePredictor("models/blitz_model")

# Starting position
fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
predictions = predictor.predict_moves(fen, num_moves=5)

for pred in predictions:
    print(f"{pred['rank']}. {pred['move']} ({pred['probability']:.3f})")
```

### 2. Custom Context

```python
# Create custom context for a blitz game
context = {
    'time_control': 'blitz',
    'per_move_time': 10.0,
    'white_elo': 2000,
    'black_elo': 2000,
    'blunder_potential': 0.4,
    'complexity': 30,
    'move_quality': 'aggressive'
}

predictions = predictor.predict_with_custom_context(
    fen, context, num_moves=3, temperature=0.8
)
```

### 3. Position Analysis

```python
# Get detailed position analysis
analysis = predictor.analyze_position(fen)
print(f"Turn: {analysis['turn']}")
print(f"Legal moves: {analysis['legal_moves_count']}")
print(f"Game phase: {analysis['context_features']['game_phase']}")
```

### 4. Temperature Control

```python
# Conservative play (low temperature)
conservative = predictor.predict_moves(fen, temperature=0.5)

# Creative play (high temperature)
creative = predictor.predict_moves(fen, temperature=1.5)
```

## 🔧 API Reference

### ChessMovePredictor

**Main class for move prediction.**

#### Methods:

- `__init__(model_path: str)` - Initialize with trained model
- `predict_moves(fen, num_moves=5, temperature=1.0, context=None)` - Predict moves
- `predict_with_custom_context(fen, context, num_moves=5, temperature=1.0)` - Predict with custom context
- `analyze_position(fen)` - Analyze position and return context

#### Parameters:

- `fen` (str): FEN string of the position
- `num_moves` (int): Number of moves to predict (1-10)
- `temperature` (float): Sampling temperature (0.1-2.0)
- `context` (dict): Custom context features

#### Returns:

List of dictionaries with:
- `move` (str): SAN notation of the move
- `uci` (str): UCI notation of the move
- `probability` (float): Move probability (0-1)
- `rank` (int): Move ranking (1-based)

## 🎭 Context Features

The model uses context features to generate human-like moves:

### Game Context:
- `eval_score`: Position evaluation (centipawns)
- `eval_delta`: Evaluation change from previous move
- `blunder_potential`: Risk of blundering (0-1)
- `complexity`: Position complexity (1-50)
- `material_balance`: Material advantage (centipawns)

### Game State:
- `move_number`: Current move number
- `game_phase`: "opening", "middlegame", or "endgame"
- `time_control`: "blitz", "rapid", or "classical"
- `per_move_time`: Average time per move (seconds)

### Player Context:
- `white_elo` / `black_elo`: Player skill ratings
- `center_control`: Control of center squares
- `king_safety`: King safety score (0-1)
- `pawn_structure`: Pawn structure evaluation

### Position Features:
- `is_capture`: Whether position has capture opportunities
- `is_check`: Whether king is in check
- `is_checkmate` / `is_stalemate`: Terminal positions
- `move_quality`: "normal", "tactical", "aggressive", "cautious"

## 🌡️ Temperature Control

Temperature controls the randomness of move selection:

- **0.1-0.5**: Very conservative, always plays the most likely move
- **0.5-1.0**: Balanced play, considers multiple options
- **1.0-1.5**: Creative play, explores less likely moves
- **1.5-2.0**: Very creative, may play unusual moves

## 🔌 Integration

### UCI Engine

The system can be used as a UCI-compatible chess engine:

```bash
python uci_engine.py models/blitz_model
```

This allows integration with:
- Chess.com
- Lichess
- ChessBase
- Arena
- Any UCI-compatible chess application

### Python API

```python
# Simple integration
predictor = ChessMovePredictor("models/blitz_model")
best_move = predictor.predict_moves(fen, num_moves=1)[0]['uci']

# Batch processing
positions = [fen1, fen2, fen3, ...]
for fen in positions:
    predictions = predictor.predict_moves(fen)
    # Process predictions...
```

## 📊 Performance

Typical performance characteristics:
- **Inference time**: ~10-50ms per position (GPU), ~100-500ms (CPU)
- **Memory usage**: ~2-4GB for model loading
- **Accuracy**: Varies by model, typically 15-25% top-1 accuracy
- **Context awareness**: Full support for all context features

## 🛠️ Troubleshooting

### Common Issues:

1. **Model not found**: Ensure model files are in `models/` directory
2. **CUDA out of memory**: Use CPU mode or reduce batch size
3. **Invalid FEN**: Check FEN string format
4. **Import errors**: Install all requirements with `pip install -r requirements.txt`

### Debug Mode:

```python
# Enable detailed logging
import logging
logging.basicConfig(level=logging.DEBUG)

predictor = ChessMovePredictor("models/blitz_model")
```

## 📝 Examples

See the `examples/` directory for complete usage examples:

- `basic_usage.py`: Simple move prediction
- `advanced_usage.py`: Advanced features and scenarios
- `context_template.json`: Template for custom contexts

## 🤝 Contributing

This is a standalone release. For development and contributions, see the main project repository.

## 📄 License

This release is provided as-is for standalone use. See the main project for licensing information.

## 🆘 Support

For issues and questions:
1. Check the troubleshooting section
2. Review the examples in `examples/`
3. Ensure all dependencies are installed
4. Verify model files are correctly placed

---

**Chess Humanizer SABLE Release v1.0.0** 