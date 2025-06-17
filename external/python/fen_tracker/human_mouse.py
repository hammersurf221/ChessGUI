import math
import random
import time
import pyautogui


def bezier(p0, p1, p2, p3, t):
    mt = 1 - t
    x = (mt**3 * p0[0] + 3 * mt**2 * t * p1[0] +
         3 * mt * t**2 * p2[0] + t**3 * p3[0])
    y = (mt**3 * p0[1] + 3 * mt**2 * t * p1[1] +
         3 * mt * t**2 * p2[1] + t**3 * p3[1])
    return x, y


def simulate_human_hover(point, radius=20):
    x, y = point
    for _ in range(random.randint(2, 4)):
        offset_x = random.uniform(-radius, radius)
        offset_y = random.uniform(-radius, radius)
        pyautogui.moveTo(x + offset_x, y + offset_y, duration=random.uniform(0.05, 0.12))
        time.sleep(random.uniform(0.03, 0.08))


def simulate_micro_adjustments(point):
    for _ in range(random.randint(1, 3)):
        offset_x = random.uniform(-6, 6)
        offset_y = random.uniform(-6, 6)
        pyautogui.moveTo(point[0] + offset_x, point[1] + offset_y, duration=0.05)
        time.sleep(random.uniform(0.03, 0.07))


def human_like_drag(start, end, steps=40):
    x1, y1 = start
    x4, y4 = end

    ctrl1 = (x1 + random.randint(-60, 60), y1 + random.randint(-60, 60))
    ctrl2 = (x4 + random.randint(-60, 60), y4 + random.randint(-60, 60))

    pyautogui.moveTo(x1 + random.uniform(-3, 3), y1 + random.uniform(-3, 3), duration=0)
    pyautogui.mouseDown()

    for i in range(steps + 1):
        t = i / steps

        # Friction-like easing
        ease_t = t ** 0.4 if t < 0.5 else 1 - (1 - t) ** 0.4

        bx, by = bezier((x1, y1), ctrl1, ctrl2, (x4, y4), ease_t)
        jx = bx + random.uniform(-2.5, 2.5)
        jy = by + random.uniform(-2.5, 2.5)
        pyautogui.moveTo(jx, jy, duration=0)

        # Fake lag
        if random.random() < 0.07 and i > 5:
            back_x = jx - random.uniform(2, 4)
            back_y = jy - random.uniform(2, 4)
            pyautogui.moveTo(back_x, back_y, duration=0.02)
            time.sleep(random.uniform(0.02, 0.04))

        time.sleep(random.uniform(0.004, 0.010))

    simulate_micro_adjustments(end)
    pyautogui.moveTo(x4 + random.uniform(-3, 3), y4 + random.uniform(-3, 3), duration=0)
    pyautogui.mouseUp()

    # ✅ Simulate subtle drift after move release
    if random.random() < 0.8:
        time.sleep(0.1)  # small pause to simulate "resting"
        pyautogui.moveRel(random.choice([-1, 1]), random.choice([-1, 1]), duration=0.03)

