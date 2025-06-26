import sys
import random

print("ready", flush=True)

# Placeholder best move generator
moves = ["e2e4", "d2d4", "c2c4", "g1f3"]

for line in sys.stdin:
    fen = line.strip()
    if not fen:
        continue
    best = random.choice(moves)
    print(f"bestmove {best}", flush=True)
