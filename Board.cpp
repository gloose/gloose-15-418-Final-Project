/**
 * @file Board.cpp
 * @author Greg Loose (gloose)
 * @brief This file contains the main code for my final project.
 * It can be run as follows:
 * 
 * mpirun -np X -f Y -d Z
 * 
 * Where X is an integer number of cores, Y is a file name, and Z is a
 * positive integer. If X is omitted, the default board state with all pieces
 * in their initial positions will be used.
 * 
 * The input file, if provided, should have a W or B on its first line to
 * indicate which player is to move. The following 8 lines should each be
 * 8 characters long, with each character being the symbol for the piece on
 * that square of the board, viewed from White's perspective. See Piece.cpp
 * for more details on piece symbols.
 * 
 * During a run of the program, the opponent's moves can be input in a slightly
 * simplified form of the traditional algebraic notation. This consists of 2-3
 * characters: the piece symbol (which can be omitted for a pawn), followed by
 * the column letter and row number to which it is moved. In the case of
 * ambiguous moves, the user will be prompted with a numbered list of
 * alternatives, and must input the number of the desired move.
 * 
 * @date 2022-05-04
 */

#include <vector>
#include <iostream>
#include "Board.h"
#include <stdlib.h>
#include <sstream>
#include <limits>
#include <utility>
#include <fstream>
#include <unistd.h>
#include <algorithm>
#include <math.h>

#define infty std::numeric_limits<double>::infinity()

Board::Board() {
    board.resize(WIDTH * HEIGHT);
    numCalls = new long[1];
    *numCalls = 0;
    reduceTime = new double[1];
    *reduceTime = 0;
}

/**
 * Note: when referring to positions on the board, rows and columns start
 * at 1, not 0. This is intended to map the logic more closely to the usual
 * chess notation (though to be honest it has tripped me up many times). 
 */
Piece Board::getPiece(int row, int col) {
    row--;
    col--;
    if (row < 0 || row >= HEIGHT || col < 0 || col >= WIDTH) {
        Piece piece(true);
        return piece;
    }
    return board[row * WIDTH + col];
}

Piece Board::getPiece(Position pos) {
    return getPiece(pos.row, pos.col);
}

void Board::setPiece(int row, int col, Piece piece) {
    row--;
    col--;
    board[row * WIDTH + col] = piece;
    board[row * WIDTH + col].setPos(row + 1, col + 1);
    if (piece.getType() == KING) {
        if (piece.getColor() == WHITE) {
            whiteKingPos = Position(row + 1, col + 1);
        } else {
            blackKingPos = Position(row + 1, col + 1);
        }
    }
}

void Board::initializeBoard() {
    setPiece(1, 1, Piece(WHITE, ROOK));
    setPiece(1, 2, Piece(WHITE, KNIGHT));
    setPiece(1, 3, Piece(WHITE, BISHOP));
    setPiece(1, 4, Piece(WHITE, QUEEN));
    setPiece(1, 5, Piece(WHITE, KING));
    setPiece(1, 6, Piece(WHITE, BISHOP));
    setPiece(1, 7, Piece(WHITE, KNIGHT));
    setPiece(1, 8, Piece(WHITE, ROOK));

    for (int i = 1; i <= WIDTH; i ++) {
        setPiece(2, i, Piece(WHITE, PAWN));
    }

    for (int i = 3; i <= 6; i ++) {
        for (int j = 1; j <= WIDTH; j ++) {
            setPiece(i, j, Piece(NOCOLOR, NONE));
        }
    }

    setPiece(8, 1, Piece(BLACK, ROOK));
    setPiece(8, 2, Piece(BLACK, KNIGHT));
    setPiece(8, 3, Piece(BLACK, BISHOP));
    setPiece(8, 4, Piece(BLACK, QUEEN));
    setPiece(8, 5, Piece(BLACK, KING));
    setPiece(8, 6, Piece(BLACK, BISHOP));
    setPiece(8, 7, Piece(BLACK, KNIGHT));
    setPiece(8, 8, Piece(BLACK, ROOK));

    for (int i = 1; i <= WIDTH; i ++) {
        setPiece(7, i, Piece(BLACK, PAWN));
    }

    whiteCanCastleLeft = true;
    blackCanCastleLeft = true;
    whiteCanCastleRight = true;
    blackCanCastleRight = true;
}

void Board::printBoard() {
    std::ostringstream output;
    
    for (int i = HEIGHT; i >= 1; i --) {
        output << "  ";
        for (int j = 1; j <= WIDTH; j ++) {
            output << "----";
        }
        output << "-\n";

        output << i << " ";
        for (int j = 1; j <= WIDTH; j ++) {
            output << "| ";
            output << getPiece(i, j).getPieceSymbol();
            output << " ";
        }
        output << "|\n";
    }

    output << "  ";
    for (int j = 1; j <= WIDTH; j ++) {
        output << "----";
    }
    output << "-\n";

    output << "    ";
    for (int j = 1; j <= WIDTH; j ++) {
        output << COL_NAMES[j] << "   ";
    }
    output << "\n";

    std::cout << output.str() << std::endl;
}

Move Board::makeMove(Piece piece, int row, int col) {
    return Move(piece.getRow(), piece.getCol(), row, col);
}

bool Board::findCheck(Color toMove) {
    int r = 0;
    int c = 0;
    
    if (toMove == WHITE) {
        if (!whiteKingPos.isValid()) {
            return false;
        }
        r = whiteKingPos.row;
        c = whiteKingPos.col;
    } else {
        if (!blackKingPos.isValid()) {
            return false;
        }
        r = blackKingPos.row;
        c = blackKingPos.col;
    }

    // Find pawn check
    Piece leftDiag = Piece();
    Piece rightDiag = Piece();
    if (toMove == WHITE) {
        leftDiag = getPiece(r + 1, c - 1);
        rightDiag = getPiece(r + 1, c + 1);
    } else {
        leftDiag = getPiece(r - 1, c - 1);
        rightDiag = getPiece(r - 1, c + 1);
    }
    if ((leftDiag.getColor() != toMove && leftDiag.getType() == PAWN) || (rightDiag.getColor() != toMove && rightDiag.getType() == PAWN)) {
        return true;
    }

    // Find knight check
    for (int i = -2; i <= 2; i ++) {
        for (int j = -2; j <= 2; j ++) {
            if (abs(i) + abs(j) == 3) {
                Piece p = getPiece(r + i, c + j);
                if (!p.isInvalid() && p.getColor() != toMove && p.getType() == KNIGHT) {
                    return true;
                }
            }
        }
    }

    // Find rook check
    for (int dy = -1; dy <= 1; dy ++) {
        for (int dx = -1; dx <= 1; dx ++) {
            if ((dx == 0) != (dy == 0)) {
                for (int i = 1; r + i * dy <= HEIGHT && r + i * dy >= 1 && c + i * dx <= WIDTH && c + i * dx >= 1; i ++) {
                    Piece p = getPiece(r + i * dy, c + i * dx);
                    if (p.getColor() != toMove && (p.getType() == ROOK || p.getType() == QUEEN)) {
                        return true;
                    }
                    if (p.getType() != NONE) {
                        break;
                    }
                }
            }
        }
    }

    // Find bishop check
    for (int dx = -1; dx <= 1; dx += 2) {
        for (int dy = -1; dy <= 1; dy += 2) {
            for (int i = 1; r + i * dy <= HEIGHT && r + i * dy >= 1 && c + i * dx <= WIDTH && c + i * dx >= 1; i ++) {
                Piece p = getPiece(r + i * dy, c + i * dx);
                if (p.getColor() != toMove && (p.getType() == BISHOP || p.getType() == QUEEN)) {
                    return true;
                }
                if (p.getType() != NONE) {
                    break;
                }
            }
        }
    }

    // Find king check
    for (int i = r - 1; i <= r + 1; i ++) {
        for (int j = c - 1; j <= c + 1; j ++) {
            Piece p = getPiece(i, j);
            if (!p.isInvalid() && p.getColor() != toMove && p.getType() == KING) {
                return true;
            }
        }
    }

    return false;
}

bool Board::isValidMove(Move move) {
    if (move.row1 < 1 || move.row1 > HEIGHT || move.col1 < 1 || move.col1 > WIDTH || move.row2 < 1 || move.row2 > HEIGHT || move.col2 < 1 || move.col2 > WIDTH) {
        return false;
    }
    Color toMove = getPiece(move.row1, move.col1).getColor();
    Board prevState = *this;
    Piece taken = applyMove(move);
    bool check = findCheck(toMove);
    *this = prevState;
    return !check;
}

void Board::addMove(Move move, std::vector< std::pair<double, Move> >& moves) {
    if (isValidMove(move)) {
        moves.push_back(std::pair<double, Move>(0, move));
    }
}

/**
 * An alternative comparison for move sorting for alpha-beta pruning.
 * This sort was suggested by the Cornell University site (see references in
 * report), but ultimately provided worse performance.
 */
bool comparePairs(std::pair<double, Move> a, std::pair<double, Move> b) {
    return fabs(a.first) < fabs(b.first);
}

/**
 * Sorts (score, move) pairs by decreasing score.
 */
bool comparePairsWhite(std::pair<double, Move> a, std::pair<double, Move> b) {
    return a.first > b.first;
}

/**
 * Sorts (score, move) pairs by increasing score.
 */
bool comparePairsBlack(std::pair<double, Move> a, std::pair<double, Move> b) {
    return a.first < b.first;
}

/**
 * Can the piece at this position be taken en passant?
 */
bool Board::enPassant(int row, int col, Color toMove) {
    if (!Position(row, col).isValid()) {
        return false;
    }

    Color toTake;
    if (toMove == WHITE) {
        if (row != 5) {
            return false;
        }
        toTake = BLACK;
    } else {
        if (row != 4) {
            return false;
        }
        toTake = WHITE;
    }

    Piece piece = getPiece(row, col);
    return piece.getColor() == toTake && ((toMove == WHITE && whiteCanEnPassant == col) || (toMove == BLACK && blackCanEnPassant == col));
}

bool Board::canCastleLeft(Color toMove) {
    Piece king;
    if (toMove == WHITE) {
        if (!whiteCanCastleLeft) {
            return false;
        }
        king = getPiece(whiteKingPos);
    } else {
        if (!blackCanCastleLeft) {
            return false;
        }
        king = getPiece(blackKingPos);
    }

    int kingRow = king.getRow();
    int kingCol = king.getCol();

    Piece rook = getPiece(kingRow, 1);

    for (int i = kingCol - 1; i > rook.getCol(); i --) {
        if (getPiece(kingRow, i).getType() != NONE) {
            return false;
        }
    }

    for (int i = kingCol; i >= kingCol - 2; i --) {
        if (toMove == WHITE) {
            whiteKingPos.col = i;
        } else {
            blackKingPos.col = i;
        }
        if (findCheck(toMove)) {
            if (toMove == WHITE) {
                whiteKingPos.col = kingCol;
            } else {
                blackKingPos.col = kingCol;
            }
            return false;
        }
    }
    if (toMove == WHITE) {
        whiteKingPos.col = kingCol;
    } else {
        blackKingPos.col = kingCol;
    }

    return true;
}

bool Board::canCastleRight(Color toMove) {
    Piece king;
    if (toMove == WHITE) {
        if (!whiteCanCastleRight) {
            return false;
        }
        king = getPiece(whiteKingPos);
    } else {
        if (!blackCanCastleRight) {
            return false;
        }
        king = getPiece(blackKingPos);
    }

    int kingRow = king.getRow();
    int kingCol = king.getCol();

    Piece rook = getPiece(kingRow, 8);

    for (int i = kingCol + 1; i < rook.getCol(); i ++) {
        if (getPiece(kingRow, i).getType() != NONE) {
            return false;
        }
    }

    for (int i = kingCol; i <= kingCol + 2; i ++) {
        if (toMove == WHITE) {
            whiteKingPos.col = i;
        } else {
            blackKingPos.col = i;
        }
        if (findCheck(toMove)) {
            if (toMove == WHITE) {
                whiteKingPos.col = kingCol;
            } else {
                blackKingPos.col = kingCol;
            }
            return false;
        }
    }
    if (toMove == WHITE) {
        whiteKingPos.col = kingCol;
    } else {
        blackKingPos.col = kingCol;
    }

    return true;
}

void Board::getAllMoves(Color toMove, std::vector< std::pair<double, Move> >& moves) {
    for (int r = 1; r <= HEIGHT; r ++) {
        for (int c = 1; c <= WIDTH; c ++) {
            Piece piece = getPiece(r, c);
            PieceType type = piece.getType();
            Color color = piece.getColor();
            
            if (color != toMove) {
                continue;
            }

            switch (type) {
                case NONE:
                    break;
                case PAWN:
                    if (color == WHITE) {
                        if (r < HEIGHT && getPiece(r + 1, c).getType() == NONE) {
                            addMove(makeMove(piece, r + 1, c), moves);
                        }
                        if (r == 2 && getPiece(r + 1, c).getType() == NONE && getPiece(r + 2, c).getType() == NONE) {
                            addMove(makeMove(piece, r + 2, c), moves);
                        }
                        
                        Piece diagLeft = getPiece(r + 1, c - 1);
                        Piece diagRight = getPiece(r + 1, c + 1);
                        if (!diagLeft.isInvalid() && diagLeft.getColor() == BLACK) {
                            addMove(makeMove(piece, r + 1, c - 1), moves);
                        }
                        if (!diagRight.isInvalid() && diagRight.getColor() == BLACK) {
                            addMove(makeMove(piece, r + 1, c + 1), moves);
                        }
                        
                        if (enPassant(r, c - 1, WHITE)) {
                            addMove(makeMove(piece, r + 1, c - 1), moves);
                        }
                        if (enPassant(r, c + 1, WHITE)) {
                            addMove(makeMove(piece, r + 1, c + 1), moves);
                        }
                    } else {
                        if (r > 1 && getPiece(r - 1, c).getType() == NONE) {
                            addMove(makeMove(piece, r - 1, c), moves);
                        }
                        if (r == 7 && getPiece(r - 1, c).getType() == NONE && getPiece(r - 2, c).getType() == NONE) {
                            addMove(makeMove(piece, r - 2, c), moves);
                        }
                        
                        Piece diagLeft = getPiece(r - 1, c - 1);
                        Piece diagRight = getPiece(r - 1, c + 1);
                        if (!diagLeft.isInvalid() && diagLeft.getColor() == WHITE) {
                            addMove(makeMove(piece, r - 1, c - 1), moves);
                        }
                        if (!diagRight.isInvalid() && diagRight.getColor() == WHITE) {
                            addMove(makeMove(piece, r - 1, c + 1), moves);
                        }

                        if (enPassant(r, c - 1, BLACK)) {
                            addMove(makeMove(piece, r - 1, c - 1), moves);
                        }
                        if (enPassant(r, c + 1, BLACK)) {
                            addMove(makeMove(piece, r - 1, c + 1), moves);
                        }
                    }
                    break;
                case ROOK:
                    for (int dy = -1; dy <= 1; dy ++) {
                        for (int dx = -1; dx <= 1; dx ++) {
                            if ((dx == 0) != (dy == 0)) {
                                for (int i = 1; r + i * dy <= HEIGHT && r + i * dy >= 1 && c + i * dx <= WIDTH && c + i * dx >= 1; i ++) {
                                    Piece p = getPiece(r + i * dy, c + i * dx);
                                    if (p.getColor() != color) {
                                        addMove(makeMove(piece, r + i * dy, c + i * dx), moves);
                                    }
                                    if (p.getType() != NONE) {
                                        break;
                                    }
                                }
                            }
                        }
                    }
                    break;
                case KNIGHT:
                    for (int i = -2; i <= 2; i ++) {
                        for (int j = -2; j <= 2; j ++) {
                            if (abs(i) + abs(j) == 3) {
                                Piece p = getPiece(r + i, c + j);
                                if (!p.isInvalid() && p.getColor() != color) {
                                    addMove(makeMove(piece, r + i, c + j), moves);
                                }
                            }
                        }
                    }
                    break;
                case BISHOP:
                    for (int dx = -1; dx <= 1; dx += 2) {
                        for (int dy = -1; dy <= 1; dy += 2) {
                            for (int i = 1; r + i * dy <= HEIGHT && r + i * dy >= 1 && c + i * dx <= WIDTH && c + i * dx >= 1; i ++) {
                                Piece p = getPiece(r + i * dy, c + i * dx);
                                if (p.getColor() != color) {
                                    addMove(makeMove(piece, r + i * dy, c + i * dx), moves);
                                }
                                if (p.getType() != NONE) {
                                    break;
                                }
                            }
                        }
                    }
                    break;
                case QUEEN:
                    for (int dy = -1; dy <= 1; dy ++) {
                        for (int dx = -1; dx <= 1; dx ++) {
                            if ((dx == 0) != (dy == 0)) {
                                for (int i = 1; r + i * dy <= HEIGHT && r + i * dy >= 1 && c + i * dx <= WIDTH && c + i * dx >= 1; i ++) {
                                    Piece p = getPiece(r + i * dy, c + i * dx);
                                    if (p.getColor() != color) {
                                        addMove(makeMove(piece, r + i * dy, c + i * dx), moves);
                                    }
                                    if (p.getType() != NONE) {
                                        break;
                                    }
                                }
                            }
                        }
                    }

                    for (int dx = -1; dx <= 1; dx += 2) {
                        for (int dy = -1; dy <= 1; dy += 2) {
                            for (int i = 1; r + i * dy <= HEIGHT && r + i * dy >= 1 && c + i * dx <= WIDTH && c + i * dx >= 1; i ++) {
                                Piece p = getPiece(r + i * dy, c + i * dx);
                                if (p.getColor() != color) {
                                    addMove(makeMove(piece, r + i * dy, c + i * dx), moves);
                                }
                                if (p.getType() != NONE) {
                                    break;
                                }
                            }
                        }
                    }

                    break;
                case KING:
                    for (int i = r - 1; i <= r + 1; i ++) {
                        for (int j = c - 1; j <= c + 1; j ++) {
                            Piece p = getPiece(i, j);
                            if (!p.isInvalid() && p.getColor() != color) {
                                addMove(makeMove(piece, i, j), moves);
                            }
                        }
                    }

                    if (canCastleLeft(toMove)) {
                        addMove(makeMove(piece, r, c - 2), moves);
                    }
                    if (canCastleRight(toMove)) {
                        addMove(makeMove(piece, r, c + 2), moves);
                    }
                    break;
            }
        }
    }
}

/**
 * This is basically the same as getAllMoves, but is more efficient as it
 * doesn't create the actual array. There is obviously a lot of code
 * duplication here, but I wasn't really going for pretty code in this project.
 */
int Board::countNumMoves(Color toMove) {
    int numMoves = 0;
    for (int r = 1; r <= HEIGHT; r ++) {
        for (int c = 1; c <= WIDTH; c ++) {
            Piece piece = getPiece(r, c);
            PieceType type = piece.getType();
            Color color = piece.getColor();
            
            if (color != toMove) {
                continue;
            }

            switch (type) {
                case NONE:
                    break;
                case PAWN:
                    if (color == WHITE) {
                        if (r < HEIGHT && getPiece(r + 1, c).getType() == NONE) {
                            numMoves++;
                        }
                        if (r == 2 && getPiece(r + 1, c).getType() == NONE && getPiece(r + 2, c).getType() == NONE) {
                            numMoves++;
                        }
                        
                        Piece diagLeft = getPiece(r + 1, c - 1);
                        Piece diagRight = getPiece(r + 1, c + 1);
                        if (!diagLeft.isInvalid() && diagLeft.getColor() == BLACK) {
                            numMoves++;
                        }
                        if (!diagRight.isInvalid() && diagRight.getColor() == BLACK) {
                            numMoves++;
                        }
                        
                        if (enPassant(r, c - 1, WHITE)) {
                            numMoves++;
                        }
                        if (enPassant(r, c + 1, WHITE)) {
                            numMoves++;
                        }
                    } else {
                        if (r > 1 && getPiece(r - 1, c).getType() == NONE) {
                            numMoves++;
                        }
                        if (r == 7 && getPiece(r - 1, c).getType() == NONE && getPiece(r - 2, c).getType() == NONE) {
                            numMoves++;
                        }
                        
                        Piece diagLeft = getPiece(r - 1, c - 1);
                        Piece diagRight = getPiece(r - 1, c + 1);
                        if (!diagLeft.isInvalid() && diagLeft.getColor() == WHITE) {
                            numMoves++;
                        }
                        if (!diagRight.isInvalid() && diagRight.getColor() == WHITE) {
                            numMoves++;
                        }

                        if (enPassant(r, c - 1, BLACK)) {
                            numMoves++;
                        }
                        if (enPassant(r, c + 1, BLACK)) {
                            numMoves++;
                        }
                    }
                    break;
                case ROOK:
                    for (int dy = -1; dy <= 1; dy ++) {
                        for (int dx = -1; dx <= 1; dx ++) {
                            if ((dx == 0) != (dy == 0)) {
                                for (int i = 1; r + i * dy <= HEIGHT && r + i * dy >= 1 && c + i * dx <= WIDTH && c + i * dx >= 1; i ++) {
                                    Piece p = getPiece(r + i * dy, c + i * dx);
                                    if (p.getColor() != color) {
                                        numMoves++;
                                    }
                                    if (p.getType() != NONE) {
                                        break;
                                    }
                                }
                            }
                        }
                    }
                    break;
                case KNIGHT:
                    for (int i = -2; i <= 2; i ++) {
                        for (int j = -2; j <= 2; j ++) {
                            if (abs(i) + abs(j) == 3) {
                                Piece p = getPiece(r + i, c + j);
                                if (!p.isInvalid() && p.getColor() != color) {
                                    numMoves++;
                                }
                            }
                        }
                    }
                    break;
                case BISHOP:
                    for (int dx = -1; dx <= 1; dx += 2) {
                        for (int dy = -1; dy <= 1; dy += 2) {
                            for (int i = 1; r + i * dy <= HEIGHT && r + i * dy >= 1 && c + i * dx <= WIDTH && c + i * dx >= 1; i ++) {
                                Piece p = getPiece(r + i * dy, c + i * dx);
                                if (p.getColor() != color) {
                                    numMoves++;
                                }
                                if (p.getType() != NONE) {
                                    break;
                                }
                            }
                        }
                    }
                    break;
                case QUEEN:
                    for (int dy = -1; dy <= 1; dy ++) {
                        for (int dx = -1; dx <= 1; dx ++) {
                            if ((dx == 0) != (dy == 0)) {
                                for (int i = 1; r + i * dy <= HEIGHT && r + i * dy >= 1 && c + i * dx <= WIDTH && c + i * dx >= 1; i ++) {
                                    Piece p = getPiece(r + i * dy, c + i * dx);
                                    if (p.getColor() != color) {
                                        numMoves++;
                                    }
                                    if (p.getType() != NONE) {
                                        break;
                                    }
                                }
                            }
                        }
                    }

                    for (int dx = -1; dx <= 1; dx += 2) {
                        for (int dy = -1; dy <= 1; dy += 2) {
                            for (int i = 1; r + i * dy <= HEIGHT && r + i * dy >= 1 && c + i * dx <= WIDTH && c + i * dx >= 1; i ++) {
                                Piece p = getPiece(r + i * dy, c + i * dx);
                                if (p.getColor() != color) {
                                    numMoves++;
                                }
                                if (p.getType() != NONE) {
                                    break;
                                }
                            }
                        }
                    }

                    break;
                case KING:
                    for (int i = r - 1; i <= r + 1; i ++) {
                        for (int j = c - 1; j <= c + 1; j ++) {
                            Piece p = getPiece(i, j);
                            if (!p.isInvalid() && p.getColor() != color) {
                                numMoves++;
                            }
                        }
                    }

                    if (canCastleLeft(toMove)) {
                        numMoves++;
                    }
                    if (canCastleRight(toMove)) {
                        numMoves++;
                    }
                    break;
            }
        }
    }

    return numMoves;
}

std::pair<Move, double> Board::findBestMove(int depth, Color toMove, MPI_Comm comm, double alpha) {
    *numCalls = *numCalls + 1;

    int procID;
    int nproc;

    MPI_Comm_rank(comm, &procID);
    MPI_Comm_size(comm, &nproc);

    double bestValue;
    if (toMove == WHITE) {
        bestValue = -infty;
    } else {
        bestValue = infty;
    }
    Move bestMove;

    std::vector< std::pair<double, Move> > moves;
    getAllMoves(toMove, moves);

    if (moves.size() == 0) {
        if (!findCheck(toMove)) {
            return std::pair<Move, double>(bestMove, 0);
        }
        if (toMove == BLACK) {
            return std::pair<Move, double>(bestMove, 1000 + depth);
        } else {
            return std::pair<Move, double>(bestMove, -1000 - depth);
        }
    }

    if (nproc <= moves.size()) {
        if (depth > 1) {
            for (int i = 0; i < moves.size(); i ++) {
                moves[i].first = evaluateMove(moves[i].second, 1, MPI_COMM_WORLD, 0);
            }
            
            if (toMove == WHITE) {
                std::sort(moves.begin(), moves.end(), comparePairsWhite);
            } else {
                std::sort(moves.begin(), moves.end(), comparePairsBlack);
            }
        }

        MPI_Comm newcomm;
        MPI_Comm_split(comm, procID, procID, &newcomm);

        for (int i = procID; i < moves.size(); i += nproc) {
            Move move = moves[i].second;

            double value = evaluateMove(move, depth, newcomm, bestValue);

            if (((toMove == BLACK && value <= alpha) || (toMove == WHITE && value >= alpha)) && bestMove.row1 != 0) {
                if (toMove == WHITE) {
                    bestValue = infty;
                } else {
                    bestValue = -infty;
                }
                bestMove = move;
                break;
            }

            if ((toMove == WHITE && value >= bestValue) || (toMove == BLACK && value <= bestValue)) {
                bestValue = value;
                bestMove = move;
            }
        }

        MPI_Comm_free(&newcomm);

        std::pair<double, int> sendBest(bestValue, bestMove.compress());
        std::pair<double, int> globalBest;

        double startTime = MPI_Wtime();
        if (toMove == WHITE) {
            MPI_Allreduce(&sendBest, &globalBest, 1, MPI_DOUBLE_INT, MPI_MAXLOC, comm);
        } else {
            MPI_Allreduce(&sendBest, &globalBest, 1, MPI_DOUBLE_INT, MPI_MINLOC, comm);
        }
        *reduceTime = *reduceTime + MPI_Wtime() - startTime;

        return std::pair<Move, double>(Move(globalBest.second), globalBest.first);
    } else {
        int procsPerMove = (nproc + moves.size() - 1) / moves.size();
        int remainder = nproc % moves.size();
        if (remainder == 0) {
            remainder = nproc;
        }

        int moveIndex;
        if (procID < remainder) {
            moveIndex = procID / procsPerMove;
        } else {
            moveIndex = remainder + (procID - remainder * procsPerMove) / (procsPerMove - 1);
        }

        Move move = moves[moveIndex].second;
        MPI_Comm newcomm;
        MPI_Comm_split(comm, moveIndex, procID, &newcomm);
        bestValue = evaluateMove(move, depth, newcomm, bestValue);
        bestMove = move;
        MPI_Comm_free(&newcomm);

        std::pair<double, int> sendBest(bestValue, bestMove.compress());
        std::pair<double, int> globalBest;

        double startTime = MPI_Wtime();
        if (toMove == WHITE) {
            MPI_Allreduce(&sendBest, &globalBest, 1, MPI_DOUBLE_INT, MPI_MAXLOC, comm);
        } else {
            MPI_Allreduce(&sendBest, &globalBest, 1, MPI_DOUBLE_INT, MPI_MINLOC, comm);
        }
        *reduceTime = *reduceTime + MPI_Wtime() - startTime;

        return std::pair<Move, double>(Move(globalBest.second), globalBest.first);
    }
}

double Board::calculateScore() {
    double score = 0;
    for (int i = 1; i <= HEIGHT; i ++) {
        for (int j = 1; j <= WIDTH; j ++) {
            Piece piece = getPiece(i, j);
            double value = piece.getValue();
            if (piece.getColor() == WHITE) {
                score += value;
            } else {
                score -= value;
            }
        }
    }

    score += countNumMoves(WHITE) * 0.01;
    score -= countNumMoves(BLACK) * 0.01;

    return score;
}

Piece Board::applyMove(Move move) {
    Piece moved = getPiece(move.row1, move.col1);
    Piece taken = getPiece(move.row2, move.col2);

    whiteCanEnPassant = 0;
    blackCanEnPassant = 0;

    if (moved.getType() == KING) {
        if (moved.getColor() == WHITE) {
            whiteCanCastleLeft = false;
            whiteCanCastleRight = false;
        } else {
            blackCanCastleLeft = false;
            blackCanCastleRight = false;
        }
    }

    if (whiteCanCastleLeft && (moved.getRow() == 1 && moved.getCol() == 1) || (taken.getRow() == 1 && taken.getCol() == 1)) {
        whiteCanCastleLeft = false;
    }
    if (whiteCanCastleRight && (moved.getRow() == 1 && moved.getCol() == 8) || (taken.getRow() == 1 && taken.getCol() == 8)) {
        whiteCanCastleLeft = false;
    }
    if (blackCanCastleLeft && (moved.getRow() == 8 && moved.getCol() == 1) || (taken.getRow() == 8 && taken.getCol() == 1)) {
        blackCanCastleLeft = false;
    }
    if (blackCanCastleRight && (moved.getRow() == 8 && moved.getCol() == 8) || (taken.getRow() == 8 && taken.getCol() == 8)) {
        blackCanCastleLeft = false;
    }

    if (moved.getType() == KING && abs(move.col2 - move.col1) == 2) {
        if (move.col2 == move.col1 - 2) {
            setPiece(move.row1, move.col1 - 1, getPiece(move.row1, 1));
            setPiece(move.row1, 1, Piece());
        } else {
            setPiece(move.row1, move.col1 + 1, getPiece(move.row1, 8));
            setPiece(move.row1, 8, Piece());
        }
    }

    if (moved.getType() == PAWN && move.col2 != move.col1 && taken.getType() == NONE) {
        taken = getPiece(move.row1, move.col2);
    }
    
    if (moved.getType() == PAWN && abs(move.row1 - move.row2) == 2) {
        if (moved.getColor() == BLACK) {
            whiteCanEnPassant = move.col1;
        } else {
            blackCanEnPassant = move.col1;
        }
    }

    setPiece(move.row1, move.col1, Piece());

    setPiece(taken.getRow(), taken.getCol(), Piece());

    // This doesn't allow underpromotion, but it'll work like 99% of the time and it's not worth the extra complexity.
    if (moved.getType() == PAWN && ( (moved.getColor() == WHITE && move.row2 == 8) || (moved.getColor() == BLACK && move.row2 == 1) )) {
        setPiece(move.row2, move.col2, Piece(moved.getColor(), QUEEN));
    } else {
        setPiece(move.row2, move.col2, moved);
    }    

    return taken;
}

double Board::evaluateMove(Move move, int depth, MPI_Comm comm, double alpha) {
    double value;
    Board prevState = *this;

    Piece taken = applyMove(move);

    if (depth == 1) {
        value = calculateScore();
    } else {
        Color toMove = getPiece(move.row2, move.col2).getColor();
        Color other;
        if (toMove == WHITE) {
            other = BLACK;
        } else {
            other = WHITE;
        }

        value = findBestMove(depth - 1, other, comm, alpha).second;
    }

    *this = prevState;

    return value;
}

std::string Board::algebraicNotation(Move move) {
    Piece piece = getPiece(move.row1, move.col1);
    std::stringstream ret;
    ret << piece.getPieceSymbol() << COL_NAMES[move.col1] << move.row1 << COL_NAMES[move.col2] << move.row2;
    return ret.str();
}

Move Board::getInputMove(Color toMove) {
    std::vector< std::pair<double, Move> > moves;
    getAllMoves(toMove, moves);

    if (moves.size() == 0) {
        if (findCheck(toMove)) {
            std::cout << "Checkmate. I win!" << std::endl;
        } else {
            std::cout << "Stalemate!" << std::endl;
        }
        return Move();
    }

    std::cout << "Enter the opponent's move" << std::endl;
    
    while (true) {
        std::string alg;
        std::cin >> alg;

        if (alg.size() < 2 || alg.size() > 3) {
            std::cout << "Invalid move" << std::endl;
            continue;
        }

        PieceType type;
        if (alg.size() == 2) {
            alg = 'p' + alg;
        }

        switch(alg.at(0)) {
            case 'p':
            case 'P':
                type = PAWN;
                break;
            case 'r':
            case 'R':
                type = ROOK;
                break;
            case 'n':
            case 'N':
                type = KNIGHT;
                break;
            case 'b':
            case 'B':
                type = BISHOP;
                break;
            case 'q':
            case 'Q':
                type = QUEEN;
                break;
            case 'k':
            case 'K':
                type = KING;
                break;
            default:
                std::cout << "Invalid piece type" << std::endl;
                continue;
        }

        int row = (int)alg.at(2) - '0';
        if (row < 1 || row > 8) {
            std::cout << "Invalid position" << std::endl;
            continue;
        }

        int col;
        switch(alg.at(1)) {
            case 'a':
            case 'A':
                col = 1;
                break;
            case 'b':
            case 'B':
                col = 2;
                break;
            case 'c':
            case 'C':
                col = 3;
                break;
            case 'd':
            case 'D':
                col = 4;
                break;
            case 'e':
            case 'E':
                col = 5;
                break;
            case 'f':
            case 'F':
                col = 6;
                break;
            case 'g':
            case 'G':
                col = 7;
                break;
            case 'h':
            case 'H':
                col = 8;
                break;
            default:
                std::cout << "Invalid position" << std::endl;
                continue;
        }

        std::vector<Move> valid;
        for (int i = 0; i < moves.size(); i ++) {
            Move move = moves[i].second;
            Piece piece = getPiece(move.row1, move.col1);
            if (piece.getType() == type && piece.getColor() == toMove && move.row2 == row && move.col2 == col) {
                valid.push_back(move);
            }
        }

        if (valid.size() == 1) {
            return valid[0];
        } else if (valid.size() > 1) {
            std::cout << "Ambiguous move. Which piece would you like to move?" << std::endl;
            for (int i = 0; i < valid.size(); i ++) {
                std::cout << "(" << i << ") " << COL_NAMES[valid[i].col1] << valid[i].row1 << " " << std::endl;
            }

            while (true) {
                std::string input;
                std::cin >> input;
                int index = input.at(0) - '0';
                if (index >= 0 && index < valid.size()) {
                    return valid[index];
                }
            }
        }

        std::cout << "Invalid move" << std::endl;
    }
}

long Board::getNumCalls() {
    return *numCalls;
}

double Board::getReduceTime() {
    return *reduceTime;
}

int main(int argc, char *argv[]) {
    Color toMove = WHITE;
    int depth = 1;
    char* inputFilename = NULL;
    int opt = 0;
    Color playing = WHITE;

    MPI_Init(&argc, &argv);

    do {
        opt = getopt(argc, argv, "f:d:");
        switch (opt) {
            case 'f':
                inputFilename = optarg;
                break;
            case 'd':
                depth = atoi(optarg);
                break;
        }
    } while (opt != -1);

    Board* board = new Board();

    if (inputFilename != NULL) {
        MPI_File input;
        MPI_File_open(MPI_COMM_WORLD, inputFilename, MPI_MODE_RDONLY, MPI_INFO_NULL, &input);

        char line[WIDTH + 1];
        MPI_File_read(input, line, 2, MPI_CHAR, MPI_STATUS_IGNORE);
        
        if (line[0] == 'W' || line[0] == 'w') {
            toMove = WHITE;
        } else if (line[0] == 'B' || line[0] == 'b') {
            toMove = BLACK;
        } else {
            std::cout << "First line of input file must be W or B" << std::endl;
            return 1;
        }

        playing = toMove;
        
        for (int i = HEIGHT; i >= 1; i --) {
            MPI_File_read(input, line, WIDTH + 1, MPI_CHAR, MPI_STATUS_IGNORE);
            for (int j = 1; j <= WIDTH; j ++) {
                Piece piece;
                switch (line[j - 1]) {
                    case 'P':
                        piece = Piece(WHITE, PAWN);
                        break;
                    case 'p':
                        piece = Piece(BLACK, PAWN);
                        break;
                    case 'R':
                        piece = Piece(WHITE, ROOK);
                        break;
                    case 'r':
                        piece = Piece(BLACK, ROOK);
                        break;
                    case 'N':
                        piece = Piece(WHITE, KNIGHT);
                        break;
                    case 'n':
                        piece = Piece(BLACK, KNIGHT);
                        break;
                    case 'B':
                        piece = Piece(WHITE, BISHOP);
                        break;
                    case 'b':
                        piece = Piece(BLACK, BISHOP);
                        break;
                    case 'Q':
                        piece = Piece(WHITE, QUEEN);
                        break;
                    case 'q':
                        piece = Piece(BLACK, QUEEN);
                        break;
                    case 'K':
                        piece = Piece(WHITE, KING);
                        break;
                    case 'k':
                        piece = Piece(BLACK, KING);
                        break;
                    case ' ':
                        piece = Piece(NOCOLOR, NONE);
                        break;
                }
                board->setPiece(i, j, piece);
            }
        }
    } else {
        board->initializeBoard();
    }

    int procID;
    int nproc;

    MPI_Comm_rank(MPI_COMM_WORLD, &procID);
    MPI_Comm_size(MPI_COMM_WORLD, &nproc);

    while (true) {
        if (procID == 0) {
            board->printBoard();
        }

        if (toMove == playing) {
            double alpha;
            if (toMove == WHITE) {
                alpha = infty;
            } else {
                alpha = -infty;
            }

            double startTime = MPI_Wtime();

            std::pair<Move, double> best = board->findBestMove(depth, toMove, MPI_COMM_WORLD, alpha);

            double endTime = MPI_Wtime();

            // Uncomment to print various diagnostic data
            /*
            if (procID == 0) {
                std::cout << "Elapsed time for proc " << procID << ": " << endTime - startTime << std::endl;
            }

            long numCalls = board->getNumCalls();
            std::cout << "Number of calls for proc " << procID << ": " << numCalls << std::endl;

            std::cout << "Reduce time for proc " << procID << " = " << board->getReduceTime() << std::endl;

            long globalNumCalls;
            MPI_Allreduce(&numCalls, &globalNumCalls, 1, MPI_LONG, MPI_SUM, MPI_COMM_WORLD);
            if (procID == 0) {
                std::cout << "Total number of calls is " << globalNumCalls << std::endl;
            }
            */

            std::pair<double, int> sendBest(best.second, best.first.compress());
            std::pair<double, int> globalBest;

            if (toMove == WHITE) {
                MPI_Allreduce(&sendBest, &globalBest, 1, MPI_DOUBLE_INT, MPI_MAXLOC, MPI_COMM_WORLD);
            } else {
                MPI_Allreduce(&sendBest, &globalBest, 1, MPI_DOUBLE_INT, MPI_MINLOC, MPI_COMM_WORLD);
            }

            if (procID == 0) {
                if (globalBest.second == 0) {
                    if (board->findCheck(toMove)) {
                        std::cout << "Checkmate. I lose!" << std::endl;
                    } else {
                        std::cout << "Stalemate!" << std::endl;
                    }
                } else {
                    std::cout << "Best move: " << board->algebraicNotation(Move(globalBest.second)) << ", " << globalBest.first << std::endl;
                }
            }

            if (globalBest.second == 0) {
                break;
            }

            board->applyMove(Move(globalBest.second));
        } else {
            int cmove;
            if (procID == 0) {
                Move move = board->getInputMove(toMove);
                cmove = move.compress();
                if (std::cin.fail()) {
                    cmove = 0;
                }
            }
            
            MPI_Bcast(&cmove, 1, MPI_INT, 0, MPI_COMM_WORLD);
            if (cmove == 0) {
                break;;
            }
            Move move = Move(cmove);
            board->applyMove(move);
        }
        
        if (toMove == WHITE) {
            toMove = BLACK;
        } else {
            toMove = WHITE;
        }
    }

    MPI_Finalize();
}