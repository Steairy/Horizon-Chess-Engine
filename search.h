#pragma once
#include "NNUE.h"
#include "board.h"
#include <array>
#include <chrono>
#include <cstdint>
#include <algorithm>
#include <iostream>
#include <vector>

struct TTEntry {
    uint64_t hash = 0;
    uint8_t flag; // 0 is lower, 1 is exact, 2 is upper
    int eval;
    uint8_t depth;
    uint32_t move;
};

struct SearchInfo {
    long long nodeCount = 0;
    bool stopSearch = false;
    std::chrono::steady_clock::time_point startTime;
    double timeLimit = 0;
    uint32_t lastIterationBest = 0;
    int lastIterationEval = 0;
    uint8_t iterativeDepth = 1;
    std::array<std::array<uint32_t, 2>, 128> killerMoves;

    void reset(){
        nodeCount = 0;
        stopSearch = false;
        timeLimit = 0;
        lastIterationBest = 0;
        lastIterationEval = 0;
        killerMoves = {};
        iterativeDepth = 1;
    }
};

class Search {
public:
    NNUE nnue;
    uint32_t bestMove = 0;
    
    std::vector<TTEntry> TT;

    Search(){
        TT.resize(33554432);
    }

    int MATE_VALUE = 100000;

    SearchInfo info;
    std::array<int, 70> activatedArray;

    // Returns the NNUE input array for a given board
    int getActivatedArray(Board& board, std::array<int, 70>& out);

    // Alpha-beta search
    int alphabeta(Board& board, uint8_t depth, int alpha, int beta, bool root, int ply);

    // Iterative deepening search
    int iterative(Board& board, double limit);
};
