// search.cpp

#include "search.h"
#include "threadpool.h"

#include <limits>

const int INF = std::numeric_limits<int>::max();

std::vector<Move> generateMoves(const BoardData& board) {
    std::vector<Move> moves;
    int i, j, n;
    int side;
    if (board.whiteToMove) side = WHITE;
    else side = BLACK;
    int xside = side == WHITE ? BLACK : WHITE;

    for (i = 0; i < 64; ++i)
		if (board.color[i] == side) {
			if (board.piece[i] == PAWN) {
				if (side == WHITE) {
					if (COL(i) != 0 && board.color[i - 9] == BLACK)
						moves.push_back({i, i - 9, 0, 17});
					if (COL(i) != 7 && board.color[i - 7] == BLACK)
						moves.push_back({i, i - 7, 0, 17});
					if (board.color[i - 8] == EMPTY) {
						moves.push_back({i, i - 8, 0, 16});
						if (i >= 48 && board.color[i - 16] == EMPTY)
							moves.push_back({i, i - 16, 0, 24});
					}
				}
				else {
					if (COL(i) != 0 && board.color[i + 7] == WHITE)
						moves.push_back({i, i + 7, 0, 17});
					if (COL(i) != 7 && board.color[i + 9] == WHITE)
						moves.push_back({i, i + 9, 0, 17});
					if (board.color[i + 8] == EMPTY) {
						moves.push_back({i, i + 8, 0, 16});
						if (i <= 15 && board.color[i + 16] == EMPTY)
							moves.push_back({i, i + 16, 0, 24});
					}
				}
			}
			else
				for (j = 0; j < offsets[board.piece[i]]; ++j)
					for (n = i;;) {
						n = mailbox[mailbox64[n] + offset[board.piece[i]][j]];
						if (n == -1)
							break;
						if (board.color[n] != EMPTY) {
							if (board.color[n] == xside)
								moves.push_back({i, n, 0, 1});
							break;
						}
						if (!slide[board.piece[i]])
							break;
					}
		}
	if (board.ep != -1) {
        int ep = board.ep;
		if (side == WHITE) {
			if (COL(ep) != 0 && board.color[ep + 7] == WHITE && board.piece[ep + 7] == PAWN)
				moves.push_back({ep + 7, ep, 0, 21});
			if (COL(ep) != 7 && board.color[ep + 9] == WHITE && board.piece[ep + 9] == PAWN)
				moves.push_back({ep + 9, ep, 0, 21});
		}
		else {
			if (COL(ep) != 0 && board.color[ep - 9] == BLACK && board.piece[ep - 9] == PAWN)
				moves.push_back({ep - 9, ep, 0, 21});
			if (COL(ep) != 7 && board.color[ep - 7] == BLACK && board.piece[ep - 7] == PAWN)
				moves.push_back({ep - 7, ep, 0, 21});
		}
	}
    return moves;
}

Move findBestMoveParallel(BoardData board, int depth, int timeLimitMs) {
    auto moves = generateMoves(board);
    if (moves.empty()) return {0, 0, 0, 0}; // No moves available
    if (moves.size() == 1) return moves[0]; // Only one move available return it
    if (depth < 1) depth = 1; // Ensure depth is at least 1
    if (timeLimitMs < 100) timeLimitMs = 100; // Ensure time limit is at least 100ms

    if (depth == 1) {
        // If depth is 1, just return the best move based on evaluation
        int bestScore = -INF;
        Move bestMove = moves[0];
        for (const auto& m : moves) {
            BoardData nextBoard = applyMove(board, m);
            int score = evaluate(nextBoard);
            if (score > bestScore) {
                bestScore = score;
                bestMove = m;
            }
        }
        return bestMove;
    }

    // Create a thread pool with the number of threads equal to the number of available cores.
    ThreadPool pool(std::thread::hardware_concurrency());
    // Create vector to hold futures for the results of each task.
    std::vector<std::future<int>> futures;

    // Create atomic flag to stop the search if time limit is reached.
    std::atomic<bool> localStop(false);
    // Set the deadline for the search based on the time limit.
    auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeLimitMs);

    // Enqueue tasks for each move and collect futures.
    // This allows us to run the alphabeta search in parallel for each move.
    for (const auto& m : moves) {
        BoardData nextBoard = applyMove(board, m);
        futures.emplace_back(pool.enqueue([=, &localStop]() {
            return alphabetaTimed(nextBoard, depth - 1, -INF, INF, false, deadline, localStop);
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

int alphabetaTimed(BoardData board, int depth, int alpha, int beta, bool maximizing, std::chrono::steady_clock::time_point deadline, std::atomic<bool>& stop) {
    if (stop.load() || std::chrono::steady_clock::now() > deadline) return 0;
    if (depth == 0) return evaluate(board);

    auto moves = generateMoves(board);
    if (moves.empty()) return evaluate(board);

    if (maximizing) {
        int maxEval = -INF;
        for (auto& m : moves) {
            BoardData newBoard = applyMove(board, m);
            int eval = alphabetaTimed(newBoard, depth - 1, alpha, beta, false, deadline, stop);
            maxEval = std::max(maxEval, eval);
            alpha = std::max(alpha, eval);
            if (beta <= alpha) break;
        }
        return maxEval;
    } else {
        int minEval = INF;
        for (auto& m : moves) {
            BoardData newBoard = applyMove(board, m);
            int eval = alphabetaTimed(newBoard, depth - 1, alpha, beta, true, deadline, stop);
            minEval = std::min(minEval, eval);
            beta = std::min(beta, eval);
            if (beta <= alpha) break;
        }
        return minEval;
    }
}
