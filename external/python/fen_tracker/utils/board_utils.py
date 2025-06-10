# utils/board_utils.py

IDX_TO_PIECE = {
    0: '.', 1: 'P', 2: 'N', 3: 'B', 4: 'R', 5: 'Q', 6: 'K',
    7: 'p', 8: 'n', 9: 'b', 10: 'r', 11: 'q', 12: 'k'
}
PIECE_TO_IDX = {v: k for k, v in IDX_TO_PIECE.items()}


def square_name(row, col):
    return chr(ord('a') + col) + str(8 - row)


def matrix_to_fen(board):
    fen_rows = []
    for row in board:
        row_fen = ''
        empty = 0
        for square in row:
            piece = IDX_TO_PIECE[square]
            if piece == '.':
                empty += 1
            else:
                if empty:
                    row_fen += str(empty)
                    empty = 0
                row_fen += piece
        if empty:
            row_fen += str(empty)
        fen_rows.append(row_fen)
    return '/'.join(fen_rows)

def flip_fen_pov(fen):
    parts = fen.strip().split()
    if len(parts) != 6:
        raise ValueError("Invalid FEN format")

    # 1. Flip board piece layout 180°
    ranks = parts[0].split('/')
    rotated_ranks = []
    for rank in reversed(ranks):
        expanded = []
        for ch in rank:
            if ch.isdigit():
                expanded.extend(['.'] * int(ch))
            else:
                expanded.append(ch)
        expanded.reverse()
        # Compress back into FEN format
        new_rank = ''
        empty = 0
        for ch in expanded:
            if ch == '.':
                empty += 1
            else:
                if empty:
                    new_rank += str(empty)
                    empty = 0
                new_rank += ch
        if empty:
            new_rank += str(empty)
        rotated_ranks.append(new_rank)
    parts[0] = '/'.join(rotated_ranks)

    # 2. Keep side to move unchanged — we're rotating, not changing players
    # 3. Flip castling rights geometrically: KQkq ↔ kqKQ
    castle = parts[2]
    swap = str.maketrans('KQkq', 'kqKQ')
    parts[2] = castle.translate(swap)

    # 4. Flip en passant square geometrically
    if parts[3] != '-':
        file = parts[3][0]
        rank = parts[3][1]
        file_flipped = chr(ord('h') - (ord(file) - ord('a')))
        rank_flipped = str(9 - int(rank))
        parts[3] = file_flipped + rank_flipped

    return ' '.join(parts)

if __name__ == "__main__":
    test_fen = "r1bqkbnr/pppppppp/2n5/8/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 2 2"
    flipped = flip_fen_pov(test_fen)
    print("Original FEN:", test_fen)
    print("Flipped FEN: ", flipped)
