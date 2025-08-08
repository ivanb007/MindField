// evaluate.cpp
// This file implements the evaluation function for the chess engine.
#include "evaluate.h"
#include "engine.h"

int evaluate(const BoardData& board) {
    int score = 0;
    for (int i = 0; i < 64; ++i) {
        int p = board.piece[i];
        if (p == EMPTY) continue; // Skip empty squares
        int val;
        switch (p) {
            case PAWN: val = 10; break; // Pawn
            case KNIGHT: case BISHOP: val = 30; break; // Knight or Bishop
            case ROOK: val = 50; break; // Rook
            case QUEEN: val = 90; break; // Queen
            case KING: val = 900; break; // King
            default: val = 0; // Unknown piece
        }
        // Add or subtract score based on piece color
        if (board.color[i] == BLACK) {
            score = -score; // Negate score for black pieces
        }
        else if (board.color[i] == WHITE) {
            score += val; // Add score for white pieces
        }
    }
    return score;
}