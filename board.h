#ifndef BOARD_H
#define BOARD_H

#include <bits/stdc++.h>

struct Zobrist {
    uint64_t hash;
    std::array<std::array<uint64_t, 12>, 64> hashTable;
    std::array<uint64_t, 64> enPassantTable;
    std::array<uint64_t, 5>  boardStateTable;
};

class Board {
public:
    enum indexModes {WHITE=0, BLACK=1, FRIENDLY=2, ENEMY=3};
    enum pieceTypes {Pawn = 0, Knight = 1, Bishop = 2, Rook = 3, Queen = 4, King = 5};

    std::array<uint64_t, 12> bitboards = {
        0x000000000000FF00ULL, 0x0000000000000042ULL, 0x0000000000000024ULL,
        0x0000000000000081ULL, 0x0000000000000008ULL, 0x0000000000000010ULL,
        0x00FF000000000000ULL, 0x4200000000000000ULL, 0x2400000000000000ULL,
        0x8100000000000000ULL, 0x0800000000000000ULL, 0x1000000000000000ULL
    };

    std::array<uint32_t, 5900> moves;
    std::array<uint32_t, 218> legalMoves;
    std::array<uint16_t, 5900> stateHistory;
    std::array<uint64_t, 5900> hashHistory;

    Zobrist zobrist;

    std::array<uint64_t, 64> pinMasks;

    int totalMoves = 0;
    std::array<bool, 4> castlingRights = { true, true, true, true };
    int halfMoveClock = 0;
    int enPassantSquare = 0;
    bool turn = false;

    bool gameOver = false;
    int outcome = 0;
    int legalMoveCount = 0;

    Board();

    void importFEN(std::string fen);

    bool checkBit(uint64_t bitboard, int bit);
    uint64_t getAllPieces();
    uint64_t getFriendlyPieces();
    uint64_t getEnemyPieces();
    int pieceAtPosition(int square);

    void initZobrist();

    uint32_t encodeMove(int startSquare, int endSquare, int movingPiece, bool isCapture,
                        int capturedPiece, bool isEnPassant, bool isCastling,
                        bool castleSide, bool isPromotion, int promotedPiece);

    uint16_t packState();
    void unpackState(uint16_t packedState);

    bool squareAttacked(int square, bool attacker);

    void removeBit(int bitboardIndex, int bit);

    void generatePinMasks();
    bool leavesInCheck(uint32_t move);
    void generateLegal(bool capturesOnly=false);
    void generatePseudoLegal(int piece, int square, std::array<uint32_t, 218>& movesVector, int& currentMove, bool capturesOnly=false);

    uint64_t generateWhitePawn(int square, std::array<uint32_t, 218>& movesVector, int& currentMove);
    uint64_t generateBlackPawn(int square, std::array<uint32_t, 218>& movesVector, int& currentMove);
    uint64_t generateKnight(int square, std::array<uint32_t, 218>& movesVector, int& currentMove);
    uint64_t generateBishop(int square, std::array<uint32_t, 218>& movesVector, int& currentMove);
    uint64_t generateRook(int square, std::array<uint32_t, 218>& movesVector, int& currentMove);
    uint64_t generateQueen(int square, std::array<uint32_t, 218>& movesVector, int& currentMove);
    uint64_t generateWhiteKing(int square, std::array<uint32_t, 218>& movesVector, int& currentMove);
    uint64_t generateBlackKing(int square, std::array<uint32_t, 218>& movesVector, int& currentMove);

    void makeMove(uint32_t move, bool outcomeCheck = true);
    void unmakeMove();

    void isGameOver();

    std::string moveToUCI(uint32_t move);
    uint32_t moveFromUCI(std::string move);
};

long long perft(Board& board, int depth);

std::string square_to_name(int index);


#endif // BOARD_H
