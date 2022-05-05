/**
 * @file Piece.cpp
 * @author Greg Loose (gloose)
 * @brief This file contains various operations involving the Pieces that
 * make up the game board grid. See getPieceSymbol for details on piece
 * symbols, which are used in the input files.
 * 
 * @date 2022-05-04
 */

#include "Piece.h"
#include <limits>

Piece::Piece() {
    color = NOCOLOR;
    type = NONE;
    invalid = false;
}

Piece::Piece(bool inv) {
    color = NOCOLOR;
    type = NONE;
    invalid = inv;
}

Piece::Piece(Color c, PieceType t) {
    color = c;
    type = t;
    invalid = false;
}

Color Piece::getColor() {
    return color;
}

PieceType Piece::getType() {
    return type;
}

bool Piece::isInvalid() {
    return invalid;
}

std::string Piece::getPieceSymbol() {
    return getPieceSymbol(type, color);
}

/**
 * Pieces are represented by the first letter of their name, except for
 * knights, which use the letter "N", since K is used by the king.
 * Capital letters represent white pieces, and lowercase letters represent
 * black pieces. This notation is used when printing the board to the console,
 * as well as when reading an input file.
 */
std::string Piece::getPieceSymbol(PieceType type, Color color) {
    if (color == WHITE) {
        switch (type) {
            case PAWN:
                return "P";
            case ROOK:
                return "R";
            case KNIGHT:
                return "N";
            case BISHOP:
                return "B";
            case QUEEN:
                return "Q";
            case KING:
                return "K";
            case NONE:
                return " ";
        }
    }
    else {
        switch (type) {
            case PAWN:
                return "p";
            case ROOK:
                return "r";
            case KNIGHT:
                return "n";
            case BISHOP:
                return "b";
            case QUEEN:
                return "q";
            case KING:
                return "k";
            case NONE:
                return " ";
        }
    }
}

void Piece::setPos(int r, int c) {
    row = r;
    col = c;
}

int Piece::getRow() {
    return row;
}

int Piece::getCol() {
    return col;
}

double Piece::getValue() {
    switch (type) {
        case PAWN:
            return 1;
        case ROOK:
            return 5;
        case KNIGHT:
            return 3;
        case BISHOP:
            return 3;
        case QUEEN:
            return 9;
        default:
            return 0;
    }
}