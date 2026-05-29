#pragma once
#include "chess.hpp"
#include <atomic>
#include <cstdint>

extern std::atomic<bool> stop;
extern std::atomic<int64_t> nodeCount;
extern int64_t benchNps; // nodes/sec measured at isready

void moveorder(chess::Movelist& moves, chess::Board& board);
double eval(chess::Board& board, int movelen);
double minimax(chess::Board& board, double alpha, double beta, int side, int depth);
chess::Move searchmove(chess::Board& board, int side, int thinkMs);
void runBenchmark();
