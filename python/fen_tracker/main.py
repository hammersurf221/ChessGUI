# main.py
import argparse
import os
import torch
import torch.nn.functional as F
from torchvision import transforms
from PIL import Image
from ccn_model import CCN
from core.game_state_tracker import GameStateTracker
from utils.board_utils import flip_fen_pov
from skimage.metrics import structural_similarity as ssim
import numpy as np
from collections import deque


parser = argparse.ArgumentParser()
parser.add_argument("--color", choices=["w", "b"], default="w")
args = parser.parse_args()
my_color = args.color
print(f"[startup] my_color = {my_color}", flush=True)



def predict_board(model, image_tensor):
    model.eval()
    with torch.no_grad():
        out = model(image_tensor.unsqueeze(0))  # [1, 8, 8, 13]
        pred = F.softmax(out, dim=-1).argmax(dim=-1)  # [1, 8, 8]
    return pred.squeeze(0).cpu().numpy()

last_image_array = None
last_ssim = 0.0
current_ssim = 0.0
last_emitted_fen = None
SSIM_THRESHOLD = 0.98

def main():
    import sys
    from collections import deque

    model = CCN()

    script_dir = os.path.dirname(os.path.abspath(__file__))
    model_path = os.path.join(script_dir, "ccn_model_default.pth")

    model.load_state_dict(torch.load(model_path, map_location="cpu"))

    model.eval()

    transform = transforms.Compose([
        transforms.Resize((256, 256)),
        transforms.ToTensor()
    ])

    tracker = GameStateTracker()
    print("ready")
    sys.stdout.flush()

    for line in sys.stdin:
        path = os.path.abspath(line.strip())
        print(f"[python received] {path}", flush=True)

        try:
            image = Image.open(path).convert("RGB")
            image_array = np.array(image)

            global last_image_array, last_emitted_fen, last_ssim, current_ssim

            if last_image_array is None:
                # Initialize on the first frame
                last_image_array = np.copy(image_array)
                last_ssim = 0.0
                current_ssim = 1.0
                print("[debug] First frame — initializing SSIM", flush=True)
                continue

            similarity = ssim(last_image_array, image_array, channel_axis=2)
            print(f"[debug] SSIM: {similarity}", flush=True)

            last_ssim, current_ssim = current_ssim, similarity

            if last_ssim < SSIM_THRESHOLD and current_ssim >= SSIM_THRESHOLD:
                tensor = transform(image)
                board = predict_board(model, tensor)
                tracker.update(board)
                fen = tracker.generate_fen(board)

                if my_color == 'b':
                    fen = flip_fen_pov(fen)

                if fen != last_emitted_fen:
                    print(f"[FEN] {fen}", flush=True)
                    last_emitted_fen = fen
                else:
                    print("[skip] FEN unchanged — skipping output", flush=True)
            else:
                print("[skip] Board not stable yet", flush=True)

            # store the current frame for next comparison
            last_image_array = np.copy(image_array)

        except Exception as e:
            print(f"[error] {e}", flush=True)

        sys.stdout.flush()

if __name__ == "__main__":
    main()
