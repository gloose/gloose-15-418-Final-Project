#include "Position.h"

Position::Position() {
    row = 0;
    col = 0;
}

Position::Position(int r, int c) {
    row = r;
    col = c;
}

bool Position::isValid() {
    return row >= 1 && row <= HEIGHT && col >= 1 && col <= WIDTH;
}