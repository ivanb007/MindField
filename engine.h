// engine.h

#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <random>

// Constant Definitions

// Constants used to indicate the colour of each side
#define WHITE			0
#define BLACK			1

// Constants used to indicate square is occupied by one of the 6 piece types:
// PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING
#define PAWN			0
#define KNIGHT			1
#define BISHOP			2
#define ROOK			3
#define QUEEN			4
#define KING			5
// Constant used to indicate an empty square
#define EMPTY			6

// Returns the row index of the square sq
constexpr int ROW(int sq) {
    return sq >> 3;
}

// Returns the column index of the square sq
constexpr int COL(int sq) {
    return sq & 7;
}

// The mailbox array is so called because it looks like a
//    mailbox, at least according to Bob Hyatt. This is useful when we
//    need to figure out what pieces can go where. Let's say we have a
//    rook on square a4 (32) and we want to know if it can move one
//    square to the left. We subtract 1, and we get 31 (h5). The rook
//    obviously can't move to h5, but we don't know that without doing
//    a lot of annoying work. Sooooo, what we do is figure out a4's
//    mailbox number, which is 61. Then we subtract 1 from 61 (60) and
//    see what mailbox[60] is. In this case, it's -1, so it's out of
//    bounds and we can forget it.

const int mailbox[120] = {
	 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	 -1,  0,  1,  2,  3,  4,  5,  6,  7, -1,
	 -1,  8,  9, 10, 11, 12, 13, 14, 15, -1,
	 -1, 16, 17, 18, 19, 20, 21, 22, 23, -1,
	 -1, 24, 25, 26, 27, 28, 29, 30, 31, -1,
	 -1, 32, 33, 34, 35, 36, 37, 38, 39, -1,
	 -1, 40, 41, 42, 43, 44, 45, 46, 47, -1,
	 -1, 48, 49, 50, 51, 52, 53, 54, 55, -1,
	 -1, 56, 57, 58, 59, 60, 61, 62, 63, -1,
	 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};

const int mailbox64[64] = {
	21, 22, 23, 24, 25, 26, 27, 28,
	31, 32, 33, 34, 35, 36, 37, 38,
	41, 42, 43, 44, 45, 46, 47, 48,
	51, 52, 53, 54, 55, 56, 57, 58,
	61, 62, 63, 64, 65, 66, 67, 68,
	71, 72, 73, 74, 75, 76, 77, 78,
	81, 82, 83, 84, 85, 86, 87, 88,
	91, 92, 93, 94, 95, 96, 97, 98
};

// The slide, offsets, and offset arrays below define the vectors that pieces can move in.

// The slide array indicates if the piece can slide (e.g. rook or bishop) or not (e.g. knight or king).
// If slide for the piece is false, it can only move one square in any one direction.
const bool slide[6] = {
	false, false, true, true, true, false
};
// The offsets array indicates the number of directions each piece can move in.
const int offsets[6] = {
	0, 8, 4, 4, 8, 8
};
// The offset array contains the actual offsets for each piece type.
const int offset[6][8] = {
	{ 0, 0, 0, 0, 0, 0, 0, 0 },
	{ -21, -19, -12, -8, 8, 12, 19, 21 },
	{ -11, -9, 9, 11, 0, 0, 0, 0 },
	{ -10, -1, 1, 10, 0, 0, 0, 0 },
	{ -11, -10, -9, -1, 1, 9, 10, 11 },
	{ -11, -10, -9, -1, 1, 9, 10, 11 }
};

// The castle_mask array is used it to determine the castling permissions after a move. 
// After a move, logical-AND the castle bits with the castle_mask bits for both of the move's squares.
// For example, if castle is 1 (meaning white can still castle kingside) then we play a move
// where the rook on h1 gets captured. We AND castle with castle_mask[63], so we have 1 & 14, 
// so castle becomes 0 and white can't castle kingside anymore.
const int castle_mask[64] = {
	 7, 15, 15, 15,  3, 15, 15, 11,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	13, 15, 15, 15, 12, 15, 15, 14
};

// Constants that define the piece letters, for print_board() */
const char piece_char[6] = {
	'P', 'N', 'B', 'R', 'Q', 'K'
};

const char white_piece_char[6] = {
	'P', 'N', 'B', 'R', 'Q', 'K'
};

const char black_piece_char[6] = {
	'p', 'n', 'b', 'r', 'q', 'k'
};

// Constants that define the initial board state
const int init_color[64] = {
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0
};

const int init_piece[64] = {
	3, 1, 2, 4, 5, 2, 1, 3,
	0, 0, 0, 0, 0, 0, 0, 0,
	6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6,
	0, 0, 0, 0, 0, 0, 0, 0,
	3, 1, 2, 4, 5, 2, 1, 3
};

// Structure Definitions

// The Move structure is the basic representation of a move.
// It contains the starting and ending positions of the move, followed by:
// promote which optionally specifies the promotion piece and
// bits which contains the following flags:
//  1 - 0x01 - Indicates if the move is a capture.
//  2 - 0x02 - Indicates if the move is a castling move.
//  4 - 0x04 - Indicates if the move is an en passant capture. a castling move.
//  8 - 0x08 - Indicates if the move is advancing a pawn two squares.
// 16 - 0x10 - Indicates if the move is a pawn move.
// 32 - 0x20 - Indicates if the move is a pawn promotion.
struct Move {
    int from, to; // from and to are the square indices of the move
    char promote; // promote is the piece type to promote to, if applicable
    char bits; // bits is a bitfield that contains flags for the move
};

// The BoardData structure is the basic representation of the board and associated game state.
// The 8x8 square-centric board representation consists of two 64 element arrays: 
// color indicates piece color per square: WHITE (0), BLACK (1), or EMPTY (6) and
// piece indicates piece type per square: PAWN (0), KNIGHT (1), BISHOP (2), ROOK (3), QUEEN (4), KING (5), or EMPTY (6).
//
// The following diagram shows how the array index numbers (0 to 63) map to squares (A1 to H8)
//
//  A8 B8 C8 D8 E8 F8 G8 H8
//  A7 B7 C7 D7 E7 F7 G7 H7
//  A6 B6 C6 D6 E6 F6 G6 H6
//  A5 B5 C5 D5 E5 F5 G5 H5
//  A4 B4 C4 D4 E4 F4 G4 H4
//  A3 B3 C3 D3 E3 F3 G3 H3
//  A2 B2 C2 D2 E2 F2 G2 H2
//  A1 B1 C1 D1 E1 F1 G1 H1
//
//   0  1  2  3  4  5  6  7
//   8  9 10 11 12 13 14 15
//  16 17 18 19 20 21 22 23
//  24 25 26 27 28 29 30 31
//  32 33 34 35 36 37 38 39
//  40 41 42 43 44 45 46 47
//  48 49 50 51 52 53 54 55
//  56 57 58 59 60 61 62 63
//
struct BoardData {
    // The 8x8 board representation consists of two 64 element arrays:
    int color[64]; // Piece color per square
    int piece[64]; // Piece type per square

    // The following fields are used to track the game state.
    bool whiteToMove; // The side to move. It is white's turn to move if whiteToMove is true.
    int castle; // A bitfield with the castle permissions for each side.
    // If 1 is set white can still castle kingside.
    // If 2 is set white can still castle queenside.
    // If 4 is set black can still castle kingside.
    // If 8 is set black can still castle queenside.
    // The en passant square is the square where a pawn can be captured en passant.
    int ep; // The en passant square after the last move on the 8x8 board.
    // For example, if white moves e2e4 the en passant square is set to square e3,
    // because that is where a black pawn would move in an en passant capture.
    int fifty; // The number of half-moves (ply) since the last capture or pawn move.
    // Used to handle the fifty-move-draw rule.
    int pos_hash; // A hash value that can be used as an index to the position in hash tables.
    int ply; // The number of half-moves (ply) since the root of the search tree.
    // This is used to determine the current position in the search tree.
    // ply = 0 at the root of the search tree.
    int hist_ply; // The number of ply since the beginning of the game (h for history).
};

struct Zobrist {
    uint64_t pieceHash[2][6][64]; // 2 colors x 6 piece types x 64 squares
    uint64_t whiteToMoveHash;

    Zobrist();
    uint64_t computeHash(const BoardData& board);
};

// Function Prototypes

BoardData getInitialBoard();
void parsePosition(const std::string& input, BoardData& board);
Move findBestMoveParallel(BoardData board, int depth, int timeLimitMs);
std::string moveToUci(const Move& m);
BoardData applyMove(BoardData board, Move m);
