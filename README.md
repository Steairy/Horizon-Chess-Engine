## Overview
Horizon is a UCI chess engine built in C++. It has a NNUE and tries to play the best moves.

## Usage
You can use it by putting it into any chess GUI that allows putting in chess bots. To use the bot in these GUI's, just give the path to the horizon executable.

Make sure you don't actually move the executable itself, since it does some file operations and will break if you move it.

## Compiling
To compile Horizon for yourself, you can run the following terminal command in the project folder:
```
g++ board.cpp NNUE.cpp search.cpp uci.cpp -O3 -march=native -ffast-math -o horizon
```

## Tools used
The bitboard chess implementation is the one from my bitboard repository.

The NNUE was trained using my NN trainer.

CuteChess was used to test the bot.
