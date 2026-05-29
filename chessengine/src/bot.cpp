#include "chess.hpp"
#include "bot.hpp"
#include <algorithm>
#include <atomic>
#include <cfloat>
#include <chrono>
#include <cmath>

std::atomic<bool> stop(false);
std::atomic<int64_t> nodeCount(0);
int64_t benchNps = 500000; // conservative fallback

const double PAWNM = 1;
const double KNIGHTM = 3;
const double BISHOPM = 3;
const double ROOKM = 5;
const double QUEENM = 9;

const double PAWNP[64] = {
   0,  0,  0,  0,  0,  0,  0,  0,
   5, 10, 10,-20,-20, 10, 10,  5,
   5, -5,-10,  0,  0,-10, -5,  5,
   0,  0,  0, 20, 20,  0,  0,  0,
   5,  5, 10, 25, 25, 10,  5,  5,
  10, 10, 20, 30, 30, 20, 10, 10,
  50, 50, 50, 50, 50, 50, 50, 50,
   0,  0,  0,  0,  0,  0,  0,  0
};

const double KNIGHTP[64] = {
  -50,-40,-30,-30,-30,-30,-40,-50,
  -40,-20,  0,  0,  0,  0,-20,-40,
  -30,  0, 10, 15, 15, 10,  0,-30,
  -30,  5, 15, 20, 20, 15,  5,-30,
  -30,  0, 15, 20, 20, 15,  0,-30,
  -30,  5, 10, 15, 15, 10,  5,-30,
  -40,-20,  0,  5,  5,  0,-20,-40,
  -50,-40,-30,-30,-30,-30,-40,-50
};

const double BISHOPP[64] = {
  -20,-10,-10,-10,-10,-10,-10,-20,
  -10,  0,  0,  0,  0,  0,  0,-10,
  -10,  0,  5, 10, 10,  5,  0,-10,
  -10,  5,  5, 10, 10,  5,  5,-10,
  -10,  0, 10, 10, 10, 10,  0,-10,
  -10, 10, 10, 10, 10, 10, 10,-10,
  -10,  5,  0,  0,  0,  0,  5,-10,
  -20,-10,-10,-10,-10,-10,-10,-20
};

const double ROOKP[64] = {
   0,  0,  0,  0,  0,  0,  0,  0,
  -5, -5, -5, -5, -5, -5, -5, -5,
  -5, -5, -5, -5, -5, -5, -5, -5,
  -5, -5, -5, -5, -5, -5, -5, -5,
  -5, -5, -5, -5, -5, -5, -5, -5,
  -5, -5, -5, -5, -5, -5, -5, -5,
   5, 10, 10, 10, 10, 10, 10,  5,
   0,  0,  0,  5,  5,  0,  0,  0
};

const double QUEENP[64] = {
  -20,-10,-10, -5, -5,-10,-10,-20,
  -10,  0,  0,  0,  0,  0,  0,-10,
  -10,  0,  5,  5,  5,  5,  0,-10,
   -5,  0,  5,  5,  5,  5,  0, -5,
    0,  0,  5,  5,  5,  5,  0, -5,
  -10,  5,  5,  5,  5,  5,  0,-10,
  -10,  0,  5,  0,  0,  0,  0,-10,
  -20,-10,-10, -5, -5,-10,-10,-20
};

const double KINGP[64] = {
   20, 30, 10,  0,  0, 10, 30, 20,
   20, 20,  0,  0,  0,  0, 20, 20,
  -10,-20,-20,-20,-20,-20,-20,-10,
  -20,-30,-30,-40,-40,-30,-30,-20,
  -30,-40,-40,-50,-50,-40,-40,-30,
  -30,-40,-40,-50,-50,-40,-40,-30,
  -30,-40,-40,-50,-50,-40,-40,-30,
  -30,-40,-40,-50,-50,-40,-40,-30
};

inline int mirror(int sq) { return 56 - (sq / 8) * 8 + (sq % 8); }

const double POSITIONM = 0.01;

double eval(chess::Board& board, int movelen) {
  if (board.inCheck() && movelen==0) {
    if (board.sideToMove() == chess::Color::WHITE) {
      return -DBL_MAX;
    } else {
      return DBL_MAX;
    }
  } else if (movelen==0 || board.isRepetition(1)) {
    return 0.0;
  }
  double e = 0;
  for (int i = 0; i < 64; i++) {
    chess::Piece p = board.at(chess::Square(i));
    double m = (p.color() == chess::Color::WHITE) ? 1 : -1;
    int sq = (p.color() == chess::Color::WHITE) ? i : mirror(i);
    if (p.type() == chess::PieceType::NONE) continue;
    else if (p.type() == chess::PieceType::PAWN) e += m*(PAWNM+PAWNP[sq]*POSITIONM);
    else if (p.type() == chess::PieceType::KNIGHT) e += m*(KNIGHTM+KNIGHTP[sq]*POSITIONM);
    else if (p.type() == chess::PieceType::BISHOP) e += m*(BISHOPM+BISHOPP[sq]*POSITIONM);
    else if (p.type() == chess::PieceType::ROOK) e += m*(ROOKM+ROOKP[sq]*POSITIONM);
    else if (p.type() == chess::PieceType::QUEEN) e += m*(QUEENM+QUEENP[sq]*POSITIONM);
  }
  return e;
}

double minimax(chess::Board& board, double alpha, double beta, int s, int d) {
  nodeCount++;
  chess::Movelist moves;
  chess::movegen::legalmoves(moves, board);
  if (stop) return eval(board, moves.size());

  if (d == 0 || moves.size() == 0) {
    return eval(board, moves.size());
  }
  moveorder(moves, board);

  double best = (s == 0) ? -DBL_MAX : DBL_MAX;
  for (chess::Move m : moves) {
    board.makeMove(m);
    double e = minimax(board, alpha, beta, !s, d - 1);
    board.unmakeMove(m);
    if (!s) {
      best = (e > best) ? e : best;
      alpha = (alpha > e) ? alpha : e;
      if (beta <= alpha) break;
    } else {
      best = (e < best) ? e : best;
      beta = (beta < e) ? beta : e;
      if (beta <= alpha) break;
    }
  }
  return best;
}

int moveinterest(chess::Move& m, const chess::Board& board) {
  int i=0;
  if (board.isCapture(m)) i++;
  if (board.givesCheck(m) != chess::CheckType::NO_CHECK) i++;
  if (m.typeOf() == chess::Move::PROMOTION) i++;
  return i;
}

void moveorder(chess::Movelist& moves, chess::Board& board) {
  std::ranges::sort(moves, [&board](chess::Move& a, chess::Move& b) {
    return moveinterest(a, board) > moveinterest(b, board);
  });
}

// Estimate tree size: rootMoves * branchFactor^(depth-1)
// With alpha-beta, effective branch factor ~sqrt(b), we use 6 as a rough middle ground.
static int pickDepth(int rootMoves, int64_t nps, int thinkMs) {
  if (thinkMs <= 0) return 4;
  int64_t budget = (int64_t)nps * thinkMs / 1000;
  const double bf = 16.0; // effective branching factor estimate
  for (int d = 6; d >= 1; d--) {
    int64_t est = (int64_t)(rootMoves * std::pow(bf, d - 1));
    if (est <= budget) return d;
  }
  return 1;
}

void runBenchmark() {
  chess::Board b;
  b.setFen(chess::constants::STARTPOS);
  std::atomic<bool> dummy(false);
  // swap in dummy stop so benchmark doesn't get interrupted
  bool saved = stop.load();
  stop = false;
  nodeCount = 0;

  auto t0 = std::chrono::steady_clock::now();
  chess::Movelist moves;
  chess::movegen::legalmoves(moves, b);
  for (chess::Move m : moves) {
    b.makeMove(m);
    minimax(b, -DBL_MAX, DBL_MAX, 1, 3);
    b.unmakeMove(m);
  }
  auto t1 = std::chrono::steady_clock::now();

  int64_t ns = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();
  int64_t n  = nodeCount.load();
  benchNps = (ns > 0) ? (n * 1000000000LL / ns) : 500000;

  stop = saved;
  nodeCount = 0;
  std::cout << "info bench nps " << benchNps << std::endl;
}

chess::Move searchmove(chess::Board& board, int s, int thinkMs) {
  nodeCount = 0;
  chess::Movelist moves;
  chess::movegen::legalmoves(moves, board);
  moveorder(moves, board);

  if (moves.size() == 0) return chess::Move::NULL_MOVE;

  int depth = pickDepth((int)moves.size(), benchNps, thinkMs);
  std::cerr << "info depth " << depth << " rootmoves " << moves.size() << std::endl;

  double w = (s == 0) ? -DBL_MAX : DBL_MAX;
  chess::Move best = moves[0];
  for (chess::Move m : moves) {
    if (stop) break;

    board.makeMove(m);
    double e = minimax(board, -DBL_MAX, DBL_MAX, !s, depth - 1);
    board.unmakeMove(m);

    if (s == 0 && e > w) { w = e; best = m; }
    else if (s == 1 && e < w) { w = e; best = m; }
  }
  std::cout << "info nodes " << nodeCount.load() << " score cp " << w << " depth " << depth << std::endl;
  return best;
}
