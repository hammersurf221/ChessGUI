import sys
import time
import math
import pyautogui
import random

def bezier(p0, p1, p2, p3, t):
    mt = 1 - t
    x = (mt ** 3 * p0[0] + 3 * mt ** 2 * t * p1[0] +
         3 * mt * t ** 2 * p2[0] + t ** 3 * p3[0])
    y = (mt ** 3 * p0[1] + 3 * mt ** 2 * t * p1[1] +
         3 * mt * t ** 2 * p2[1] + t ** 3 * p3[1])
    return x, y

def human_like_drag(start, end, steps=30):
    x1, y1 = start
    x4, y4 = end
    ctrl1 = (x1 + random.randint(-20, 20), y1 + random.randint(-20, 20))
    ctrl2 = (x4 + random.randint(-20, 20), y4 + random.randint(-20, 20))

    pyautogui.moveTo(x1, y1)
    pyautogui.mouseDown()

    for i in range(steps + 1):
        t = i / steps
        ease_t = 0.5 - math.cos(t * math.pi) / 2
        bx, by = bezier((x1, y1), ctrl1, ctrl2, (x4, y4), ease_t)
        jx = bx + random.uniform(-2, 2)
        jy = by + random.uniform(-2, 2)
        pyautogui.moveTo(jx, jy, duration=0)
        time.sleep(0.01 + random.uniform(0.005, 0.015))

    pyautogui.moveTo(x4 + random.randint(-3, 3), y4 + random.randint(-3, 3), duration=0)
    time.sleep(random.uniform(0.15, 0.5))
    pyautogui.mouseUp()

def random_square_point(square_coords, tile_size):
    x, y = square_coords
    return (
        x + random.randint(-tile_size//3, tile_size//3),
        y + random.randint(-tile_size//3, tile_size//3)
    )

def main():
    if len(sys.argv) < 7:
        print("Usage: play_move.py from_sq to_sq originX originY tileSize flipped [stealth]", file=sys.stderr)
        sys.exit(2)

    from_sq = sys.argv[1]
    to_sq = sys.argv[2]
    origin_x = int(sys.argv[3])
    origin_y = int(sys.argv[4])
    tile_size = int(sys.argv[5])
    flipped = sys.argv[6].lower() == "true"
    stealth = sys.argv[7].lower() == "true" if len(sys.argv) >= 8 else False

    def square_to_center(square):
        file = ord(square[0]) - ord('a')
        rank = int(square[1]) - 1
        if flipped:
            file = 7 - file
            rank = 7 - rank
        x = origin_x + file * tile_size + tile_size // 2
        y = origin_y + (7 - rank) * tile_size + tile_size // 2
        return (x, y)

    from_center = square_to_center(from_sq)
    to_center = square_to_center(to_sq)
    from_px = random_square_point(from_center, tile_size)
    to_px = random_square_point(to_center, tile_size)

    if stealth:
        # Hover on nearby square
        hover = (
            from_px[0] + random.randint(-40, 40),
            from_px[1] + random.randint(-40, 40)
        )
        pyautogui.moveTo(*hover)
        time.sleep(random.uniform(0.3, 0.8))

        # Fake click (misclick simulation)
        if random.random() < 0.3:
            pyautogui.click()
            time.sleep(random.uniform(0.2, 0.5))

        # Hover actual piece
        pyautogui.moveTo(*from_px)
        time.sleep(random.uniform(0.4, 1.2))

        # Human-like drag
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
