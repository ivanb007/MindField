// uci.cpp (Update inside runUciLoop and searchPosition)
#include "uci.h"
#include "engine.h"
#include "threadpool.h"
#include <iostream>
#include <sstream>
#include <thread>
#include <atomic>
#include <chrono>

std::atomic<bool> stopSearch(false);
std::thread searchThread;

int timeLimitMs = 1000;  // default 1 second per move

void runUciLoop() {
    GameState state = getInitialState();
    std::string line;

    while (std::getline(std::cin, line)) {
        std::istringstream iss(line);
        std::string token;
        iss >> token;

        if (token == "uci") {
            std::cout << "id name ModularChessEngine\n";
            std::cout << "id author You\n";
            std::cout << "uciok\n";
        } else if (token == "isready") {
            std::cout << "readyok\n";
        } else if (token == "position") {
            parsePosition(line, state);
        } else if (token == "go") {
            stopSearch = false;
            timeLimitMs = 1000; // reset default

            std::string sub;
            while (iss >> sub) {
                if (sub == "movetime") {
                    iss >> timeLimitMs;
                } else if (sub == "wtime" || sub == "btime") {
                    int timeRemaining;
                    iss >> timeRemaining;
                    timeLimitMs = timeRemaining / 30; // rough allocation: 1/30th of time
                }
            }

            if (searchThread.joinable()) searchThread.join();
            searchThread = std::thread([state]() {
                auto start = std::chrono::steady_clock::now();
                Move bestMove = findBestMoveParallel(state, 4, timeLimitMs);
                auto end = std::chrono::steady_clock::now();

                if (!stopSearch.load()) {
                    std::cout << "bestmove " << moveToUci(bestMove) << "\n";
                    std::cout.flush();
                }
            });
        } else if (token == "stop") {
            stopSearch = true;
            if (searchThread.joinable()) searchThread.join();
        } else if (token == "quit") {
            stopSearch = true;
            if (searchThread.joinable()) searchThread.join();
            break;
        }
    }
}

// search.h (update signature)
Move findBestMoveParallel(GameState state, int depth, int timeLimitMs);

// search.cpp (update function)
Move findBestMoveParallel(GameState state, int depth, int timeLimitMs) {
    auto moves = generateMoves(state);
    ThreadPool pool(std::thread::hardware_concurrency());
    std::vector<std::future<int>> futures;

    std::atomic<bool> localStop(false);
    auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeLimitMs);

    for (const auto& m : moves) {
        GameState next = applyMove(state, m);
        futures.emplace_back(pool.enqueue([=, &localStop]() {
            return alphabetaTimed(next, depth - 1, -INF, INF, false, deadline, localStop);
        }));
    }

    int bestScore = -INF;
    Move bestMove = moves[0];
    for (size_t i = 0; i < moves.size(); ++i) {
        if (std::chrono::steady_clock::now() > deadline) {
            localStop = true;
            break;
        }
        int score = futures[i].get();
        if (score > bestScore) {
            bestScore = score;
            bestMove = moves[i];
        }
    }
    return bestMove;
}

int alphabetaTimed(GameState state, int depth, int alpha, int beta, bool maximizing, std::chrono::steady_clock::time_point deadline, std::atomic<bool>& stop) {
    if (stop.load() || std::chrono::steady_clock::now() > deadline) return 0;
    if (depth == 0) return evaluate(state);

    auto moves = generateMoves(state);
    if (moves.empty()) return evaluate(state);

    if (maximizing) {
        int maxEval = -INF;
        for (auto& m : moves) {
            GameState newState = applyMove(state, m);
            int eval = alphabetaTimed(newState, depth - 1, alpha, beta, false, deadline, stop);
            maxEval = std::max(maxEval, eval);
            alpha = std::max(alpha, eval);
            if (beta <= alpha) break;
        }
        return maxEval;
    } else {
        int minEval = INF;
        for (auto& m : moves) {
            GameState newState = applyMove(state, m);
            int eval = alphabetaTimed(newState, depth - 1, alpha, beta, true, deadline, stop);
            minEval = std::min(minEval, eval);
            beta = std::min(beta, eval);
            if (beta <= alpha) break;
        }
        return minEval;
    }
}
