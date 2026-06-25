#include <iostream>
#include <stdexcept>
#include <vector>

#include "utils/Types.h"
#include "utils/bitops.h"

using namespace ChessEngine::Utils;

int convertNotationToInt(const std::string& notation) {
    int file = static_cast<int>(notation[0] - 'a');
    int rank = static_cast<int>(notation[1] - '0');

    if ((file > 7) || (file < 0) || (rank > 8) || (rank < 1)) {
        throw std::runtime_error("Notation " + notation + "is out of range ");
    }

    return (file + (rank - 1) * 8);
}

int convertLocationToRows(const int location) {
    if (location < 0 || location > 63) {
        throw std::runtime_error("Location is out of range");
    }
    return location / 8;
}

int countOnes(uint64_t state) {
    return popcount64(state);
}

Move convertAlgebraicNotationToMove(const std::string& notation) {
    if (notation.length() < 4 || notation.length() > 5)
        return {};

    std::string from = notation.substr(0, 2);
    std::string to = notation.substr(2, 2);

    Pieces promotionPiece = None;

    Move move;

    try {
        if (notation.length() == 5) {
            promotionPiece = promotionCharToPiece.at(notation[4]);
        }

        move.to = convertNotationToInt(to);
        move.from = convertNotationToInt(from);
        move.promotionPiece = promotionPiece;

        return move;

    } catch (...) {
        move.nullMove = true;
        return move;
    }
}

Colour getSquareColour(Square square) {
    int rows = convertLocationToRows(square);
    int columns = convertLocationToColumns(square);

    if (rows % 2 == 0) {
        if (columns % 2 == 0)
            return Colour::Black;
        return Colour::White;
    }

    if (columns % 2 == 0)
        return Colour::White;

    return Colour::Black;
}

Colour invertColour(Colour colour) {
    if (colour == Colour::White)
        return Colour::Black;
    if (colour == Colour::Black)
        return Colour::White;

    throw std::runtime_error("Can only invert White / Black colour");
}

void PieceList::addPiece(Square square, Colour colour) {
    if (pieceCount[(size_t)colour] >= 16) {
        std::cout << "DEBUG: Piece list overflow attempt" << std::endl;
        return;
    }
    list[(size_t)colour][pieceCount[(size_t)colour]] = square;
    mapBoardLocToList[square] = pieceCount[(size_t)colour];
    pieceCount[(size_t)colour]++;
}

void PieceList::removePiece(Square square, Colour colour) {
    int listIndex = mapBoardLocToList[square];

    // Swap current index with last index, to keep O(1)
    list[(size_t)colour][listIndex] = list[(size_t)colour][pieceCount[(size_t)colour] - 1];

    mapBoardLocToList[list[(size_t)colour][listIndex]] = listIndex;

    pieceCount[(size_t)colour]--;

    mapBoardLocToList[square] = -1;
}

void PieceList::movePiece(int to, int from, Colour colour) {
    int listIndex = mapBoardLocToList[from];
    list[(size_t)colour][listIndex] = to;
    mapBoardLocToList[from] = -1;
    mapBoardLocToList[to] = listIndex;
}

std::vector<int> getLocationsFromBitBoard(uint64_t bitBoard) {
    std::vector<int> locations = {};
    while (bitBoard) {
        int location = ctz64(bitBoard);
        locations.push_back(location);

        // Toggle the bit off
        bitBoard ^= (1ULL << location);
    }

    return locations;
}

RaysDirection squaresToDirection(Square square1, Square square2) {
    int square1Rows = convertLocationToRows(square1), square2Rows = convertLocationToRows(square2),
        square1Columns = convertLocationToColumns(square1), square2Columns = convertLocationToColumns(square2),

        rowsDifference = square1Rows - square2Rows, columnsDifference = square1Columns - square2Columns;

    bool isDiagonal = abs(rowsDifference) == abs(columnsDifference);

    // Direction relative to pos1
    RaysDirection direction;

    if (rowsDifference == 0 && columnsDifference > 0) {
        direction = West;
    } else if (rowsDifference == 0 && columnsDifference < 0) {
        direction = East;
    } else if (columnsDifference == 0 && rowsDifference > 0) {
        direction = South;
    } else if (columnsDifference == 0 && rowsDifference < 0) {
        direction = North;
    } else if (rowsDifference > 0 && columnsDifference > 0 && isDiagonal) {
        direction = SouthWest;
    } else if (rowsDifference < 0 && columnsDifference < 0 && isDiagonal) {
        direction = NorthEast;
    } else if (rowsDifference > 0 && columnsDifference < 0 && isDiagonal) {
        direction = SouthEast;
    } else if (rowsDifference < 0 && columnsDifference > 0 && isDiagonal) {
        direction = NorthWest;
    }

    return direction;
}

std::string convertMoveToAlgebraicNotation(Move move) {
    // Check for invalid moves (null moves or uninitialized moves)
    if (move.nullMove || move.from > 63 || move.to > 63) {
        return "0000";  // Return null move notation for invalid moves
    }

    char numberTo = convertLocationToRows(move.to) + '1';
    char letterTo = convertLocationToColumns(move.to) + 'a';

    char numberFrom = convertLocationToRows(move.from) + '1';
    char letterFrom = convertLocationToColumns(move.from) + 'a';

    std::string result = std::string() + letterFrom + numberFrom + letterTo + numberTo;

    if (move.promotionPiece != None)
        result += pieceToNotation.at(move.promotionPiece);

    return result;
}

int convertLocationToColumns(const int location) {
    if (location < 0 || location > 63) {
        throw std::runtime_error("Location " + std::to_string(location) + " is out of range");
    }
    return location % 8;
}

bool onlyOnePiece(uint64_t state) {
    if (state == 0)
        return false;
    return (ctz64(state) + clz64(state) == 63);
}

bool squareInBounds(Square square) {
    return (square >= 0 && square <= 63);
}

bool pieceWrapsTheBoard(Square square1, Square square2) {
    int columnsDifference = abs(convertLocationToColumns(square1) - convertLocationToColumns(square2));
    int rowsDifference = abs(convertLocationToRows(square1) - convertLocationToRows(square2));

    return !((columnsDifference < 2) && (rowsDifference < 2));
}

uint8_t convertPieceToBinary(Pieces pieceEnum, bool isWhite) {
    return isWhite ? pieceEnum : pieceEnum + 8;
}
