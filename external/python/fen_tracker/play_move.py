import sys
import time
import math
import random
import argparse

import pyautogui

random.seed(time.time())

def bezier(p0, p1, p2, p3, t):
    mt = 1 - t
    x = (mt ** 3 * p0[0] + 3 * mt ** 2 * t * p1[0] +
         3 * mt * t ** 2 * p2[0] + t ** 3 * p3[0])
    y = (mt ** 3 * p0[1] + 3 * mt ** 2 * t * p1[1] +
         3 * mt * t ** 2 * p2[1] + t ** 3 * p3[1])
    return x, y

def human_like_drag(start, end, steps=None):
    if steps is None:
        steps = random.randint(30, 40)
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
        jx = bx + random.uniform(-3, 3)
        jy = by + random.uniform(-3, 3)
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

def generate_human_delay(phase, complexity, eval_score):
    base = {
        "opening": 800,
        "mid": 1600,
        "end": 1200,
    }.get(phase, 800)
    delay = base + complexity * 200
    if eval_score is not None and abs(eval_score) < 0.3 and complexity >= 3:
        delay += 400
    delay += random.randint(-100, 250)
    return max(300, delay)

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("from_sq")
    parser.add_argument("to_sq")
    parser.add_argument("originX", type=int)
    parser.add_argument("originY", type=int)
    parser.add_argument("tileSize", type=int)
    parser.add_argument("flipped")
    parser.add_argument("stealth", nargs="?", default="false")
    parser.add_argument("--phase", choices=["opening", "mid", "end"], default="opening")
    parser.add_argument("--complexity", type=int, default=0)
    parser.add_argument("--eval", dest="eval", type=float, default=None)

    args = parser.parse_args()

    from_sq = args.from_sq
    to_sq = args.to_sq
    origin_x = args.originX
    origin_y = args.originY
    tile_size = args.tileSize
    flipped = args.flipped.lower() == "true"
    stealth = args.stealth.lower() == "true"

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

    delay_ms = generate_human_delay(args.phase, args.complexity, args.eval)
    misclick = False

    if stealth:
        hover = (
            from_px[0] + random.randint(-40, 40),
            from_px[1] + random.randint(-40, 40)
        )
        pyautogui.moveTo(*hover)
        time.sleep(random.uniform(0.2, 0.5))

        if random.random() < 0.3:
            pyautogui.click()
            misclick = True
            time.sleep(random.uniform(0.2, 0.4))

        time.sleep(delay_ms / 1000)
        pyautogui.moveTo(*from_px)
        time.sleep(random.uniform(0.2, 0.4))
        human_like_drag(from_px, to_px)
    else:
        pyautogui.moveTo(*from_px, duration=0.2)
        pyautogui.mouseDown()
        time.sleep(0.05)
        pyautogui.moveTo(*to_px, duration=0.2)
        time.sleep(0.05)
        pyautogui.mouseUp()

    mode = "[stealth]" if stealth else "[auto]"
    print(f"{mode} Moving {from_sq} to {to_sq}: delay={delay_ms}ms, misclick={misclick}, from={from_px}, to={to_px}")

if __name__ == "__main__":
    main()
