#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <map>
#include <span>
#include <string>
#include <unordered_map>
#include <vector>

enum Pieces { Bishop, Queen, Rook, King, Pawn, Knight, None };
enum RaysDirection { North, South, East, West, NorthEast, SouthEast, NorthWest, SouthWest };
enum class Colour : uint8_t { Black, White, Both };

constexpr size_t ColourCount = 3;
constexpr size_t PieceCount = 6;

constexpr std::array<RaysDirection, 8> ALL_DIRECTIONS = {North,     East,      South,     West,
                                                         NorthEast, SouthEast, NorthWest, SouthWest};
constexpr std::array<RaysDirection, 4> ORTHOGONAL_DIRECTIONS = {North, East, South, West};
constexpr std::array<RaysDirection, 4> DIAGONAL_DIRECTIONS = {NorthEast, SouthEast, NorthWest, SouthWest};

// North, South, East, West, NorthEast, NorthWest, SouthEast, SouthWest
constexpr std::array<int, 8> DIRECTION_OFFSETS = {8, -8, 1, -1, 9, -7, 7, -9};

const std::string STARTING_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
const int32_t MATESCORE = 30000;

class PieceList {
    public:
    int list[2][16];
    int mapBoardLocToList[64] = {-1};
    int pieceCount[2];

    PieceList() {
        std::fill(std::begin(mapBoardLocToList), std::end(mapBoardLocToList), -1);

        pieceCount[0] = 0;
        pieceCount[1] = 0;
    }

    void addPiece(int pos, Colour colour);
    void removePiece(int pos, Colour colour);
    void movePiece(int to, int from, Colour colour);
};

struct BitboardView {
    uint64_t bitboards[ColourCount][PieceCount] = {0ULL};

    uint64_t& operator()(Colour colour, Pieces piece) {
        return bitboards[static_cast<size_t>(colour)][static_cast<size_t>(piece)];
    }

    const uint64_t& operator()(Colour colour, Pieces piece) const {
        return bitboards[static_cast<size_t>(colour)][static_cast<size_t>(piece)];
    }
};

struct Occupancy {
    uint64_t bitboards[ColourCount] = {};

    uint64_t& operator()(Colour colour) { return bitboards[static_cast<size_t>(colour)]; }
};

struct BoardState {
    BitboardView bitboards;
    Occupancy occupancy;

    bool whiteToMove = 0;
    bool whiteStarts = 1;
    int enPassantSquare = -1;
    int halfMoveClock = 0;
    int fullMoveClock = 0;

    uint64_t zhash = 0ULL;

    // least three significant bits represent the pieceType enum 0-5 for each piece, the 4th represents colour, white =
    // 0
    std::array<uint8_t, 64> mailBox = {};

    PieceList pieceList;

    uint8_t castlingRights = 0;
};

struct Move {
    bool nullMove = false;
    uint8_t to;
    uint8_t from;
    Pieces promotionPiece = None;
    int orderScore = 0;

    bool operator==(const Move& otherMove) {
        return (nullMove == otherMove.nullMove && to == otherMove.to && from == otherMove.from &&
                promotionPiece == otherMove.promotionPiece);
    }
};

struct SearchInfo {
    int wtime = -1;
    int btime = -1;
    int winc = 0;
    int binc = 0;
    int movestogo = -1;
    int depth = -1;
    int movetime = -1;
    bool infinite = false;
};

const std::unordered_map<char, Pieces> promotionCharToPiece = {{'q', Queen}, {'r', Rook}, {'n', Knight}, {'b', Bishop}};

const std::unordered_map<char, Pieces> notationToPieceMap = {{'k', King},   {'q', Queen},  {'r', Rook},
                                                             {'b', Bishop}, {'n', Knight}, {'p', Pawn}};

const std::unordered_map<Pieces, char> pieceToNotation = {{King, 'k'},   {Queen, 'q'},  {Rook, 'r'},
                                                          {Bishop, 'b'}, {Knight, 'n'}, {Pawn, 'p'}};

int convertNotationToInt(const std::string& notation);
int convertLocationToRows(const int location);
int convertLocationToColumns(const int location);

int countOnes(uint64_t state);

bool onlyOnePiece(uint64_t state);
bool posInBounds(int pos);
bool pieceWrapsTheBoard(int pos1, int pos2);

uint8_t convertPieceToBinary(Pieces pieceEnum, bool isWhite);

RaysDirection convertPositionsToDirections(int pos1, int pos2);

constexpr std::span<const RaysDirection> SLIDING_DIRECTIONS(Pieces piece) {
    switch (piece) {
        case Bishop:
            return DIAGONAL_DIRECTIONS;
        case Rook:
            return ORTHOGONAL_DIRECTIONS;
        case Queen:
            return ALL_DIRECTIONS;
        default:
            return {};
    }
}

Colour getSquareColour(int pos);
Colour invertColour(Colour colour);
std::string convertMoveToAlgebraicNotation(Move move);

Move convertAlgebraicNotationToMove(const std::string& notation);
std::vector<int> getLocationsFromBitBoard(uint64_t bitBoard);
