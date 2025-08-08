// search.h

#pragma once

#include "engine.h"
#include "threadpool.h"

#include <chrono>
#include <atomic>
#include <vector>
#include <future>


int evaluate(const GameState& state);
GameState applyMove(GameState state, Move m);
std::vector<Move> generateMoves(const GameState& state);
Move findBestMoveParallel(GameState state, int depth, int timeLimitMs);
int alphabetaTimed(GameState state, int depth, int alpha, int beta, bool maximizing, std::chrono::steady_clock::time_point deadline, std::atomic<bool>& stop);