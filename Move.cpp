/**
 * @file Move.cpp
 * @author Greg Loose (gloose)
 * @brief This class represents a move, by which a player moves a piece from
 * one position to another. Offers compression of a move into the space of a
 * single int for ease of transmission over MPI messages.
 * 
 * @date 2022-05-04
 */

#include "Move.h"

Move::Move() {
}

Move::Move(int r1, int c1, int r2, int c2) {
    row1 = r1;
    col1 = c1;
    row2 = r2;
    col2 = c2;
}

Move::Move(int compressed) {
    int mask = 0xFF;
    row1 = (compressed >> 24) & mask;
    col1 = (compressed >> 16) & mask;
    row2 = (compressed >> 8) & mask;
    col2 = (compressed) & mask;
}

int Move::compress() {
    return (row1 << 24) | (col1 << 16) | (row2 << 8) | (col2);
}