#include "search.h"

int Search::getActivatedArray(Board& board, std::array<int, 70>& out){
    int count = 0;
    for(int i = 0; i < 12; i++){
        uint64_t bb = board.bitboards[i];
        int add = board.turn == board.BLACK ? 6 : 0;
        int pieceID = i + (i > 5 ? -add : add);
        while(bb){
            int ind = __builtin_ctzll(bb);
            if(board.turn == board.BLACK){
                ind ^= 56;
            }
            out[count++] = (ind*12)+pieceID;
            bb &= bb-1;
        }
    }

    int add = board.turn == board.WHITE ? 0 : 2;
    if(board.castlingRights[0+add]) out[count++] = 768;
    if(board.castlingRights[1+add]) out[count++] = 769;
    if(board.castlingRights[2-add]) out[count++] = 770;
    if(board.castlingRights[3-add]) out[count++] = 771;
    
    return count;
}

int Search::alphabeta(Board& board, uint8_t depth, int alpha, int beta, bool root, int ply){
    if(info.nodeCount % 512 == 0){
        auto now = std::chrono::steady_clock::now();
        double elapsed = std::chrono::duration<double, std::milli>(now - info.startTime).count();
        if(elapsed > info.timeLimit) info.stopSearch = true;
    }

    if(info.stopSearch){
        return 0;
    }

    bool quiescence = false;
    info.nodeCount++;

    int TTHash = board.zobrist.hash & 0x1FFFFFF;
    TTEntry currentT = TT[TTHash];
    bool ttMatch = board.zobrist.hash == currentT.hash;

    if(ttMatch && currentT.depth >= depth && !root){
        if((currentT.flag == 0 && currentT.eval >= beta) || (currentT.flag == 2 && currentT.eval < beta) || (currentT.flag == 1)){
            return currentT.eval;
        }
    }

    if(board.gameOver){
        if(board.outcome == 0){
            return 0;
        }
        else{
            return -MATE_VALUE + ply; // Priorotizes getting mated late and mating early
        }
    }

    int v = getActivatedArray(board, activatedArray);
    int staticEval = nnue.forward(activatedArray, v);

    // Reverse Futility Pruning
    bool inCheck = board.squareAttacked(__builtin_ctzll(board.bitboards[5+(6*board.turn)]), 1-board.turn);
    if(depth > 0 && depth <= 3 && !inCheck && !root){
        int margin = 120 * depth;

        if(staticEval-margin > beta){
            return staticEval-margin;
        }
    }

    // Null Move Pruning
    int staticAdd = 6*board.turn;
    bool notEndgame = (board.bitboards[4+staticAdd] | board.bitboards[3+staticAdd]) || (__builtin_popcountll(board.bitboards[1+staticAdd] | board.bitboards[2+staticAdd]) > 1);
    if(notEndgame && !inCheck && !root && staticEval >= beta && depth>=2){
        int r = 2+ depth/4;

        int prevEnPassant = board.enPassantSquare;
        // Make null move
        board.turn = !board.turn;
        board.zobrist.hash ^= board.zobrist.boardStateTable[0];
        board.enPassantSquare = 0;
        if(prevEnPassant != 0){
            board.zobrist.hash ^= board.zobrist.enPassantTable[prevEnPassant];
        }

        int v = -alphabeta(board, std::max(0, depth-1-r), -beta, -(beta-1), false, ply+1);

        // Unmake null move
        board.turn = !board.turn;
        board.zobrist.hash ^= board.zobrist.boardStateTable[0];
        if(prevEnPassant != 0){
            board.enPassantSquare = prevEnPassant;
            board.zobrist.hash ^= board.zobrist.enPassantTable[prevEnPassant];
        }

        if(v >= beta){
            return v;
        }
    }

    if(depth == 0){
        if(staticEval >= beta){
            return staticEval;
        }
        board.generateLegal(true);
        if(board.legalMoveCount == 0){
            return staticEval;
        }
        alpha = std::max(alpha, staticEval);
        depth = 1;
        quiescence = true;
    }

    if(!quiescence) board.generateLegal();
    std::array<uint32_t, 218> legal = board.legalMoves;
    int count = board.legalMoveCount;

    uint32_t bestMoveApproximation = ttMatch ? currentT.move : 0;
    auto moveOrderingLambda = [&](uint32_t m1, uint32_t m2){ // for move ordering
        auto getScore = [&](uint32_t m) {
            if (m == bestMoveApproximation) return 1000000; // Best guess from TT
            
            bool isCapture = (m >> 16) & 0x1;
            if (isCapture) {
                int moving = ((m >> 12) & 0xF) % 6;
                int captured = ((m >> 17) & 0xF) % 6;
                return 900000 + ((captured * 3)- moving); // MVV-LVA
            }

            // Killer move
            if (m == info.killerMoves[ply][0]) return 800000;
            if (m == info.killerMoves[ply][1]) return 700000;

            return 0; // Normal move
        };

        return getScore(m1) > getScore(m2);
    };


    std::sort(legal.begin(), legal.begin() + count, moveOrderingLambda);

    int originalAlpha = alpha;
    uint32_t currentBest = 0;

    int eval = -MATE_VALUE;
    for(int i = 0; i < count; i++){
        if(info.stopSearch){
            break;
        }

        uint32_t move = legal[i];
        bool isCapture = (move >> 16) & 0x1;

        board.makeMove(move, true);

        // Late move reductions
        int reduction = ((i >= 5) && (depth >= 3) && !isCapture) ? 1 + (depth >= 6) : 0;
        
        int score;
        score = -alphabeta(board, depth-1-reduction, -beta, -alpha, false, ply+1);
        if(score > alpha && reduction){
            score = -alphabeta(board, depth-1, -beta, -alpha, false, ply+1);
        }
        
        board.unmakeMove();
        
        if(score > eval){
            eval = score;
            alpha = std::max(score, alpha);
            currentBest = move;
        }

        if(beta <= alpha){
            //put it into killer moves if not capture and not already the primary killer
            if(!isCapture && move != info.killerMoves[ply][0]){
                info.killerMoves[ply][1] = info.killerMoves[ply][0];
                info.killerMoves[ply][0] = move;
            }
            break;
        }
    }


    TTEntry t = {.hash = board.zobrist.hash, .flag = 0, .eval = eval, .depth = depth, .move = currentBest};
    if(originalAlpha >= eval){
        t.flag = 2;
    }

    else if(eval >= beta){
        t.flag = 0;
    }

    else {
        t.flag = 1;
    }

    if(!info.stopSearch){
        TT[TTHash] = t;
    }
    
    if(root) bestMove = currentBest;
    return eval;
}

int Search::iterative(Board& board, double limit){
    bestMove = 0;
    info.reset();
    info.startTime = std::chrono::steady_clock::now();
    info.timeLimit = limit;

    while(true){
        if(info.stopSearch) break;
        if(info.iterativeDepth > 1 && ((info.lastIterationEval >= MATE_VALUE) || (info.lastIterationEval <= -MATE_VALUE))) break; // no need to continue search if it is guaranteed mate

        int evaluation = alphabeta(board, info.iterativeDepth);
        info.iterativeDepth++;
        if(!info.stopSearch){
            info.lastIterationEval = evaluation;
            info.lastIterationBest = bestMove;
        }
    }

    //std::cout << (int)info.iterativeDepth-1 << "\n";

    bestMove = info.lastIterationBest;
    return info.lastIterationEval;
}