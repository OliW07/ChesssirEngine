#include "board.h"

#include <algorithm>
#include <bit>
#include <cstdint>
#include <stdexcept>
#include <string>

#include "bitops.h"
#include "evaluate.h"
#include "fenHelper.h"
#include "precompute.h"
#include "zobrist.h"

using namespace precomputedData;
using namespace ChessEngine::Utils;

void Board::init() {
    precomputeBitBoardMoves();
}

uint64_t Board::getFriendlyPieces(int pos) {
    return state.occupancy(getColour(pos));
}

uint64_t Board::getKingLocation(Colour colour) {
    return ctz64(state.bitboards(colour, King));
}

uint64_t Board::getEnemyPieces(int pos) {
    return state.occupancy(invertColour(getColour(pos)));
}

uint64_t Board::getRay(int pos1, int pos2) {
    RaysDirection direction = convertPositionsToDirections(pos1, pos2);

    uint64_t ray = rays[direction][pos1];

    ray &= ~rays[direction][pos2];

    return ray;
}

uint64_t* Board::getBitBoardFromPiece(int pieceEnum, bool isWhite) {
    uint64_t* pieceBitBoard = nullptr;
    switch (pieceEnum) {
        case 0:
            pieceBitBoard = isWhite ? &state.bitboards(Colour::White, Bishop) : &state.bitboards(Colour::Black, Bishop);
            break;
        case 1:
            pieceBitBoard = isWhite ? &state.bitboards(Colour::White, Queen) : &state.bitboards(Colour::Black, Queen);
            break;
        case 2:
            pieceBitBoard = isWhite ? &state.bitboards(Colour::White, Rook) : &state.bitboards(Colour::Black, Rook);
            break;
        case 3:
            pieceBitBoard = isWhite ? &state.bitboards(Colour::White, King) : &state.bitboards(Colour::Black, King);
            break;
        case 4:
            pieceBitBoard = isWhite ? &state.bitboards(Colour::White, Pawn) : &state.bitboards(Colour::Black, Pawn);
            break;
        case 5:
            pieceBitBoard = isWhite ? &state.bitboards(Colour::White, Knight) : &state.bitboards(Colour::Black, Knight);
            break;
        default:
            throw std::runtime_error("Invalid piece enum");
            break;
    }
    return pieceBitBoard;
}

void Board::resetPosition() {
    BoardState newState;
    state = newState;

    enginePlaysWhite = false;
    historyIndex = 0;

    for (int i = 0; i < 2048; i++) {
        history[i] = SavedData{};
    }
}

int Board::getPieceEnum(int pos) {
    // Toggle off the colour of piece, to get left with the enum
    return state.mailBox[pos] & ~8;
}

int Board::getFirstBlocker(int pos, RaysDirection direction) {
    uint64_t blockers = rays[direction][pos] & (state.occupancy(Colour::White) | state.occupancy(Colour::Black));

    if (!blockers)
        return -1;

    if (direction == North || direction == East || direction == NorthEast || direction == NorthWest)
        return ctz64(blockers);

    return 63 - clz64(blockers);
}

void Game::setPosition(std::string fen, MoveList moves) {
    parseFenString(fen, board.state);

    board.state.zhash = generateFullHash(board);

    for (auto& move : moves) {
        board.makeMove(move);
    }

    board.enginePlaysWhite = board.state.whiteToMove;

    setFullEval(board);
}

Colour Board::getColour(int pos) {
    if ((1ULL << pos) & state.occupancy(Colour::White)) {
        return Colour::White;
    }
    return Colour::Black;
}

bool Board::isSquareEmpty(int pos) {
    uint64_t target = 1ULL << pos;

    uint64_t allPieces = state.occupancy(Colour::White) | state.occupancy(Colour::Black);

    return !(target & allPieces);
}

bool Game::isDraw() {
    return isTwoFoldRepition() || isInsufficientMaterial() || isFiftyMoveLimit();
}

bool Game::isTwoFoldRepition() {
    int maxHistoryDepth = std::max(0, board.historyIndex - board.state.halfMoveClock);

    // Start at the last index of the same side to move, only look at the same colour to move as we are checking for
    // duplicates
    for (int i = board.historyIndex - 2; i >= maxHistoryDepth; i -= 2) {
        if (board.state.zhash == board.history[i].zhash)
            return true;
    }

    return false;
}

bool Game::isInsufficientMaterial() {
    if (board.state.bitboards(Colour::Both, Queen) | board.state.bitboards(Colour::Both, Rook) |
        board.state.bitboards(Colour::Both, Pawn)) {
        return false;
    }

    int whiteKnights = std::popcount(board.state.bitboards(Colour::White, Knight));
    int blackKnights = std::popcount(board.state.bitboards(Colour::Black, Knight));
    int whiteBishops = std::popcount(board.state.bitboards(Colour::White, Bishop));
    int blackBishops = std::popcount(board.state.bitboards(Colour::Black, Bishop));

    int whiteMinors = whiteKnights + whiteBishops;
    int blackMinors = blackKnights + blackBishops;
    int totalMinors = whiteMinors + blackMinors;

    if (totalMinors < 2)
        return true;

    if (totalMinors == 2) {
        if (whiteKnights == 1 && blackKnights == 1) {
            return true;
        }

        if (whiteBishops == 1 && blackBishops == 1) {
            int whiteBLoc = ctz64(board.state.bitboards(Colour::White, Bishop));
            int blackBLoc = ctz64(board.state.bitboards(Colour::Black, Bishop));

            Colour whiteBColor = getSquareColour(whiteBLoc);
            Colour blackBColor = getSquareColour(blackBLoc);

            // Draw only if bishops are on the same color
            return (whiteBColor == blackBColor);
        }
    }

    // Only a draw if the side with the knights is playing against a lone King.
    if (whiteMinors == 2 && whiteKnights == 2 && blackMinors == 0)
        return true;
    if (blackMinors == 2 && blackKnights == 2 && whiteMinors == 0)
        return true;

    return false;
}

bool Game::isFiftyMoveLimit() {
    return (board.state.halfMoveClock >= 50);
}

bool Board::isCapture(Move& move) {
    return (state.occupancy(Colour::Both) & (1ULL << move.to));
}

int Board::scoreMove(Move& move) {
    if (isCapture(move)) {
        return (10 * PieceValues[getPieceEnum(move.to)]) - PieceValues[getPieceEnum(move.from)];
    }
    return 0;
}
