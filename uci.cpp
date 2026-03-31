#include "board.h"
#include "search.h"
#include <iostream>
#include <string>

class Interface {
    public:
    Search search;
    Board board;

    void handleCommand(std::string command){
        std::vector<std::string> tokens;
        std::string token;
        std::stringstream ss(command);

        while(ss >> token){
            tokens.push_back(token);
        }

        if(tokens[0] == "uci"){
            std::cout << "id name Horizon 1.1" << std::endl;
            std::cout << "id author Steairy" << std::endl;
            std::cout << "uciok" << std::endl;
        }

        if(tokens[0] == "isready"){
            std::cout << "readyok" << std::endl;
        }

        if(tokens[0] == "ucinewgame"){
            search = Search();
            board = Board();
            board.generateLegal();
            board.initZobrist();
        }

        if(tokens[0] == "position"){
            int currentToken = 1;
            if(tokens[1] == "startpos"){
                board.importFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
                currentToken++;
            }
            else if(tokens[1] == "fen"){
                std::string fen = "";
                for(int i = 2; i <= 7; i++){
                    fen += tokens[i] + " ";
                }
                board.importFEN(fen);
                currentToken += 7;
            }

            if(currentToken < tokens.size() && tokens[currentToken] == "moves"){
                for(int i = currentToken+1; i < tokens.size(); i++){
                    board.generateLegal();
                    uint32_t m = board.moveFromUCI(tokens[i]);
                    board.makeMove(m);
                }
            }

            board.generateLegal();
        }

        if(tokens[0] == "go"){
            int moveTime = -1; int timeLeft = -1; int increment = 0;
            for(int i = 1; i < tokens.size(); i++){ // handle optional settings
                if(tokens[i] == "movetime"){
                    i++;
                    moveTime = std::stoi(tokens[i]);
                }
                if((tokens[i] == "wtime" && board.turn == board.WHITE) || (tokens[i] == "btime" && board.turn == board.BLACK)){ // our own time left
                    i++;
                    timeLeft = std::stoi(tokens[i]);
                }

                if((tokens[i] == "winc" && board.turn == board.WHITE) || (tokens[i] == "binc" && board.turn == board.BLACK)){ // our own increment
                    i++;
                    increment = std::stoi(tokens[i]);
                }
            }

            if(moveTime == -1 && timeLeft != -1){
                moveTime = (timeLeft * 0.05) + (increment * 0.8);
            }

            if(moveTime == -1){
                moveTime = 1000; // default
            }

            int eval = search.iterative(board, moveTime);
            long long nodes = search.info.nodeCount;
            uint8_t depth = search.info.iterativeDepth-1;

            uint32_t bestMove = search.bestMove;

            std::string move = bestMove != 0 ? board.moveToUCI(bestMove) : "0000";

            printf("info depth %d score cp %d nodes %lld nps %lld\n", depth, eval, nodes, (nodes*1000/moveTime)); fflush(stdout);
            std::cout << "bestmove " << move << std::endl;
        }
    }

    void startInterface(){
        std::ios_base::sync_with_stdio(false);
        std::cin.tie(NULL);

        std::string line;
        while(std::getline(std::cin, line)){
            if (line.empty()) continue;

            if(line == "quit"){
                break;
            }
            
            handleCommand(line);
        }
    }
};

int main(){
    Interface i;
    i.startInterface();
}