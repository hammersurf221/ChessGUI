import sys
import time
import math
import random
import argparse
import pyautogui
from human_mouse import simulate_human_hover, bezier, simulate_micro_adjustments

random.seed(time.time())

def random_square_point(center, tile_size):
    # Avoid center: pick a point in a ring around center
    radius = tile_size * 0.3
    angle = random.uniform(0, 2 * math.pi)
    dx = int(math.cos(angle) * radius + random.uniform(-5, 5))
    dy = int(math.sin(angle) * radius + random.uniform(-5, 5))
    return (center[0] + dx, center[1] + dy)

def generate_human_delay(phase, complexity, eval_score):
    base = {
        "opening": 800,
        "mid": 1600,
        "end": 1200,
    }.get(phase, 1000)
    delay = base + complexity * 200
    if eval_score is not None and abs(eval_score) < 0.3 and complexity >= 3:
        delay += 400
    delay += random.randint(-100, 250)
    return max(300, delay)


def human_like_move(start, end, steps=30):
    x1, y1 = start
    x4, y4 = end
    ctrl1 = (x1 + random.randint(-40, 40), y1 + random.randint(-40, 40))
    ctrl2 = (x4 + random.randint(-40, 40), y4 + random.randint(-40, 40))

    for i in range(steps + 1):
        t = i / steps
        ease_t = t ** 0.4 if t < 0.5 else 1 - (1 - t) ** 0.4
        bx, by = bezier((x1, y1), ctrl1, ctrl2, (x4, y4), ease_t)
        pyautogui.moveTo(bx, by, duration=0)
        time.sleep(random.uniform(0.004, 0.010))


def drag_from_current(end, steps=40):
    x1, y1 = pyautogui.position()
    x4, y4 = end
    ctrl1 = (x1 + random.randint(-60, 60), y1 + random.randint(-60, 60))
    ctrl2 = (x4 + random.randint(-60, 60), y4 + random.randint(-60, 60))

    pyautogui.mouseDown()
    for i in range(steps + 1):
        t = i / steps
        ease_t = t ** 0.4 if t < 0.5 else 1 - (1 - t) ** 0.4
        bx, by = bezier((x1, y1), ctrl1, ctrl2, (x4, y4), ease_t)
        jx = bx + random.uniform(-2.5, 2.5)
        jy = by + random.uniform(-2.5, 2.5)
        pyautogui.moveTo(jx, jy, duration=0)
        if random.random() < 0.07 and i > 5:
            back_x = jx - random.uniform(2, 4)
            back_y = jy - random.uniform(2, 4)
            pyautogui.moveTo(back_x, back_y, duration=0.02)
            time.sleep(random.uniform(0.02, 0.04))
        time.sleep(random.uniform(0.004, 0.010))

    simulate_micro_adjustments(end)
    pyautogui.moveTo(x4 + random.uniform(-3, 3), y4 + random.uniform(-3, 3), duration=0)
    pyautogui.mouseUp()
    if random.random() < 0.8:
        time.sleep(0.1)
        pyautogui.moveRel(random.choice([-1, 1]), random.choice([-1, 1]), duration=0.03)

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

    current_pos = pyautogui.position()
    if (abs(current_pos[0] - from_px[0]) > 3 or
            abs(current_pos[1] - from_px[1]) > 3):
        human_like_move(current_pos, from_px)

    simulate_human_hover(from_px)

    if random.random() < 0.25:
        pyautogui.click()
        misclick = True
        time.sleep(random.uniform(0.2, 0.4))

    time.sleep(delay_ms / 1000)

    drag_from_current(to_px)

    print(f"[human] Moving {from_sq} to {to_sq}: delay={delay_ms}ms, misclick={misclick}, from={from_px}, to={to_px}")

if __name__ == "__main__":
    main()
