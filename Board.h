/**
 * @file Board.h
 * @author Greg Loose (gloose)
 * @date 2022-05-04
 */

#pragma once
#include <vector>
#include <string>
#include "Piece.h"
#include "Move.h"
#include <utility>
#include "Position.h"
#include "mpi.h"

const char COL_NAMES[9] = { '?', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h' };

class Board {
private:
    std::vector<Piece> board;
    Position whiteKingPos;
    Position blackKingPos;
    int whiteCanEnPassant = 0;
    int blackCanEnPassant = 0;
    bool whiteCanCastleLeft = false;
    bool blackCanCastleLeft = false;
    bool whiteCanCastleRight = false;
    bool blackCanCastleRight = false;
    long* numCalls;
    double* reduceTime;
public:
    Board();
    Piece getPiece(int row, int col);
    Piece getPiece(Position pos);
    void setPiece(int row, int col, Piece piece);
    void initializeBoard();
    void printBoard();
    Move makeMove(Piece piece, int row, int col);
    double calculateScore();
    double evaluateMove(Move move, int depth, MPI_Comm comm, double alpha);
    Piece applyMove(Move move);
    void undoMove(Move move, Piece taken);
    std::pair<Move, double> findBestMove(int depth, Color toMove, MPI_Comm comm, double alpha);
    bool findCheck(Color toMove);
    bool isValidMove(Move move);
    std::string algebraicNotation(Move move);
    void addMove(Move move, std::vector< std::pair<double, Move> >& moves);
    bool enPassant(int row, int col, Color toMove);
    bool canCastleLeft(Color toMove);
    bool canCastleRight(Color toMove);
    void getAllMoves(Color toMove, std::vector< std::pair<double, Move> >& moves);
    int countNumMoves(Color toMove);
    Move getInputMove(Color toMove);
    long getNumCalls();
    double getReduceTime();
};