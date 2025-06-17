import sys
import time
import math
import random
import argparse
from human_mouse import simulate_human_hover, human_like_drag

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

    # Simulate hover
    simulate_human_hover(from_px)

    # Fake misclick
    if random.random() < 0.25:
        import pyautogui
        pyautogui.click()
        misclick = True
        time.sleep(random.uniform(0.2, 0.4))

    # Wait before real move
    time.sleep(delay_ms / 1000)

    # Final drag
    human_like_drag(from_px, to_px)

    print(f"[human] Moving {from_sq} to {to_sq}: delay={delay_ms}ms, misclick={misclick}, from={from_px}, to={to_px}")

if __name__ == "__main__":
    main()
