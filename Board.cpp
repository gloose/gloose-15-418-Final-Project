#include <vector>
#include <iostream>
#include "Board.h"
#include <stdlib.h>
#include <sstream>
#include <limits>
#include <utility>
#include <fstream>
#include <unistd.h>

Board::Board() {
    initializeBoard();
}

Piece Board::getPiece(int row, int col) {
    row--;
    col--;
    if (row < 0 || row >= HEIGHT || col < 0 || col >= WIDTH) {
        Piece piece(true);
        return piece;
    }
    return board[row * WIDTH + col];
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
    board.resize(WIDTH * HEIGHT);

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
        output << COL_NAMES[j - 1] << "   ";
    }
    output << "\n";

    std::cout << output.str() << std::endl;
}

Move Board::makeMove(Piece piece, int row, int col) {
    return Move(piece.getRow(), piece.getCol(), row, col);
}

bool Board::findCheck(Color toMove) {
    int r, c;
    
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
    Piece leftDiag, rightDiag;
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

    return false;
}

bool Board::isValidMove(Move move) {
    if (move.row1 < 1 || move.row1 > HEIGHT || move.col1 < 1 || move.col1 > WIDTH || move.row2 < 1 || move.row2 > HEIGHT || move.col2 < 1 || move.col2 > WIDTH) {
        return false;
    }
    Color toMove = getPiece(move.row1, move.col1).getColor();
    Piece taken = applyMove(move);
    bool check = findCheck(toMove);
    undoMove(move, taken);
    return !check;
}

void Board::addMove(Move move, std::vector<Move>& moves) {
    if (isValidMove(move)) {
        moves.push_back(move);
    }
}

std::pair<Move, double> Board::findBestMove(int depth, Color toMove, MPI_Comm comm) {
    int procID;
    int nproc;

    MPI_Comm_rank(comm, &procID);
    MPI_Comm_size(comm, &nproc);

    double bestValue;
    if (toMove == WHITE) {
        bestValue = -std::numeric_limits<double>::infinity();
    } else {
        bestValue = std::numeric_limits<double>::infinity();
    }
    Move bestMove;

    // TODO: is there any chance you could have each processor do a separate set of moves, then allgather?

    std::vector<Move> moves;

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
                    // TODO: promotion, en passant
                    if (color == WHITE) {
                        Piece diagLeft = getPiece(r + 1, c - 1);
                        Piece diagRight = getPiece(r + 1, c + 1);
                        if (r < HEIGHT && getPiece(r + 1, c).getType() == NONE) {
                            addMove(makeMove(piece, r + 1, c), moves);
                        }
                        if (r == 2 && getPiece(r + 1, c).getType() == NONE && getPiece(r + 2, c).getType() == NONE) {
                            addMove(makeMove(piece, r + 2, c), moves);
                        }
                        if (!diagLeft.isInvalid() && diagLeft.getColor() == BLACK) {
                            addMove(makeMove(piece, r + 1, c - 1), moves);
                        }
                        if (!diagRight.isInvalid() && diagRight.getColor() == BLACK) {
                            addMove(makeMove(piece, r + 1, c + 1), moves);
                        }
                    } else {
                        Piece diagLeft = getPiece(r - 1, c - 1);
                        Piece diagRight = getPiece(r - 1, c + 1);
                        if (r > 1 && getPiece(r - 1, c).getType() == NONE) {
                            addMove(makeMove(piece, r - 1, c), moves);
                        }
                        if (r == 7 && getPiece(r - 1, c).getType() == NONE && getPiece(r - 2, c).getType() == NONE) {
                            addMove(makeMove(piece, r - 2, c), moves);
                        }
                        if (!diagLeft.isInvalid() && diagLeft.getColor() == WHITE) {
                            addMove(makeMove(piece, r - 1, c - 1), moves);
                        }
                        if (!diagRight.isInvalid() && diagRight.getColor() == WHITE) {
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
                    // TODO: castling
                    for (int i = r - 1; i <= r + 1; i ++) {
                        for (int j = c - 1; j <= c + 1; j ++) {
                            Piece p = getPiece(i, j);
                            if (!p.isInvalid() && p.getColor() != color) {
                                addMove(makeMove(piece, i, j), moves);
                            }
                        }
                    }
                    break;
            }
        }
    }

    if (moves.size() == 0) {
        return std::pair<Move, double>(bestMove, bestValue);
    }

    if (nproc <= moves.size()) {
        MPI_Comm newcomm;
        MPI_Comm_split(comm, procID, procID, &newcomm);

        for (int i = procID; i < moves.size(); i += nproc) {
            Move move = moves[i];

            double value = evaluateMove(move, depth, newcomm);

            if ((toMove == WHITE && value > bestValue) || (toMove == BLACK && value < bestValue)) {
                bestValue = value;
                bestMove = move;
            }
        }

        MPI_Comm_free(&newcomm);

        std::pair<double, int> sendBest(bestValue, bestMove.compress());
        std::pair<double, int> globalBest;

        if (toMove == WHITE) {
            MPI_Allreduce(&sendBest, &globalBest, 1, MPI_DOUBLE_INT, MPI_MAXLOC, comm);
        } else {
            MPI_Allreduce(&sendBest, &globalBest, 1, MPI_DOUBLE_INT, MPI_MINLOC, comm);
        }

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

        Move move = moves[moveIndex];
        MPI_Comm newcomm;
        MPI_Comm_split(comm, moveIndex, procID, &newcomm);
        bestValue = evaluateMove(move, depth, newcomm);
        bestMove = move;
        MPI_Comm_free(&newcomm);

        std::pair<double, int> sendBest(bestValue, bestMove.compress());
        std::pair<double, int> globalBest;

        if (toMove == WHITE) {
            MPI_Allreduce(&sendBest, &globalBest, 1, MPI_DOUBLE_INT, MPI_MAXLOC, comm);
        } else {
            MPI_Allreduce(&sendBest, &globalBest, 1, MPI_DOUBLE_INT, MPI_MINLOC, comm);
        }

        return std::pair<Move, double>(Move(globalBest.second), globalBest.first);
    }
}

double Board::calculateScore() {
    // TODO: speed this up somehow? Could maybe just use the changes from the previous state.
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

    return score;
}

Piece Board::applyMove(Move move) {
    Piece taken = getPiece(move.row2, move.col2);
    setPiece(move.row2, move.col2, getPiece(move.row1, move.col1));
    setPiece(move.row1, move.col1, Piece());
    return taken;
}

void Board::undoMove(Move move, Piece taken) {
    setPiece(move.row1, move.col1, getPiece(move.row2, move.col2));
    setPiece(move.row2, move.col2, taken);
}

double Board::evaluateMove(Move move, int depth, MPI_Comm comm) {
    double value;
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

        value = findBestMove(depth - 1, other, comm).second;
    }

    undoMove(move, taken);
    return value;
}

std::string Board::algebraicNotation(Move move) {
    Piece piece = getPiece(move.row1, move.col1);
    std::stringstream ret;
    ret << piece.getPieceSymbol() << COL_NAMES[move.col2 - 1] << move.row2;
    return ret.str();
}

int main(int argc, char *argv[]) {
    Color toMove = NOCOLOR;
    int depth = 0;
    char* inputFilename = NULL;
    int opt = 0;
    Color playing;

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

    if (inputFilename == NULL || depth <= 0) {
        std::cout << "Usage: ./Board -f <filename> -d <depth>" << std::endl;
        return 1;
    }

    Board* board = new Board();

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

    int procID;
    int nproc;

    MPI_Comm_rank(MPI_COMM_WORLD, &procID);
    MPI_Comm_size(MPI_COMM_WORLD, &nproc);

    while (true) {
        if (procID == 0) {
            board->printBoard();
        }

        if (toMove == playing) {
            double startTime = MPI_Wtime();
            
            std::pair<Move, double> best = board->findBestMove(depth, toMove, MPI_COMM_WORLD);

            std::pair<double, int> sendBest(best.second, best.first.compress());
            std::pair<double, int> globalBest;

            if (toMove == WHITE) {
                MPI_Allreduce(&sendBest, &globalBest, 1, MPI_DOUBLE_INT, MPI_MAXLOC, MPI_COMM_WORLD);
            } else {
                MPI_Allreduce(&sendBest, &globalBest, 1, MPI_DOUBLE_INT, MPI_MINLOC, MPI_COMM_WORLD);
            }

            double endTime = MPI_Wtime();

            std::cout << "Elapsed time for proc " << procID << ": " << endTime - startTime << std::endl;
            if (procID == 0) {
                std::cout << "Best move: " << board->algebraicNotation(Move(globalBest.second)) << ", " << globalBest.first << std::endl;
            }

            board->applyMove(Move(globalBest.second));
        } else {
            int cmove;
            if (procID == 0) {
                Move move;
                std::cout << "Enter the opponent's move: <r1> <c1> <r2> <c2>" << std::endl;
                std::cin >> move.row1 >> move.col1 >> move.row2 >> move.col2;
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