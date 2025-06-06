import sys
import os
import time
import torch
import hashlib
import numpy as np
from PIL import Image, ImageDraw, ImageFont
from io import TextIOWrapper
from dataset import PIECE_TO_IDX
from ccn_model import CCN
from skimage.metrics import structural_similarity as ssim

IDX_TO_PIECE = {v: k for k, v in PIECE_TO_IDX.items()}
last_image_array = None
last_hash = None
last_board_fen = None
last_turn = None
frame_counter = 0

os.makedirs("debug_frames", exist_ok=True)


def save_debug_image(image_path, preds, output_path="debug_frame.png", turn=None, frame_id=None):
    img = Image.open(image_path).convert("RGB").resize((256, 256))
    draw = ImageDraw.Draw(img)
    square_size = 256 // 8

    try:
        font = ImageFont.truetype("arial.ttf", 20)
    except:
        font = ImageFont.load_default()

    for row in range(8):
        for col in range(8):
            x = col * square_size
            y = row * square_size
            draw.rectangle([x, y, x + square_size, y + square_size], outline="gray", width=1)
            idx = int(preds[row][col])
            piece = IDX_TO_PIECE[idx]
            if piece != ".":
                draw.text((x + 5, y + 2), piece, fill="red", font=font)

    # 🧠 Turn label
    if turn is not None:
        draw.rectangle([0, 0, 110, 22], fill="black")
        draw.text((5, 2), f"Turn: {turn.upper()}", fill="white", font=font)

    # 🧠 Frame label
    if frame_id is not None:
        draw.rectangle([150, 0, 250, 22], fill="black")
        draw.text((155, 2), f"Frame: {frame_id}", fill="white", font=font)

    img.save(output_path)


def hash_image_gray_array(arr):
    return hashlib.sha256(arr.tobytes()).hexdigest()


def load_model(path="ccn_model_default.pth", device=None):
    script_dir = os.path.dirname(os.path.realpath(__file__))
    model_path = os.path.join(script_dir, path)
    print(f"[MODEL] Loading from {model_path}", file=sys.stderr)

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


def detect_moved_color(old_fen, new_fen):
    def expand_row(row):
        return "".join("." * int(ch) if ch.isdigit() else ch for ch in row)

    old_rows = old_fen.split("/")
    new_rows = new_fen.split("/")
    if len(old_rows) != 8 or len(new_rows) != 8:
        return None

    for r in range(8):
        o_row = expand_row(old_rows[r])
        n_row = expand_row(new_rows[r])
        for c in range(8):
            if o_row[c] != n_row[c]:
                if n_row[c].islower():
                    return "b"
                elif n_row[c].isupper():
                    return "w"
    return None


def predict_fen(model, image_tensor, image_path=None, save_debug=False, debug_output_path="debug_frame.png", turn=None, frame_id=None):
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
    fen = "/".join(fen_rows) + f" ? {castling} - 0 1"

    if save_debug and image_path is not None:
        save_debug_image(image_path, preds, output_path=debug_output_path, turn=turn, frame_id=frame_id)

    return fen


# === Main Loop ===
model = load_model()
print("ready", flush=True)
stdin = TextIOWrapper(sys.stdin.buffer, encoding='utf-8')

while True:
    try:
        line = stdin.readline().strip()
        if not line:
            continue

        frame_id = frame_counter
        frame_counter += 1
        overall_start = time.time()

        current_img = load_image_gray_array(line)
        ssim_score = ssim(current_img, last_image_array) if last_image_array is not None else 0
        if ssim_score > 0.997:
            print(f"[frame {frame_id:04d}] [SKIP] not stable (SSIM: {ssim_score:.4f})", flush=True)
            continue

        last_image_array = current_img

        tensor = load_image_tensor(line)
        raw_fen = predict_fen(model, tensor, image_path=line, save_debug=False)

        fen_parts = (raw_fen.strip().split(" ") + ["-"] * 6)[:6]
        board_only_fen = fen_parts[0]

        if last_board_fen is None:
            last_board_fen = board_only_fen
            last_turn = "w"
            print("[first board]", flush=True)
            continue

        moved_color = detect_moved_color(last_board_fen, board_only_fen)
        if moved_color is None:
            turn = 'b' if last_turn == 'w' else 'w'
            print(f"[frame {frame_id:04d}] fallback: unknown turn, assuming {turn}", flush=True)
        else:
            turn = 'b' if moved_color == 'w' else 'w'

        last_turn = turn
        last_board_fen = board_only_fen
        fen_parts[1] = turn
        final_fen = " ".join(fen_parts)

        debug_path = f"debug_frames/frame_{frame_id:04d}.png"
        predict_fen(model, tensor, image_path=line, save_debug=True, debug_output_path=debug_path, turn=turn, frame_id=frame_id)

        print(final_fen, flush=True)
        print(f"[frame {frame_id:04d}] debug saved", flush=True)

    except Exception as e:
        print(f"[error] {e}", flush=True)
        sys.exit(2)
