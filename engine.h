// engine.h

#pragma once
#include <string>
#include <vector>

struct Move {
    int from, to;
};

struct GameState {
    std::string board;
    bool whiteToMove;
};

GameState getInitialState();
void parsePosition(const std::string& input, GameState& state);
Move findBestMoveParallel(GameState state, int depth);
std::string moveToUci(const Move& m);
