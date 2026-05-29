#include <bits/stdc++.h>
#include <sstream>
#include <string>
#include <atomic>
#include <thread>
#include <chrono>
using namespace std;

#include "chess.hpp"
using namespace chess;

#include "bot.hpp"
extern std::atomic<bool> stop;

void setBoard(const string& line, Board& board) {
  istringstream ss(line);
  string token;
  ss >> token;

  if (token == "startpos") {
    board.setFen(constants::STARTPOS);
    ss >> token;
  } else if (token == "fen") {
    string fen;
    while (ss >> token && token != "moves") {
      fen += token + " ";
    }
    if (!fen.empty()) fen.pop_back();
    board.setFen(fen);
  }

  if (token == "moves") {
    string move;
    while (ss >> move) {
      Move mv = uci::uciToMove(board, move);
      board.makeMove(mv);
    }
  }
}

void parseGo(const std::string& line, Board& board, int& thinkMs) {
  std::istringstream ss(line);
  std::string token;
  ss >> token; // consume "go"

  thinkMs = 1000;

  while (ss >> token) {
    if (token == "movetime") {
      ss >> thinkMs;
    } else if (token == "infinite") {
      thinkMs = INT_MAX;
    } else if (token == "wtime" && board.sideToMove() == Color::WHITE) {
      int t; ss >> t;
      thinkMs = t / 20;
    } else if (token == "btime" && board.sideToMove() == Color::BLACK) {
      int t; ss >> t;
      thinkMs = t / 20;
    }
  }

  stop = false;
}

int main() {
  string command;
  Board board;

  while (true) {
    if (!getline(cin, command)) break;
    if (command.empty()) continue;

    istringstream iss(command);
    iss >> command;

    if (command == "isready") {
      runBenchmark();
      cout << "readyok" << endl;
      cout.flush();

    } else if (command == "quit") {
      return 0;

    } else if (command == "uci") {
      cout << "id name RottenOnion" << endl;
      cout << "id author Jokubas Raginis" << endl;
      cout << "uciok" << endl;
      cout.flush();

    } else if (command == "ucinewgame") {
      board.setFen(constants::STARTPOS);

    } else if (command == "position") {
      string remainder;
      getline(iss, remainder);
      size_t start = remainder.find_first_not_of(" \t");
      if (start != string::npos) {
        setBoard(remainder.substr(start), board);
      }

    } else if (command == "go") {
      string remainder;
      getline(iss, remainder);
      int thinkMs;
      parseGo("go" + remainder, board, thinkMs);

      stop = false;
      int s = (board.sideToMove() == Color::WHITE) ? 0 : 1;

      chess::Move result = chess::Move::NULL_MOVE;
      std::thread searchThread([&]() {
        result = searchmove(board, s, thinkMs);
      });

      if (thinkMs != INT_MAX) {
        std::this_thread::sleep_for(std::chrono::milliseconds(thinkMs));
        stop = true;
      }

      searchThread.join();

      if (result != chess::Move::NULL_MOVE) {
        cout << "bestmove " << uci::moveToUci(result) << endl;
        board.makeMove(result);
      } else {
        cout << "bestmove (none)" << endl;
      }
      cout.flush();
    }
  }

  return 0;
}
