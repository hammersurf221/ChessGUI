import argparse
import math
import random
import time
import pyautogui
from human_mouse import bezier, simulate_human_hover

random.seed(time.time())


def square_center(square: str, origin_x: int, origin_y: int, tile: int, flipped: bool):
    file = ord(square[0]) - ord('a')
    rank = int(square[1]) - 1
    if flipped:
        file = 7 - file
        rank = 7 - rank
    x = origin_x + file * tile + tile // 2
    y = origin_y + (7 - rank) * tile + tile // 2
    return x, y


def random_point_near(center, tile):
    radius = tile * 0.3
    angle = random.uniform(0, 2 * math.pi)
    dx = math.cos(angle) * radius + random.uniform(-5, 5)
    dy = math.sin(angle) * radius + random.uniform(-5, 5)
    return int(center[0] + dx), int(center[1] + dy)


def compute_delay(phase: str, complexity: int, eval_score: float | None):
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


def move_cursor(start, end, steps=30):
    x1, y1 = start
    x4, y4 = end
    c1 = (x1 + random.randint(-40, 40), y1 + random.randint(-40, 40))
    c2 = (x4 + random.randint(-40, 40), y4 + random.randint(-40, 40))
    for i in range(steps + 1):
        t = i / steps
        ease = t ** 0.4 if t < 0.5 else 1 - (1 - t) ** 0.4
        bx, by = bezier((x1, y1), c1, c2, (x4, y4), ease)
        pyautogui.moveTo(bx, by, duration=0)
        time.sleep(random.uniform(0.004, 0.010))


def drag_to(end, steps=40):
    x1, y1 = pyautogui.position()
    x4, y4 = end
    c1 = (x1 + random.randint(-60, 60), y1 + random.randint(-60, 60))
    c2 = (x4 + random.randint(-60, 60), y4 + random.randint(-60, 60))
    pyautogui.mouseDown()
    for i in range(steps + 1):
        t = i / steps
        ease = t ** 0.4 if t < 0.5 else 1 - (1 - t) ** 0.4
        bx, by = bezier((x1, y1), c1, c2, (x4, y4), ease)
        jx = bx + random.uniform(-2.5, 2.5)
        jy = by + random.uniform(-2.5, 2.5)
        pyautogui.moveTo(jx, jy, duration=0)
        time.sleep(random.uniform(0.004, 0.010))
    pyautogui.moveTo(x4 + random.uniform(-3, 3), y4 + random.uniform(-3, 3), duration=0)
    pyautogui.mouseUp()
    if random.random() < 0.8:
        time.sleep(0.1)
        pyautogui.moveRel(random.choice([-1, 1]), random.choice([-1, 1]), duration=0.03)


def main():
    parser = argparse.ArgumentParser(description="Execute a human-like chess move")
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

    flipped = args.flipped.lower() == "true"
    stealth = args.stealth.lower() == "true"

    from_center = square_center(args.from_sq, args.originX, args.originY, args.tileSize, flipped)
    to_center = square_center(args.to_sq, args.originX, args.originY, args.tileSize, flipped)

    from_pt = random_point_near(from_center, args.tileSize)
    to_pt = random_point_near(to_center, args.tileSize)

    delay_ms = compute_delay(args.phase, args.complexity, args.eval)
    misclick = False

    curr = pyautogui.position()
    if abs(curr[0] - from_pt[0]) > 3 or abs(curr[1] - from_pt[1]) > 3:
        move_cursor(curr, from_pt)

    simulate_human_hover(from_pt)

    if random.random() < 0.25:
        pyautogui.click()
        misclick = True
        time.sleep(random.uniform(0.2, 0.4))

    time.sleep(delay_ms / 1000)

    drag_to(to_pt)

    print(
        f"[human] {args.from_sq}{args.to_sq} phase={args.phase} "
        f"complexity={args.complexity} eval={args.eval} delay={delay_ms}ms misclick={misclick}",
        flush=True,
    )


if __name__ == "__main__":
    main()
