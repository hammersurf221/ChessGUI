# compare_images.py
import sys
from skimage.metrics import structural_similarity as ssim
from PIL import Image
import numpy as np

def load_image(path):
    img = Image.open(path).convert("L").resize((256, 256))  # grayscale
    return np.array(img)

if len(sys.argv) != 3:
    print("[error] Usage: compare_images.py image1 image2", flush=True)
    sys.exit(1)

img1_path, img2_path = sys.argv[1], sys.argv[2]

try:
    a = load_image(img1_path)
    b = load_image(img2_path)
    score = ssim(a, b)

    if score < 0.995:  # Tune this threshold
        print("changed", flush=True)
    else:
        print("unchanged", flush=True)
except Exception as e:
    print(f"[error] {e}", flush=True)
    sys.exit(2)
