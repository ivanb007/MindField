// uci.cpp

#include "uci.h"
#include "engine.h"
#include "search.h"
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
