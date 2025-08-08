// engine.cpp

#include "engine.h"
#include "search.h"
#include <sstream>

GameState getInitialState() {
    return {"rnbqkbnrpppppppp................................PPPPPPPPRNBQKBNR", true};
}

void parsePosition(const std::string& input, GameState& state) {
    std::istringstream iss(input);
    std::string token;
    iss >> token >> token; // skip "position startpos"
    state = getInitialState();
    if (iss >> token && token == "moves") {
        while (iss >> token) {
            int from = (8 - (token[1] - '0')) * 8 + (token[0] - 'a');
            int to = (8 - (token[3] - '0')) * 8 + (token[2] - 'a');
            state = applyMove(state, {from, to});
        }
    }
}

std::string moveToUci(const Move& m) {
    auto idxToSquare = [](int idx) {
        int row = idx / 8;
        int col = idx % 8;
        std::string square;
        square += 'a' + col;
        square += '8' - row;
        return square;
    };
    return idxToSquare(m.from) + idxToSquare(m.to);
}
