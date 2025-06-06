# core/game_state_tracker.py

import numpy as np
from utils.board_utils import matrix_to_fen, square_name, PIECE_TO_IDX
from core.move_detector import detect_move

class GameStateTracker:
    def __init__(self):
        self.prev_board = None
        self.turn = 'w'
        self.castling_rights = {'K': True, 'Q': True, 'k': True, 'q': True}
        self.en_passant = '-'

    def update_castling_rights(self, prev, curr):
        if prev[7][4] == PIECE_TO_IDX['K'] and curr[7][4] != PIECE_TO_IDX['K']:
            self.castling_rights['K'] = False
            self.castling_rights['Q'] = False
        if prev[0][4] == PIECE_TO_IDX['k'] and curr[0][4] != PIECE_TO_IDX['k']:
            self.castling_rights['k'] = False
            self.castling_rights['q'] = False
        if prev[7][7] == PIECE_TO_IDX['R'] and curr[7][7] != PIECE_TO_IDX['R']:
            self.castling_rights['K'] = False
        if prev[7][0] == PIECE_TO_IDX['R'] and curr[7][0] != PIECE_TO_IDX['R']:
            self.castling_rights['Q'] = False
        if prev[0][7] == PIECE_TO_IDX['r'] and curr[0][7] != PIECE_TO_IDX['r']:
            self.castling_rights['k'] = False
        if prev[0][0] == PIECE_TO_IDX['r'] and curr[0][0] != PIECE_TO_IDX['r']:
            self.castling_rights['q'] = False

    def detect_en_passant(self, prev, from_sq, to_sq):
        if not from_sq or not to_sq:
            return '-'
        fr, fc = from_sq
        tr, tc = to_sq
        piece = prev[fr][fc]
        if piece == PIECE_TO_IDX['P'] and fr == 6 and tr == 4:
            return square_name(5, fc)
        if piece == PIECE_TO_IDX['p'] and fr == 1 and tr == 3:
            return square_name(2, fc)
        return '-'

    def update(self, curr_board):
        if self.prev_board is None:
            self.prev_board = curr_board.copy()
            return self.generate_fen(curr_board)

        from_sq, to_sq = detect_move(self.prev_board, curr_board)
        self.update_castling_rights(self.prev_board, curr_board)
        self.en_passant = self.detect_en_passant(self.prev_board, from_sq, to_sq)

        if from_sq and to_sq:
            self.turn = 'b' if self.turn == 'w' else 'w'
        self.prev_board = curr_board.copy()
        return self.generate_fen(curr_board)

    def generate_fen(self, board):
        piece_placement = matrix_to_fen(board)
        castling = ''.join(k for k, v in self.castling_rights.items() if v) or '-'
        return f"{piece_placement} {self.turn} {castling} {self.en_passant} 0 1"
