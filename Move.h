/**
 * @file Move.h
 * @author Greg Loose (gloose)
 * @date 2022-05-04
 */

#pragma once
#include "Piece.h"

class Move {
public:
    int row1 = 0;
    int col1 = 0;
    int row2 = 0;
    int col2 = 0;
    Move();
    Move(int r1, int c1, int r2, int c2);
    Move(int compressed);
    int compress();
};