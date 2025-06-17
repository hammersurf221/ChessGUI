# core/fen_counters.py

class FenCounters:
    def __init__(self, starting_turn='w'):
        self.halfmove_clock = 0
        self.fullmove_number = 1
        self.last_turn = starting_turn

    def update(self, move_color, moved_piece, was_capture=False, was_pawn_move=False):
        # Reset halfmove clock if pawn move or capture
        if was_capture or was_pawn_move:
            self.halfmove_clock = 0
        else:
            self.halfmove_clock += 1

        # Increment fullmove after Black's move
        if move_color == 'b':
            self.fullmove_number += 1

        self.last_turn = move_color

    def get_counts(self):
        return self.halfmove_clock, self.fullmove_number
