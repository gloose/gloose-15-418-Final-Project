#pragma once

const int WIDTH = 8;
const int HEIGHT = 8;

class Position {
public:
    int row;
    int col;
    Position();
    Position(int r, int c);
    bool isValid();
};