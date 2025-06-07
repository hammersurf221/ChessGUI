import sys
import time
import pyautogui
import random

def human_like_drag(start, end, steps=20):
    x1, y1 = start
    x2, y2 = end
    pyautogui.moveTo(x1, y1)
    pyautogui.mouseDown()
    for i in range(steps):
        t = i / steps
        ease = t ** 2 * (3 - 2 * t)  # smoothstep easing
        nx = int(x1 + (x2 - x1) * ease + random.uniform(-1.5, 1.5))
        ny = int(y1 + (y2 - y1) * ease + random.uniform(-1.5, 1.5))
        pyautogui.moveTo(nx, ny, duration=random.uniform(0.01, 0.03))
    pyautogui.moveTo(x2, y2, duration=0.05)
    pyautogui.mouseUp()

def main():
    if len(sys.argv) < 7:
        print("Usage: play_move.py from_sq to_sq originX originY tileSize flipped [stealth]", file=sys.stderr)
        sys.exit(2)  # <-- Explicit exit code


    from_sq = sys.argv[1]
    to_sq = sys.argv[2]
    origin_x = int(sys.argv[3])
    origin_y = int(sys.argv[4])
    tile_size = int(sys.argv[5])
    flipped = sys.argv[6].lower() == "true"
    stealth = sys.argv[7].lower() == "true" if len(sys.argv) >= 8 else False

    def square_to_pixel(square):
        file = ord(square[0]) - ord('a')
        rank = int(square[1]) - 1
        if flipped:
            file = 7 - file
            rank = 7 - rank
        x = origin_x + file * tile_size + tile_size // 2
        y = origin_y + (7 - rank) * tile_size + tile_size // 2
        return (x, y)

    from_px = square_to_pixel(from_sq)
    to_px = square_to_pixel(to_sq)

    if stealth:
        time.sleep(0.2)
        human_like_drag(from_px, to_px)
    else:
        pyautogui.moveTo(*from_px, duration=0.2)
        pyautogui.mouseDown()
        time.sleep(0.05)
        pyautogui.moveTo(*to_px, duration=0.2)
        time.sleep(0.05)
        pyautogui.mouseUp()

    print(f"Moved from {from_sq} to {to_sq}: {from_px} -> {to_px}")

if __name__ == "__main__":
    main()
