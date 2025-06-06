# core/move_detector.py

def detect_move(prev, curr):
    diffs = []
    for r in range(8):
        for c in range(8):
            if prev[r][c] != curr[r][c]:
                diffs.append((r, c))

    if len(diffs) not in (2, 3, 4):
        return None, None

    moved_from = None
    moved_to = None

    for r, c in diffs:
        if prev[r][c] != 0 and curr[r][c] == 0:
            moved_from = (r, c)
        elif prev[r][c] == 0 and curr[r][c] != 0:
            moved_to = (r, c)
        elif prev[r][c] != curr[r][c] and curr[r][c] != 0:
            moved_to = (r, c)

    return moved_from, moved_to
