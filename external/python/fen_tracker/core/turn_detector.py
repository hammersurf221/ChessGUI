# core/turn_detector.py

from PIL import Image
import numpy as np
from skimage.metrics import structural_similarity as ssim
from utils.board_utils import square_name, IDX_TO_PIECE

SSIM_THRESHOLD = 0.98

def split_board_into_squares(image: Image.Image):
    squares = {}
    width, height = image.size
    square_w, square_h = width // 8, height // 8
    for row in range(8):
        for col in range(8):
            left = col * square_w
            upper = row * square_h
            right = left + square_w
            lower = upper + square_h
            square_img = image.crop((left, upper, right, lower))
            name = square_name(row, col)
            squares[name] = square_img
    return squares

def detect_turn_from_images(prev_image: Image.Image, curr_image: Image.Image, prev_board: np.ndarray):
    prev_squares = split_board_into_squares(prev_image)
    curr_squares = split_board_into_squares(curr_image)

    changed = []
    for name in prev_squares:
        prev_sq = np.array(prev_squares[name])
        curr_sq = np.array(curr_squares[name])
        similarity = ssim(prev_sq, curr_sq, channel_axis=2)
        if similarity < SSIM_THRESHOLD:
            changed.append(name)

    for name in changed:
        row = 8 - int(name[1])
        col = ord(name[0]) - ord('a')
        prev_piece = IDX_TO_PIECE[prev_board[row][col]]
        if prev_piece != '.':
            return 'w' if prev_piece.isupper() else 'b'

    return None
