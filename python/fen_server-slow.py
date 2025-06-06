import sys
import os
import time
import torch
import numpy as np
from PIL import Image
from io import TextIOWrapper
from dataset import PIECE_TO_IDX
from ccn_model import CCN
from skimage.metrics import structural_similarity as ssim

IDX_TO_PIECE = {v: k for k, v in PIECE_TO_IDX.items()}
global last_image_array # Holds last screenshot for SSIM diff
last_image_array = None  # Holds last screenshot for SSIM diff

def expand_row(row):
    expanded = ""
    for ch in row:
        if ch.isdigit():
            expanded += "." * int(ch)
        else:
            expanded += ch
    return expanded

def detect_moved_color(old_fen, new_fen):
    old_rows = old_fen.split("/")
    new_rows = new_fen.split("/")

    if len(old_rows) != 8 or len(new_rows) != 8:
        return None

    for r in range(8):
        o_row = expand_row(old_rows[r])
        n_row = expand_row(new_rows[r])
        for c in range(8):
            if o_row[c] != n_row[c]:
                if o_row[c] != n_row[c]:
                    if o_row[c].isupper() or n_row[c].isupper():
                        return "w"
                    elif o_row[c].islower() or n_row[c].islower():
                        return "b"
    return None

def load_model(path="ccn_model_default.pth", device=None):
    script_dir = os.path.dirname(os.path.abspath(__file__))
    model_path = os.path.join(script_dir, path)
    model = CCN()
    model.load_state_dict(torch.load(model_path, map_location=device or torch.device("cpu")))
    model.eval()
    return model

def load_image_tensor(path):
    img = Image.open(path).convert("RGB").resize((256, 256))
    tensor = torch.from_numpy(np.array(img)).permute(2, 0, 1).float() / 255.0
    return tensor.unsqueeze(0)

def load_image_gray_array(path):
    return np.array(Image.open(path).convert("L").resize((256, 256)))

def detect_castling_rights(fen_rows):
    board = []
    for row in fen_rows:
        expanded = ""
        for char in row:
            if char.isdigit():
                expanded += '.' * int(char)
            else:
                expanded += char
        board.append(expanded)

    rights = ""
    if board[7][4] == 'K':
        if board[7][7] == 'R': rights += 'K'
        if board[7][0] == 'R': rights += 'Q'
    if board[0][4] == 'k':
        if board[0][7] == 'r': rights += 'k'
        if board[0][0] == 'r': rights += 'q'

    return rights or "-"

def predict_fen(model, image_tensor):
    with torch.no_grad():
        output = model(image_tensor)
        preds = output.argmax(dim=-1).squeeze(0)

    fen_rows = []
    for row in preds:
        fen_row = ""
        empty = 0
        for idx in row:
            piece = IDX_TO_PIECE[int(idx)]
            if piece == ".":
                empty += 1
            else:
                if empty > 0:
                    fen_row += str(empty)
                    empty = 0
                fen_row += piece
        if empty > 0:
            fen_row += str(empty)
        fen_rows.append(fen_row)

    castling = detect_castling_rights(fen_rows)
    return "/".join(fen_rows) + f" ? {castling} - 0 1"  # Note: turn is '?'

# Load model once
model = load_model()
print("ready", flush=True)

stdin = TextIOWrapper(sys.stdin.buffer, encoding='utf-8')
while True:
    line = stdin.readline().strip()
    if not line:
        continue

    try:
        overall_start = time.time()

        step = lambda: time.time() - overall_start

        # Step 1: Load grayscale image
        current_img = load_image_gray_array(line)
        t1 = step()

        # Step 2: SSIM (skip if needed)
        if last_image_array is not None:
            score = ssim(current_img, last_image_array)
            print(f"[ssim] {score}", file=sys.stderr, flush=True)
            if score > 0.995:
                print("[unchanged]", flush=True)
                continue

        t2 = step()

        last_image_array = current_img

        # Step 3: Load tensor for model
        tensor = load_image_tensor(line)
        t3 = step()

        # Step 4: Predict FEN
        raw_fen = predict_fen(model, tensor)
        t4 = step()

        fen_parts = (raw_fen.strip().split(" ") + ["-"] * 6)[:6]
        board_only_fen = fen_parts[0]

        if 'last_board_fen' not in globals():
            last_board_fen = board_only_fen
            print("[first board]", flush=True)
            continue

        # Step 5: Turn detection
        moved_color = detect_moved_color(last_board_fen, board_only_fen)
        t5 = step()

        if moved_color is None:
            print("[undetected]", flush=True)
            continue

        turn = "b" if moved_color == "w" else "w"
        fen_parts[1] = turn
        final_fen = " ".join(fen_parts)

        last_board_fen = board_only_fen
        print(final_fen, flush=True)

        # Final: Report timing
        print(f"[TIMING] grayscale: {t1:.3f}s, ssim: {t2 - t1:.3f}s, tensor: {t3 - t2:.3f}s, predict: {t4 - t3:.3f}s, detect_turn: {t5 - t4:.3f}s, total: {t5:.3f}s", file=sys.stderr, flush=True)

    except Exception as e:
        print(f"[error] {e}", flush=True)
        sys.exit(2)


