#pragma once
#include <vector>
#include <string>
#include "Piece.h"
#include "Move.h"
#include <utility>
#include "Position.h"
#include "mpi.h"

const char COL_NAMES[8] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h' };

class Board {
private:
    std::vector<Piece> board;
    Position whiteKingPos;
    Position blackKingPos;
    //int moveID;
    //bool usedAllProcs;
public:
    Board();
    Piece getPiece(int row, int col);
    void setPiece(int row, int col, Piece piece);
    void initializeBoard();
    void printBoard();
    //std::vector<Move> getPossibleMoves(Color toMove);
    Move makeMove(Piece piece, int row, int col);
    double calculateScore();
    double evaluateMove(Move move, int depth, MPI_Comm comm);
    Piece applyMove(Move move);
    void undoMove(Move move, Piece taken);
    std::pair<Move, double> findBestMove(int depth, Color toMove, MPI_Comm comm);
    bool findCheck(Color toMove);
    bool isValidMove(Move move);
    //void considerMove(Move move, int depth, double& bestValue, Move& bestMove, int procID, int nproc);
    std::string algebraicNotation(Move move);
    void addMove(Move move, std::vector<Move>& moves);
};