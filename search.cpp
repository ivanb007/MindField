// search.cpp

#include "search.h"
#include "threadpool.h"
#include <limits>

const int INF = std::numeric_limits<int>::max();

int evaluate(const GameState& state) {
    int score = 0;
    for (char c : state.board) {
        if (c == '.') continue;
        int val;
        switch (tolower(c)) {
            case 'p': val = 10; break;
            case 'n': case 'b': val = 30; break;
            case 'r': val = 50; break;
            case 'q': val = 90; break;
            case 'k': val = 900; break;
            default: val = 0;
        }
        score += isupper(c) ? val : -val;
    }
    return score;
}

GameState applyMove(GameState state, Move m) {
    state.board[m.to] = state.board[m.from];
    state.board[m.from] = '.';
    state.whiteToMove = !state.whiteToMove;
    return state;
}

std::vector<Move> generateMoves(const GameState& state) {
    // Placeholder for real move generation
    std::vector<Move> moves;
    for (int i = 0; i < 64; ++i) {
        if ((state.whiteToMove && isupper(state.board[i])) ||
            (!state.whiteToMove && islower(state.board[i]))) {
            int row = i / 8, col = i % 8;
            if (col + 1 < 8) moves.push_back({i, i + 1});
        }
    }
    return moves;
}

Move findBestMoveParallel(GameState state, int depth, int timeLimitMs) {
    auto moves = generateMoves(state);
    if (moves.empty()) return {0, 0}; // No moves available
    if (moves.size() == 1) return moves[0]; // Only one move available return it
    if (depth < 1) depth = 1; // Ensure depth is at least 1
    if (timeLimitMs < 100) timeLimitMs = 100; // Ensure time limit is at least 100ms

    if (depth == 1) {
        // If depth is 1, just return the best move based on evaluation
        int bestScore = -INF;
        Move bestMove = moves[0];
        for (const auto& m : moves) {
            GameState nextState = applyMove(state, m);
            int score = evaluate(nextState);
            if (score > bestScore) {
                bestScore = score;
                bestMove = m;
            }
        }
        return bestMove;
    }

    // Create a thread pool with the number of threads equal to the number of available cores.
    ThreadPool pool(std::thread::hardware_concurrency());
    // Vector to hold futures for the results of each task.
    std::vector<std::future<int>> futures;
    // Atomic flag to stop the search if time limit is reached.
    std::atomic<bool> localStop(false);
    // Set the deadline for the search based on the time limit.
    auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeLimitMs);

    // Enqueue tasks for each move and collect futures.
    // This allows us to run the alphabeta search in parallel for each move.
    for (const auto& m : moves) {
        GameState next = applyMove(state, m);
        auto localStopPtr = &localStop;
        futures.emplace_back(pool.enqueue([=]() {
            return alphabetaTimed(next, depth - 1, -INF, INF, false, deadline, *localStopPtr);
        }));
    }

    int bestScore = -INF;
    Move bestMove = moves[0];
    // Wait for each future to complete and determine the best move.
    for (size_t i = 0; i < moves.size(); ++i) {
        // If the time limit is reached, we stop processing further.
        if (std::chrono::steady_clock::now() > deadline) {
            localStop = true;
            break;
        }
        // Get the result of the future.
        // This will block until the task is complete.
        // If the task was stopped due to time limit, it will return 0.
        // Otherwise, it will return the evaluation score for the move.
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
