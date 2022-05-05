/**
 * @file Piece.h
 * @author Greg Loose (gloose)
 * @date 2022-05-04
 */

#pragma once
#include <string>

enum Color {
    NOCOLOR,
    WHITE,
    BLACK
};

enum PieceType {
    NONE,
    PAWN,
    ROOK,
    KNIGHT,
    BISHOP,
    QUEEN,
    KING
};

class Piece {
private:
    PieceType type;
    Color color;
    bool invalid;
    int row = 0;
    int col = 0;
public:
    Piece();
    Piece(bool inv);
    Piece(Color c, PieceType t);
    Color getColor();
    PieceType getType();
    bool isInvalid();
    std::string getPieceSymbol();
    static std::string getPieceSymbol(PieceType type, Color color);
    void setPos(int r, int c);
    int getRow();
    int getCol();
    double getValue();
};
