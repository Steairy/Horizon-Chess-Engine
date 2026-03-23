#include <bits/stdc++.h>
#include "board.h"
using namespace std;

std::string int_to_hex(uint64_t value) {
    std::stringstream ss;
    ss << "0x" << std::setfill('0') << std::setw(sizeof(uint64_t) * 2) << std::hex << value;
    return ss.str();
}

array<uint64_t, 64> whitePawnAttacks; array<uint64_t, 64> blackPawnAttacks; array<uint64_t, 64> knightAttacks; array<uint64_t, 64> kingAttacks;
array<uint64_t, 64> bishopLookups; array<array<uint64_t, 512>, 64> bishopAttacks;
array<uint64_t, 64> rookLookups; array<array<uint64_t, 4096>, 64> rookAttacks;

const uint64_t bishopMagics[64] = {0x0040120324304008ULL,0x0102060100600200ULL,0x240B0181000422A8ULL,0x0012302103002100ULL,0x000C0520850C0000ULL,0x1000186890000240ULL,0x8100041006010029ULL,0x0004120010030060ULL,0x880E040026010481ULL,0x000240A220303200ULL,0x2290310009090001ULL,0x0000221200810184ULL,0x000200E481060218ULL,0x28000C03A0010000ULL,0x0000000440404000ULL,0x1400400840141000ULL,0x2000926402091240ULL,0x0048000102240080ULL,0x0088020A80091030ULL,0x0F0403880E460004ULL,0x0000200892040000ULL,0x04208802020A0200ULL,0x3000400048048085ULL,0x0800881110040945ULL,0x0000200001084140ULL,0x002118A00822000AULL,0x0340440200440040ULL,0x6001040000440080ULL,0x0061011001004002ULL,0x808204A088040100ULL,0x0029004800460103ULL,0x0201404080030010ULL,0x400018E119212409ULL,0x0000352208020040ULL,0x002440A390084080ULL,0x0122004040040100ULL,0x0020022080100880ULL,0x0000828010748804ULL,0x0118252844008802ULL,0x4300402828008400ULL,0x04401488A0401001ULL,0xC280421001019220ULL,0x0100020090042200ULL,0x0040054202000040ULL,0x0008100606002020ULL,0x011A104102002008ULL,0x20A0050810044080ULL,0x0010001E08200002ULL,0x2000840080400109ULL,0x0600400484004020ULL,0x0000500202210030ULL,0x0218400020042000ULL,0x04C0000085050000ULL,0x0200C00080218188ULL,0x1415080202420900ULL,0x00E6112940844000ULL,0x9001200460200814ULL,0x4400200220140101ULL,0x0012002424940400ULL,0x44800E0020229224ULL,0x0002010080825401ULL,0x0080004041408444ULL,0xC00001040940801BULL,0x102202C800540068ULL};
const uint64_t rookMagics[64] = {0x0180048690204001ULL,0x0040000820001004ULL,0x0100041040892001ULL,0x0080041000080006ULL,0x0408040800088202ULL,0x010010820B000400ULL,0x0240108200050440ULL,0x42000440840105AAULL,0x0008100450080060ULL,0x0400040649200800ULL,0x0402780010202008ULL,0x4804801810082214ULL,0x8009000151020800ULL,0x190A001480CA0008ULL,0x4100104180220900ULL,0x002A400780002040ULL,0x4030801000188400ULL,0x4042480480400818ULL,0x8000400A00089010ULL,0x0008705001060046ULL,0x0000020001220047ULL,0x0008090004088234ULL,0x0090044001001A00ULL,0x0000004010208101ULL,0x084A800100210040ULL,0x0022400024028024ULL,0x000020020400B200ULL,0x2000400404001008ULL,0x0838A40032000800ULL,0x1814010800402003ULL,0x1200800410428090ULL,0x0000108801000104ULL,0x0180004030100220ULL,0x1001080E460802A0ULL,0x8000200041010A1AULL,0x0000022010100004ULL,0x004108004100020CULL,0x0000092830800212ULL,0x00B8004544000A10ULL,0xA0840084044000A0ULL,0x0044030280048800ULL,0x8224014008000408ULL,0x0E0100C010461000ULL,0x08020040BE220006ULL,0x0000120200070100ULL,0x010040102138006BULL,0x0022044452024209ULL,0x0002200110008804ULL,0x034000802820802AULL,0x0005400020000884ULL,0x0000C01000040084ULL,0x400840201A004202ULL,0x4082080091000108ULL,0x004000C804010200ULL,0x4001220084201100ULL,0x8A90200040025420ULL,0xC880184100800021ULL,0x1440800821001242ULL,0x00022920002D00C5ULL,0x0448100840220002ULL,0x1804620008314612ULL,0x0000018810030402ULL,0x0043009400420009ULL,0x4000040100204082ULL};

array<array<uint64_t, 64>, 64> inBetweenMasks = {};

//used only for generating bitboards
vector<uint64_t> getBlockers(uint64_t lookupMask){
    vector<uint64_t> blockers;
    vector<int> blockIndexes;

    uint64_t tempMask = lookupMask;
    while(tempMask){
        blockIndexes.push_back(__builtin_ctzll(tempMask));
        tempMask &= tempMask-1;
    }

    for(int i = 0; i < (1 << blockIndexes.size()); i++){
        int t = i;
        uint64_t block = 0ULL;
        while(t){
            block |= 1ULL << blockIndexes[__builtin_ctz(t)];
            t &= t-1;
        }
        blockers.push_back(block);
    }
    return blockers;
}
//used only for generating bitboards
inline void safeShift(uint64_t& attackMask, int square, int amount){
    if(square + amount < 0 || square + amount > 63){
        return;
    }
    if(abs((square%8)-((square+amount)%8)) > 2){
        return;
    }
    attackMask |= 1ULL << (square+amount);
}

void calculateBitboards(){
    //for non sliding pieces
    vector<vector<int>> offsets = {{7,9}, {-7,-9}, {-17, -15, -10, -6, 6, 10, 15, 17}, {-9, -8, -7, -1, 1, 7, 8, 9}};
    vector<array<uint64_t, 64>*> nonSliding = {&whitePawnAttacks, &blackPawnAttacks, &knightAttacks, &kingAttacks};
    for(int i = 0; i < 4; i++){
        for(int sq = 0; sq < 64; sq++){
            uint64_t atk = 0ULL;
            for(int off : offsets[i]){
                safeShift(atk, sq, off);
            }
            (*nonSliding[i])[sq] = atk;
        }
    }
    //for sliding pieces
    offsets = {{9,7,-7,-9}, {8,1,-1,-8}};
    for(int piece = 0; piece < 2; piece++){
        for(int sq = 0; sq < 64; sq++){
            //find lookup table
            uint64_t lookup = 0ULL;
            for(int dir : offsets[piece]){
                uint64_t lastSet = 0ULL;
                int current = sq;
                while(true){
                    uint64_t prev = lookup;
                    safeShift(lookup, current, dir);
                    if(prev == lookup){
                        lookup &= ~(lastSet);
                        break;
                    }
                    lastSet = lookup-prev;
                    current += dir;
                }
            }
            lookup &= ~(1ULL << sq);
            if(piece == 0){
                bishopLookups[sq] = lookup;
            } else if(piece == 1){
                rookLookups[sq] = lookup;
            }          
            //go through each blocker configuration
            vector<uint64_t> blockers = getBlockers(lookup);
            for(uint64_t blocker : blockers){
                uint64_t attacks = 0ULL;
                for(int dir : offsets[piece]){
                    int current = sq;
                    while(true){
                        uint64_t prev = attacks;
                        safeShift(attacks, current, dir);
                        if((prev == attacks) || ((1ULL << (current+dir)) & blocker)){
                            break;
                        }
                        current += dir;
                    }
                }
                if(piece == 0){
                    bishopAttacks[sq][(blocker * bishopMagics[sq]) >> 55] = attacks;
                } else if(piece == 1){
                    rookAttacks[sq][(blocker * rookMagics[sq]) >> 52] = attacks;
                }
            }
        }
    }
    //Calculate in between masks
    const array<int, 8> rays = {9,7,-7,-9,8,1,-1,-8};
    for(int i = 0; i < 64; i++){
        for(int ray : rays){
            uint64_t mask = 0ULL;
            int sq = i;
            while(true){
                uint64_t prev = mask;
                safeShift(mask, sq, ray);
                if(prev == mask){
                    break;
                }
                sq += ray;
                inBetweenMasks[i][sq] = prev;
            }
        }
    }
}

Board::Board(){
    calculateBitboards();
    initZobrist();
}

void Board::importFEN(string fen){
    //reset board state
    bitboards.fill(0ULL);
    moves = {}; legalMoves = {}; stateHistory = {}; hashHistory = {};
    totalMoves = 0; castlingRights = {false, false, false, false}; halfMoveClock = 0; enPassantSquare = 0;
    turn = WHITE; gameOver = false; outcome = 0; legalMoveCount = 0;
    //read fen
    istringstream ss(fen);
    array<string, 6> tokens;
    string token; int ind = 0;
    while(getline(ss, token, ' ')){
        tokens[ind++] = token;
    }
    //start setting the board
    string pos = tokens[0];
    int rank = 7;
    int file = 0;
    for(char f : pos){
        if(f == '/'){
            rank--;
            file = 0;
            continue;
        }
        if(isdigit(f)){
            file += f - '0';
            continue;
        }
        int square = rank * 8 + file;
        uint64_t mask = 1ULL << square;
        switch(f){
            case 'P': bitboards[0] |= mask; break;
            case 'N': bitboards[1] |= mask; break;
            case 'B': bitboards[2] |= mask; break;
            case 'R': bitboards[3] |= mask; break;
            case 'Q': bitboards[4] |= mask; break;
            case 'K': bitboards[5] |= mask; break;
            case 'p': bitboards[6] |= mask; break;
            case 'n': bitboards[7] |= mask; break;
            case 'b': bitboards[8] |= mask; break;
            case 'r': bitboards[9] |= mask; break;
            case 'q': bitboards[10] |= mask; break;
            case 'k': bitboards[11] |= mask; break;
        }

        file++;
    }
    if(tokens[1] == "b"){
        turn = BLACK;
    }
    if(tokens[1] == "w"){
        turn = WHITE;
    }
    for(char i : tokens[2]){
        if(i == 'K'){
            castlingRights[0] = true;
        }
        if(i == 'Q'){
            castlingRights[1] = true;
        }
        if(i == 'k'){
            castlingRights[2] = true;
        }
        if(i == 'q'){
            castlingRights[3] = true;
        }
    }
    if(tokens[3] != "-"){
        enPassantSquare = ((tokens[3][1] - '1') * 8) + (tokens[3][0]-97);
    }
    halfMoveClock = stoi(tokens[4]);

    initZobrist();
}

bool Board::checkBit(uint64_t bitboard, int bit){
    return ((bitboard >> bit) & 1) == 1;
}

void Board::initZobrist(){
    zobrist.hash = 0ULL;
    mt19937_64 rng(random_device{}());
    for(int square = 0; square < 64; square++){
        zobrist.enPassantTable[square] = rng();
        for(int piece = 0; piece < 12; piece++){
            zobrist.hashTable[square][piece] = rng();
            if(checkBit(bitboards[piece], square)){
                zobrist.hash ^= zobrist.hashTable[square][piece];
            }
        }
    }

    for(int i = 0; i < 5; i++){
        zobrist.boardStateTable[i] = rng();
    }

    for(int i = 1; i < 5; i++){
        if(castlingRights[i-1]){
            zobrist.hash ^= zobrist.boardStateTable[i];
        }
    }

    if(turn == BLACK){
        zobrist.hash ^= zobrist.boardStateTable[0];
    }

    if (enPassantSquare != 0) {
        zobrist.hash ^= zobrist.enPassantTable[enPassantSquare];
    }
}

uint16_t Board::packState(){
    bool enPassantExists = enPassantSquare != 0;
    int encodedEnPassant = (enPassantSquare / 8 == 2 ? 0 : 1) | ((enPassantSquare%8) << 1);
    uint16_t newState = (halfMoveClock & 0x7F) | (castlingRights[0] << 7) | (castlingRights[1] << 8) | (castlingRights[2] << 9) | (castlingRights[3] << 10) | ((enPassantExists & 0x1) << 11) | ((encodedEnPassant & 0xF) << 12);
    return newState;
}

void Board::unpackState(uint16_t packedState){
    halfMoveClock = packedState&0x7F;
    castlingRights[0] = (packedState >> 7)&0x1;
    castlingRights[1] = (packedState >> 8)&0x1;
    castlingRights[2] = (packedState >> 9)&0x1;
    castlingRights[3] = (packedState >> 10)&0x1;
    bool enPassantExists = (packedState >> 11)&0x1;
    if(enPassantExists){
        enPassantSquare = ((packedState >> 12)&0x1 ? 40 : 16) + ((packedState >> 13)&0x7);
    }
    else{
        enPassantSquare = 0;
    }
}

uint32_t Board::encodeMove(int startSquare, int endSquare, int movingPiece, bool isCapture, int capturedPiece, bool isEnPassant, bool isCastling, bool castleSide, bool isPromotion, int promotedPiece){
    uint32_t move = (startSquare & 0x3F) | ((endSquare & 0x3F) << 6) | ((movingPiece & 0xF) << 12) | (isCapture << 16) | ((capturedPiece & 0xF) << 17) | (isEnPassant << 21) | (isCastling << 22) | (castleSide << 23) | (isPromotion << 24) | ((promotedPiece & 0xF) << 25);
    return move;
}

uint64_t Board::getAllPieces(){
    return bitboards[0] | bitboards[1] | bitboards[2] | bitboards[3] | bitboards[4] | bitboards[5] | bitboards[6] | bitboards[7] | bitboards[8] | bitboards[9] | bitboards[10] | bitboards[11];
}

uint64_t Board::getFriendlyPieces(){
    int add = 6*turn;
    return bitboards[add] | bitboards[1+add] | bitboards[2+add] | bitboards[3+add] | bitboards[4+add] | bitboards[5+add];
}

uint64_t Board::getEnemyPieces(){
    int add = 6*!turn;
    return bitboards[add] | bitboards[1+add] | bitboards[2+add] | bitboards[3+add] | bitboards[4+add] | bitboards[5+add];
}

void Board::removeBit(int bitboardIndex, int bit){
    bitboards[bitboardIndex] &= ~(1ULL << bit);
}

int Board::pieceAtPosition(int square){
    for(int i = 0; i < 12; i++){
        if(checkBit(bitboards[i], square)){
            return i;
        }
    }
    return -1;
}

bool Board::squareAttacked(int square, bool attacker){
    bool flag = false;
    uint64_t allPieces = getAllPieces();

    uint64_t bishopBlockers = ((bishopLookups[square] & allPieces) * bishopMagics[square]) >> 55;
    uint64_t rookBlockers = ((rookLookups[square] & allPieces) * rookMagics[square]) >> 52;

    uint64_t bishopAttacksCurrent = bishopAttacks[square][bishopBlockers];
    uint64_t rookAttacksCurrent = rookAttacks[square][rookBlockers];
    uint64_t queenAttacksCurrent = bishopAttacksCurrent | rookAttacksCurrent;

    static const array<array<uint64_t, 64>*, 2> pawnAttackTables = {&blackPawnAttacks, &whitePawnAttacks};

    int add = 6*attacker;
    flag |= ((*pawnAttackTables[attacker])[square] & bitboards[add]) != 0;
    flag |= (knightAttacks[square] & bitboards[1+add]) != 0;
    flag |= (bishopAttacksCurrent & bitboards[2+add]) != 0;
    flag |= (rookAttacksCurrent & bitboards[3+add]) != 0;
    flag |= (queenAttacksCurrent & bitboards[4+add]) != 0;
    flag |= (kingAttacks[square] & bitboards[5+add]) != 0;
    return flag;
}

uint64_t Board::generateWhitePawn(int square, array<uint32_t, 218>& movesVector, int& currentMove){
    uint64_t allPieces = getAllPieces();
    uint64_t enemyPieces = getEnemyPieces();
    int singlePush = square+8;
    int doublePush = square+16;
    uint64_t pushMask = 0x00000000FF000000;

    bool singlePushClosed = checkBit(allPieces, singlePush);
    uint64_t movesMask = ((1ULL << singlePush) * !singlePushClosed) | ((1ULL << doublePush) * !singlePushClosed * !checkBit(allPieces, doublePush) * checkBit(pushMask, doublePush));
    movesMask |= whitePawnAttacks[square] & enemyPieces;

    bool canEnPassant = checkBit(whitePawnAttacks[square], enPassantSquare) && enPassantSquare != 0;
    if(canEnPassant){
        uint32_t enPassantMove = encodeMove(square, enPassantSquare, 0, true, 6, true, false, false, false, 0);
        movesVector[currentMove++] = enPassantMove;
    }

    return movesMask;
}

uint64_t Board::generateBlackPawn(int square, array<uint32_t, 218>& movesVector, int& currentMove){
    uint64_t allPieces = getAllPieces();
    uint64_t enemyPieces = getEnemyPieces();
    int singlePush = square-8;
    int doublePush = square-16;
    uint64_t pushMask = 0x000000FF00000000;

    bool singlePushClosed = checkBit(allPieces, singlePush);
    uint64_t movesMask = ((1ULL << singlePush) * !singlePushClosed) | ((1ULL << doublePush) * !singlePushClosed * !checkBit(allPieces, doublePush) * checkBit(pushMask, doublePush));
    movesMask |= blackPawnAttacks[square] & enemyPieces;

    bool canEnPassant = checkBit(blackPawnAttacks[square], enPassantSquare) && enPassantSquare != 0;
    if(canEnPassant){
        uint32_t enPassantMove = encodeMove(square, enPassantSquare, 6, true, 0, true, false, false, false, 0);
        movesVector[currentMove++] = enPassantMove;
    }

    return movesMask;
}

uint64_t Board::generateKnight(int square, array<uint32_t, 218>& movesVector, int& currentMove){
    return knightAttacks[square];
}

uint64_t Board::generateBishop(int square, array<uint32_t, 218>& movesVector, int& currentMove){
    uint64_t allPieces = getAllPieces();
    uint64_t blockerIndex = ((bishopLookups[square] & allPieces) * bishopMagics[square]) >> 55;
    return bishopAttacks[square][blockerIndex];
}

uint64_t Board::generateRook(int square, array<uint32_t, 218>& movesVector, int& currentMove){
    uint64_t allPieces = getAllPieces();
    uint64_t blockerIndex = ((rookLookups[square] & allPieces) * rookMagics[square]) >> 52;
    return rookAttacks[square][blockerIndex];
}

uint64_t Board::generateQueen(int square, array<uint32_t, 218>& movesVector, int& currentMove){
    uint64_t allPieces = getAllPieces();
    uint64_t bishopBlockerIndex = ((bishopLookups[square] & allPieces) * bishopMagics[square]) >> 55;
    uint64_t rookBlockerIndex = ((rookLookups[square] & allPieces) * rookMagics[square]) >> 52;
    return (bishopAttacks[square][bishopBlockerIndex] | rookAttacks[square][rookBlockerIndex]);
}

uint64_t Board::generateWhiteKing(int square, array<uint32_t, 218>& movesVector, int& currentMove){
    uint64_t allPieces = getAllPieces();
    if(castlingRights[1] && (allPieces & 0xE) == 0){ // queenside castling
        uint32_t castleMove = encodeMove(square, 2, 5, false, 0, false, true, true, false, 0);
        movesVector[currentMove++] = castleMove;
    }
    if(castlingRights[0] && (allPieces & 0x60) == 0){ // kingside castling
        uint32_t castleMove = encodeMove(square, 6, 5, false, 0, false, true, false, false, 0);
        movesVector[currentMove++] = castleMove;
    }
    return kingAttacks[square];
}

uint64_t Board::generateBlackKing(int square, array<uint32_t, 218>& movesVector, int& currentMove){
    uint64_t allPieces = getAllPieces();
    if(castlingRights[3] && (allPieces & 0x0E00000000000000) == 0){ // queenside castling
        uint32_t castleMove = encodeMove(square, 58, 11, false, 0, false, true, true, false, 0);
        movesVector[currentMove++] = castleMove;
    }
    if(castlingRights[2] && (allPieces & 0x6000000000000000) == 0){ // kingside castling
        uint32_t castleMove = encodeMove(square, 62, 11, false, 0, false, true, false, false, 0);
        movesVector[currentMove++] = castleMove;
    }
    return kingAttacks[square];
}

void Board::generatePseudoLegal(int piece, int square, array<uint32_t, 218>& movesVector, int& currentMove, bool capturesOnly){

    uint64_t friendlyPieces = getFriendlyPieces();
    uint64_t enemyPieces = getEnemyPieces();

    uint64_t movesMask;

    if(piece == 0){
        movesMask = generateWhitePawn(square, movesVector, currentMove);
    }

    if(piece == 1 || piece == 7){
        movesMask = generateKnight(square, movesVector, currentMove);
    }

    if(piece == 2 || piece == 8){
        movesMask = generateBishop(square, movesVector, currentMove);
    }

    if(piece == 3 || piece == 9){
        movesMask = generateRook(square, movesVector, currentMove);
    }

    if(piece == 4 || piece == 10){
        movesMask = generateQueen(square, movesVector, currentMove);
    }

    if(piece == 6){
        movesMask = generateBlackPawn(square, movesVector, currentMove);
    }

    if(piece == 5){
        movesMask = generateWhiteKing(square, movesVector, currentMove);
    }

    if(piece == 11){
        movesMask = generateBlackKing(square, movesVector, currentMove);
    }


    movesMask &= ~friendlyPieces;
    if(capturesOnly) movesMask &= enemyPieces;
    while(movesMask){
        int newSquare = __builtin_ctzll(movesMask);
        bool isCapture = checkBit(enemyPieces, newSquare);
        int capturedPiece = 0;
        if(isCapture){
            capturedPiece = pieceAtPosition(newSquare); //TODO: make this faster
        }

        if((piece == 0 && (checkBit(0xFF00000000000000, newSquare)) != 0) || (piece == 6 && (checkBit(0x00000000000000FF, newSquare)) != 0)){ // promotion
            movesVector[currentMove++] = encodeMove(square, newSquare, piece, isCapture, capturedPiece, false, false, false, true, piece+1);
            movesVector[currentMove++] = encodeMove(square, newSquare, piece, isCapture, capturedPiece, false, false, false, true, piece+2);
            movesVector[currentMove++] = encodeMove(square, newSquare, piece, isCapture, capturedPiece, false, false, false, true, piece+3);
            movesVector[currentMove++] = encodeMove(square, newSquare, piece, isCapture, capturedPiece, false, false, false, true, piece+4);

        }

        else{
            movesVector[currentMove++] = encodeMove(square, newSquare, piece, isCapture, capturedPiece, false, false, false, false, 0);
        }
        movesMask &= movesMask-1; // remove least significant bit
    }
}

void Board::makeMove(uint32_t move, bool outcomeCheck){
    int startSquare = move&0x3F;
    int endSquare = (move >> 6)&0x3F;
    int movingPiece = (move >> 12)&0xF;
    bool isCapture = (move >> 16)&0x1;
    int capturedPiece = (move >> 17)&0xF;
    bool isEnPassant = (move >> 21)&0x1;
    bool isCastling = (move >> 22)&0x1;
    bool castlingSide = (move >> 23)&0x1;
    bool isPromotion = (move >> 24)&0x1;
    int promotedPiece = (move >> 25)&0xF;

    std::array<bool, 4> oldCastlingRights = castlingRights;

    stateHistory[totalMoves] = packState();
    hashHistory[totalMoves] = zobrist.hash;
    if(enPassantSquare != 0){
        zobrist.hash ^= zobrist.enPassantTable[enPassantSquare];
    }
    

    enPassantSquare = 0;
    halfMoveClock++;
    halfMoveClock *= !((movingPiece == 0) || (movingPiece == 6) || isCapture);

    removeBit(movingPiece, startSquare);
    zobrist.hash ^= zobrist.hashTable[startSquare][movingPiece];
    bitboards[movingPiece] |= (1ULL << endSquare); // add to end square
    zobrist.hash ^= zobrist.hashTable[endSquare][movingPiece];

    if(isEnPassant){
        int captureLocation = turn == WHITE ? endSquare-8 : endSquare+8;
        removeBit(capturedPiece, captureLocation);
        zobrist.hash ^= zobrist.hashTable[captureLocation][capturedPiece];
    }

    if(isCapture && !isEnPassant){
        removeBit(capturedPiece, endSquare);
        zobrist.hash ^= zobrist.hashTable[endSquare][capturedPiece];
    }

    if(isPromotion){
        removeBit(movingPiece, endSquare);
        zobrist.hash ^= zobrist.hashTable[endSquare][movingPiece];
        bitboards[promotedPiece] |= (1ULL << endSquare);
        zobrist.hash ^= zobrist.hashTable[endSquare][promotedPiece];
    }

    if(isCastling){
        int rookIndex = movingPiece - 2;
        static array<int,4> removeIndexes = {7, 0, 63, 56};
        static array<int,4> addIndexes = {5, 3, 61, 59};

        int stateIndex = (turn << 1) + castlingSide;

        removeBit(rookIndex, removeIndexes[stateIndex]);
        zobrist.hash ^= zobrist.hashTable[removeIndexes[stateIndex]][rookIndex];
        bitboards[rookIndex] |= 1ULL << addIndexes[stateIndex];
        zobrist.hash ^= zobrist.hashTable[addIndexes[stateIndex]][rookIndex];
    }

    if(movingPiece == 5){
        castlingRights[0] = false;
        castlingRights[1] = false;
    }

    if(movingPiece == 11){
        castlingRights[2] = false;
        castlingRights[3] = false;
    }


    if((startSquare == 0) || (endSquare == 0)){
        castlingRights[1] = false;
    }

    if((startSquare == 7) || (endSquare == 7)){
        castlingRights[0] = false;
    }

    if((startSquare == 56) || (endSquare == 56)){
        castlingRights[3] = false;
    }

    if((startSquare == 63) || (endSquare == 63)){
        castlingRights[2] = false;
    }


    if(((movingPiece == 0) || (movingPiece == 6)) && (abs(startSquare-endSquare) == 16)){ // double push
        enPassantSquare = (startSquare+endSquare) >> 1; // middle of the 2 squares;
        zobrist.hash ^= zobrist.enPassantTable[enPassantSquare];
    }

    if(oldCastlingRights != castlingRights){
        for(int i = 0; i < 4; i++){
            if(oldCastlingRights[i] != castlingRights[i]){
                zobrist.hash ^= zobrist.boardStateTable[i+1];
            }
        }
    }

    turn = 1-turn;
    zobrist.hash ^= zobrist.boardStateTable[0];

    moves[totalMoves] = move;
    totalMoves++;

    if(outcomeCheck){
        isGameOver();
    }
}

void Board::unmakeMove(){
    turn = 1-turn;
    totalMoves--;

    unpackState(stateHistory[totalMoves]);
    stateHistory[totalMoves+1] = 0;

    zobrist.hash = hashHistory[totalMoves];
    hashHistory[totalMoves+1] = 0;

    gameOver = false;
    outcome = 0;


    uint32_t move = moves[totalMoves];
    moves[totalMoves+1] = 0;

    int startSquare = move&0x3F;
    int endSquare = (move >> 6)&0x3F;
    int movingPiece = (move >> 12)&0xF;
    bool isCapture = (move >> 16)&0x1;
    int capturedPiece = (move >> 17)&0xF;
    bool isEnPassant = (move >> 21)&0x1;
    bool isCastling = (move >> 22)&0x1;
    bool castlingSide = (move >> 23)&0x1;
    bool isPromotion = (move >> 24)&0x1;
    int promotedPiece = (move >> 25)&0xF;

    removeBit(movingPiece, endSquare);
    bitboards[movingPiece] |= 1ULL << startSquare;

    if(isEnPassant){
        bitboards[capturedPiece] |= 1ULL << (endSquare-(turn == WHITE ? 8 : -8));
    }

    if(isCapture && !isEnPassant){
        bitboards[capturedPiece] |= 1ULL << endSquare;
    }

    if(isPromotion){
        removeBit(promotedPiece, endSquare);
    }

    if(isCastling){
        int rookIndex = movingPiece-2;
        static array<int,4> removeIndexes = {5, 3, 61, 59};
        static array<int,4> addIndexes = {7, 0, 63, 56};

        int stateIndex = (turn << 1) + castlingSide;
        removeBit(rookIndex, removeIndexes[stateIndex]);
        bitboards[rookIndex] |= 1ULL << addIndexes[stateIndex];
    }
}

void Board::isGameOver(){
    array<uint32_t, 218> pseudoLegal;
    bool legalExists = false;
    int currentMove = 0;
    int prev;
    bool inCheck = squareAttacked(__builtin_ctzll(bitboards[5+(6*turn)]), 1-turn);

    for(int piece = (turn*6); piece < 6+(turn*6); piece++){
        uint64_t bb = bitboards[piece];
        while(bb){
            int square = __builtin_ctzll(bb);
            bb &= bb-1;

            prev = currentMove;
            generatePseudoLegal(piece, square, pseudoLegal, currentMove);
            for(int i = prev; i < currentMove; i++){
                uint32_t move = pseudoLegal[i];
                bool isCastling = (move >> 22)&0x1;
                bool friendlySide = turn;

                if(isCastling) continue;

                if(!leavesInCheck(move)){
                    legalExists = true;
                    break;
                }
            }
            if(legalExists){
                break;
            }
        }
        if(legalExists){
            break;
        }
    }

    // Repetition check
    int count = 0;
    bool repetition = false;
    for(int ind = totalMoves-2; ind >= 0; ind-=2){
        if(zobrist.hash == hashHistory[ind]){
            if(++count >= 2){
                repetition = true;
                break;
            }
        }
    }

    // Insufficient material check
    bool insufficient = true;
    int minorCount = __builtin_popcountll(bitboards[1] | bitboards[2] | bitboards[7] | bitboards[8]);
    if((minorCount > 1) || (bitboards[0] | bitboards[6] | bitboards[3] | bitboards[9] | bitboards[4] | bitboards[10])){
        insufficient = false;
    }

    if(inCheck && !legalExists){
        gameOver = true;
        outcome = turn == WHITE ? -1 : 1;
    }

    else if(repetition || halfMoveClock >= 100 || (!legalExists && !inCheck) || insufficient){
        gameOver = true;
        outcome = 0;
    }
}

void Board::generatePinMasks(){
    uint64_t allPieces = getAllPieces();
    uint64_t friendlyPieces = getFriendlyPieces();

    pinMasks = {};
    int add = 6*(1-turn);
    int kingSquare = __builtin_ctzll(bitboards[5+(6*turn)]);
    
    for(int piece = 2; piece < 5; piece++){
        uint64_t slidingBitboard = bitboards[piece+add];
        while(slidingBitboard){
            int sq = __builtin_ctzll(slidingBitboard);
            uint64_t pinMask = inBetweenMasks[sq][kingSquare];
            
            if(piece == 2){ // bishop
                pinMask &= bishopLookups[sq];
            }
            if(piece == 3){ // rook
                pinMask &= rookLookups[sq];
            }
            //no queen needed because if pinmask != 0 then it is in a diagonal or straight line

            uint64_t squares = pinMask & allPieces;
            if(pinMask && (__builtin_popcountll(squares) == 1)){ //if the attacker can actually reach the king and there is only 1 piece between king and attacker
                squares &= friendlyPieces;
                pinMask |= 1ULL << sq;
                while(squares){
                    pinMasks[__builtin_ctzll(squares)] = pinMask;
                    squares &= squares-1;
                }
            }
            slidingBitboard &= slidingBitboard-1;
        }
    }
}

bool Board::leavesInCheck(uint32_t move){
    array<uint64_t, 12> tempBoards = bitboards;
    int startSquare = move&0x3F; int endSquare = (move >> 6)&0x3F; int movingPiece = (move >> 12)&0xF;
    bool isCapture = (move >> 16)&0x1; int capturedPiece = (move >> 17)&0xF; bool isEnPassant = (move >> 21)&0x1;
    
    //making the move as minimally as possible
    tempBoards[movingPiece] &= ~(1ULL << startSquare);
    tempBoards[movingPiece] |= (1ULL << endSquare);

    if(isEnPassant){
        int captureLocation = turn == WHITE ? endSquare-8 : endSquare+8;
        tempBoards[capturedPiece] &= ~(1ULL << captureLocation);
    }

    if(isCapture && !isEnPassant){
        tempBoards[capturedPiece] &= ~(1ULL << endSquare);
    }
    //checking if move left in check(same thing as squareAttacked but for tempBoards)
    int square = __builtin_ctzll(tempBoards[5+(6*turn)]);
    bool flag = false;
    uint64_t allPieces = tempBoards[0] | tempBoards[1] | tempBoards[2] | tempBoards[3] | tempBoards[4] | tempBoards[5] | tempBoards[6] | tempBoards[7] | tempBoards[8] | tempBoards[9] | tempBoards[10] | tempBoards[11];

    uint64_t bishopBlockers = ((bishopLookups[square] & allPieces) * bishopMagics[square]) >> 55;
    uint64_t rookBlockers = ((rookLookups[square] & allPieces) * rookMagics[square]) >> 52;

    uint64_t bishopAttacksCurrent = bishopAttacks[square][bishopBlockers];
    uint64_t rookAttacksCurrent = rookAttacks[square][rookBlockers];
    uint64_t queenAttacksCurrent = bishopAttacksCurrent | rookAttacksCurrent;

    static const array<array<uint64_t, 64>*, 2> pawnAttackTables = {&blackPawnAttacks, &whitePawnAttacks};

    int add = 6*!turn;
    flag |= ((*pawnAttackTables[!turn])[square] & tempBoards[add]) != 0;
    flag |= (knightAttacks[square] & tempBoards[1+add]) != 0;
    flag |= (bishopAttacksCurrent & tempBoards[2+add]) != 0;
    flag |= (rookAttacksCurrent & tempBoards[3+add]) != 0;
    flag |= (queenAttacksCurrent & tempBoards[4+add]) != 0;
    flag |= (kingAttacks[square] & tempBoards[5+add]) != 0;
    return flag;
}


void Board::generateLegal(bool capturesOnly){
    generatePinMasks();
    static array<uint32_t, 218> pseudoLegal;

    bool inCheck = squareAttacked(__builtin_ctzll(bitboards[5+(6*turn)]), 1-turn);

    int currentMove = 0;
    for(int piece = 0+(turn*6); piece < 6+(turn*6); piece++){ //generate all pseudo legal moves first
        uint64_t bb = bitboards[piece];
        while(bb){
            int square = __builtin_ctzll(bb);
            bb &= bb-1;
            generatePseudoLegal(piece, square, pseudoLegal, currentMove, capturesOnly);
        }
    }

    int legalCurrent = 0;
    for(int i = 0; i < currentMove; i++){
        uint32_t move = pseudoLegal[i];
        bool isCastling = (move >> 22)&0x1;
        if(isCastling){
            int startSquare = move&0x3F;
            bool castlingSide = (move >> 23)&0x1;
            if(squareAttacked(__builtin_ctzll(bitboards[5+(6*turn)]), !turn) || squareAttacked(startSquare + (castlingSide ? -1 : 1), 1-turn)){
                continue;
            }
        }
        //fast calculation to see if a move leaves king in check

        int startSquare = move&0x3F; int endSquare = (move >> 6)&0x3F; int movingPiece = (move >> 12)&0xF; bool isEnPassant = (move >> 21)&0x1;

        bool illegal;
        if(inCheck || (movingPiece == 5+(6*turn)) || isEnPassant){ // needs full check
            illegal = leavesInCheck(move);
        }
        else{ //only need to test if pinned
            uint64_t pinMask = pinMasks[startSquare];
            illegal = (pinMask) && ((pinMask & (1ULL << endSquare)) == 0);
        }

        if(!illegal){
            legalMoves[legalCurrent++] = move;
        }
    }

    legalMoveCount = legalCurrent;
}

std::string Board::moveToUCI(uint32_t move){
    std::string out = square_to_name(move & 0x3F) + square_to_name((move >> 6) & 0x3F);
    if((move >> 24)&0x1){ // is promotion
        std::array<char, 4> promotionSymbols = {'n', 'b', 'r', 'q'};
        int promotedPiece = (move >> 25)&0xF;
        out += promotionSymbols[(promotedPiece%6)-1];
    }
    return out;
}

uint32_t Board::moveFromUCI(std::string move){
    int fromSq = (move[1]-'1')*8 + (move[0]-'a');
    int toSq   = (move[3]-'1')*8 + (move[2]-'a');
    generateLegal();
    for(int i = 0; i < legalMoveCount; i++){
        if(((legalMoves[i] & 0x3F) == fromSq) && (((legalMoves[i] >> 6) & 0x3F) == toSq)){
            if(move.length() == 5){
                std::array<char, 4> promotionSymbols = {'n', 'b', 'r', 'q'};
                int promotedPiece = (legalMoves[i] >> 25)&0xF;
                if(promotionSymbols[(promotedPiece%6)-1] == move[4]){
                    return legalMoves[i];
                }
            }
            else{
                return legalMoves[i];
            }
        }
    }
    return 0;
}

long long perft(Board& board, int depth){
    long long visited = 0;
    if(depth == 0){
        return 1;
    }

    board.generateLegal();
    array<uint32_t, 218> moves = board.legalMoves;
    int c = board.legalMoveCount;
    for(int i = 0; i < c; i++){
        uint32_t move = moves[i];
        board.makeMove(move, false);
        visited += perft(board, depth-1);
        board.unmakeMove();
    }
    return visited;
}

std::string square_to_name(int index) {
    return std::string{char('a' + (index % 8))} + std::to_string((index / 8) + 1);
}