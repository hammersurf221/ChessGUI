import sys
import time
import math
import pyautogui
import random

def bezier(p0, p1, p2, p3, t):
    """Return a point on a cubic Bezier curve for parameter t."""
    mt = 1 - t
    x = (
        mt ** 3 * p0[0]
        + 3 * mt ** 2 * t * p1[0]
        + 3 * mt * t ** 2 * p2[0]
        + t ** 3 * p3[0]
    )
    y = (
        mt ** 3 * p0[1]
        + 3 * mt ** 2 * t * p1[1]
        + 3 * mt * t ** 2 * p2[1]
        + t ** 3 * p3[1]
    )
    return x, y


def human_like_drag(start, end, steps=30):
    x1, y1 = start
    x4, y4 = end

    # Slightly randomized control points for natural variation
    ctrl1 = (x1 + random.randint(-15, 15), y1 + random.randint(-15, 15))
    ctrl2 = (x4 + random.randint(-15, 15), y4 + random.randint(-15, 15))

    pyautogui.moveTo(x1, y1)
    pyautogui.mouseDown()

    for i in range(steps + 1):
        t = i / steps
        # ease-in/ease-out using a sinusoidal curve
        ease_t = 0.5 - math.cos(t * math.pi) / 2
        bx, by = bezier((x1, y1), ctrl1, ctrl2, (x4, y4), ease_t)
        jx = bx + random.uniform(-2, 2)
        jy = by + random.uniform(-2, 2)
        pyautogui.moveTo(jx, jy, duration=0)
        time.sleep(0.01 + random.uniform(0, 0.01))

    jitter_x = random.choice([-2, -1, 1, 2])
    jitter_y = random.choice([-2, -1, 1, 2])
    pyautogui.moveTo(x4 + jitter_x, y4 + jitter_y, duration=0)
    time.sleep(random.uniform(0.1, 0.5))
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
