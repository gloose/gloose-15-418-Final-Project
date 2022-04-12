#include "Move.h"

Move::Move() {
    row1 = 0;
    col1 = 0;
    row2 = 0;
    col2 = 0;
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