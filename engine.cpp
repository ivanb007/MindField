// engine.cpp

#include "engine.h"
#include "search.h"
#include <sstream>
#include <unordered_map>
#include <chrono>

// Function to get the initial game state.
// This sets up the board with the standard starting position and indicates it's white's turn to move
BoardData getInitialBoard() {
    BoardData board;
    for (int i = 0; i < 64; ++i) {
        // Initialize color and piece arrays with predefined values}
        board.color[i] = init_color[i];
        board.piece[i] = init_piece[i];
    }
    board.whiteToMove = true;
    return board;
}

BoardData applyMove(BoardData board, Move m) {
    int fromIdx = m.from;
    int toIdx = m.to;
    board.piece[toIdx] = board.piece[fromIdx];
    board.piece[fromIdx] = EMPTY;
    // No promotion, castling or en passant logic implemented here.
    // Change the side to move after applying the move.
    board.whiteToMove = !board.whiteToMove;
    return board;
}

// Function to parse the position command from UCI input.
// It updates the game state based on the provided position string.
void parsePosition(const std::string& input, BoardData& board) {
    Move m;
    // Split the input string to extract the position command
    // The input format is expected to be "position startpos moves e2e4 e7e5"
    // where "startpos" indicates the initial position and "moves" lists the moves to apply.
    std::istringstream iss(input);
    std::string token;
    iss >> token >> token;
    board = getInitialBoard();
    if (iss >> token && token == "moves") {
        while (iss >> token) {
            // The from Row = 8 - (token[1] - '0') and the from Col = token[0] - 'a'
            m.from = (8 - (token[1] - '0')) * 8 + (token[0] - 'a'); // Convert to square index
            // The to Row = (8 - (token[3] - '0') and the to Col = token[2] - 'a';
            m.to = (8 - (token[3] - '0')) * 8 + (token[2] - 'a'); // Convert to square index
            m.promote = 0; // No promotion
            m.bits = 0; // No special bits
            // Apply the move to the board
            board = applyMove(board, m);
        }
    }
}

// Function to convert a Move to UCI format.
// This converts the move from internal representation to a string format used in UCI.
// The move is represented as "from_square to_square", e.g., "e2e4".
std::string moveToUci(const Move& m) {
    std::string uci;
    uci += 'a' + COL(m.from);
    uci += '8' - ROW(m.from);
    uci += 'a' + COL(m.to);
    uci += '8' - ROW(m.to);
    return uci;
}

Zobrist::Zobrist() {
    std::mt19937_64 rng(std::chrono::steady_clock::now().time_since_epoch().count());
    for (int i = 0; i < 2; ++i)
        for (int j = 0; j < 6; ++j)
            for (int k = 0; k < 64; ++k)
                pieceHash[i][j][k] = rng();
    // Initialize the hash for the white to move state
    whiteToMoveHash = rng();
}

uint64_t Zobrist::computeHash(const BoardData& board) {
    uint64_t h = 0;
    for (int sq = 0; sq < 64; ++sq) {
        int idx = board.color[sq];
        int idy = board.piece[sq];
        h ^= pieceHash[idx][idy][sq];
    }
    // Include the side to move in the hash
    if (board.whiteToMove)
        h ^= whiteToMoveHash;
    // Include the castle permissions in the hash
    h ^= (board.castle & 0xF); // Use only the lower 4 bits for castle permissions
    // Include the fifty-move rule in the hash
    h ^= (board.fifty & 0xFF); // Use only the lower 8 bits for fifty-move rule
    // Include the en passant square in the hash
    if (board.ep >= 0 && board.ep < 64) {
        h ^= pieceHash[board.whiteToMove ? WHITE : BLACK][PAWN][board.ep];
    }
    return h;
}