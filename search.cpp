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

int Search::alphabeta(Board& board, uint8_t depth, int alpha, int beta, bool root){
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
            return -MATE_VALUE - depth; // Depth will be lower if it is deeper, so this priorotizes getting mated late and mating early
        }
    }

    if(depth == 0){
        int v = getActivatedArray(board, activatedArray);
        int ev = nnue.forward(activatedArray, v);
        if(ev >= beta){
            return ev;
        }
        board.generateLegal(true);
        if(board.legalMoveCount == 0){
            return ev;
        }
        alpha = std::max(alpha, ev);
        depth = 1;
        quiescence = true;
    }

    if(!quiescence) board.generateLegal();
    std::array<uint32_t, 218> legal = board.legalMoves;
    int count = board.legalMoveCount;

    uint32_t bestMoveApproximation = ttMatch ? currentT.move : 0;
    auto moveOrderingLambda = [&](uint32_t m1, uint32_t m2){ // for move ordering
        int score1 = 0, score2 = 0;

        if(m1 == bestMoveApproximation) score1 = 100;
        if(m2 == bestMoveApproximation) score2 = 100;

        // Captures
        if((m1 >> 16)&0x1){
            int movingPiece = (m1 >> 12)&0xF;
            int capturedPiece = (m1 >> 17)&0xF;
            score1 = std::max(score1, (capturedPiece%6)-(movingPiece%6));
        }
        if((m2 >> 16)&0x1){ 
            int movingPiece = (m2 >> 12)&0xF;
            int capturedPiece = (m2 >> 17)&0xF;
            score2 = std::max(score2, (capturedPiece%6)-(movingPiece%6));
        }

        return score1 > score2;
    };


    std::sort(legal.begin(), legal.begin() + count, moveOrderingLambda);

    int originalAlpha = alpha;
    uint32_t currentBest = 0;

    int eval = -10000000;
    for(int i = 0; i < count; i++){
        if(info.nodeCount % 512 == 0){
            auto now = std::chrono::steady_clock::now();
            double elapsed = std::chrono::duration<double, std::milli>(now - info.startTime).count();
            if(elapsed > info.timeLimit) info.stopSearch = true;
        }
        
        if(info.stopSearch){
            break;
        }

        board.makeMove(legal[i], true);

        //Late move reductions
        int reduction = ((i >= 4) && (depth >= 3) && !(legal[i] >> 16)&0x1) ? 1 + (depth >= 6) : 0; // no reductions if capture

        int score;
        score = -alphabeta(board, depth-1-reduction, -beta, -alpha, false);
        if(score > alpha && reduction){
            score = -alphabeta(board, depth-1, -beta, -alpha, false);
        }
        
        board.unmakeMove();
        
        if(score > eval){
            eval = score;
            alpha = std::max(score, alpha);
            currentBest = legal[i];
        }

        if(beta <= alpha){
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
    info.stopSearch = false;
    info.nodeCount = 0;
    info.timeLimit = limit;

    uint32_t lastIterationBest;
    int lastIterationEval;

    int evaluation;
    uint8_t currentDepth = 1;

    info.startTime = std::chrono::steady_clock::now();
    while(true){
        if(info.stopSearch) break;
        if(currentDepth > 1 && ((lastIterationEval >= MATE_VALUE) || (lastIterationEval <= -MATE_VALUE))) break; // no need to continue search if it is guaranteed mate

        evaluation = alphabeta(board, currentDepth);
        currentDepth++;
        if(!info.stopSearch){
            lastIterationEval = evaluation;
            lastIterationBest = bestMove;
        }
    }

    //std::cout << (int)currentDepth-1 << "\n";

    bestMove = lastIterationBest;
    return lastIterationEval;
}
