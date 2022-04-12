#pragma once
#include "Piece.h"

class Move {
public:
    int row1;
    int col1;
    int row2;
    int col2;
    Move();
    Move(int r1, int c1, int r2, int c2);
    Move(int compressed);
    int compress();
};