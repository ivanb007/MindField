// search.h

#pragma once

#include "engine.h"
#include "threadpool.h"

#include <chrono>
#include <atomic>
#include <vector>
#include <future>


int evaluate(const BoardData& board);
std::vector<Move> generateMoves(const BoardData& board);
Move findBestMoveParallel(BoardData board, int depth, int timeLimitMs);
int alphabetaTimed(BoardData board, int depth, int alpha, int beta, bool maximizing, std::chrono::steady_clock::time_point deadline, std::atomic<bool>& stop);