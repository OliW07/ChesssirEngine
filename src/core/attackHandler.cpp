#include "attackHandler.h"

#include <stdexcept>

#include "Types.h"
#include "bitops.h"
#include "board.h"
#include "moveGenerator.h"
#include "precompute.h"

using namespace precomputedData;
using namespace ChessEngine::Utils;

uint64_t AttackHandler::getAttacks(const int pos) {
    uint64_t PieceAttacks = 0ULL;
    uint64_t target = 1ULL << pos;

    if (game.board.isSquareEmpty(pos))
        return 0ULL;

    enum Pieces pieceType = (Pieces)game.board.getPieceEnum(pos);
    Colour colour = game.board.getColour(pos);

    switch (pieceType) {
        case Pawn:
            PieceAttacks = game.board.getColour(pos) == Colour::White ? whitePawnAttacks[pos] : blackPawnAttacks[pos];
            // The pawn can only attack (diagonally) is there is an enemy piece there
            PieceAttacks &= game.board.state.occupancy(invertColour(colour));
            break;
        case Knight:
            PieceAttacks = knightMoves[pos];
            break;
        case Bishop:
            PieceAttacks = bishopMoves[pos];
            break;
        case Rook:
            PieceAttacks = rookMoves[pos];
            break;
        case King:
            PieceAttacks = kingMoves[pos];
            break;
        case Queen:
            PieceAttacks = queenMoves[pos];
            break;
        default:
            throw std::runtime_error("Invalid piecetype: ");
            break;
    }

    // Find which piece it at the position

    uint64_t blockingRays = 0ULL;
    uint64_t blockers = PieceAttacks & game.board.state.occupancy(Colour::Both);

    for (RaysDirection direction : SLIDING_DIRECTIONS(pieceType)) {
        if (!blockers)
            break;

        uint64_t blockers1Direction = blockers & rays[direction][pos];

        if (!blockers1Direction)
            continue;

        int firstBlockerPos = game.board.getFirstBlocker(pos, direction);

        if (firstBlockerPos == -1)
            continue;

        blockingRays |= rays[direction][firstBlockerPos];
    }

    uint64_t attacks = (PieceAttacks) & (~game.board.getFriendlyPieces(pos)) & (~blockingRays);

    if (pieceType == Pawn && game.board.state.enPassantSquare != -1) {
        bool pieceWraps = pieceWrapsTheBoard(game.board.state.enPassantSquare, pos);

        // The pawn must be diagonally next to the enpassant square to move there
        if ((game.board.getColour(pos) == Colour::White &&
             ((game.board.state.enPassantSquare - 9 == pos) || (game.board.state.enPassantSquare - 7 == pos))) &&
            !pieceWraps) {
            attacks |= (1ULL << game.board.state.enPassantSquare);
        } else if (game.board.getColour(pos) == Colour::Black &&
                   ((game.board.state.enPassantSquare + 9 == pos) || (game.board.state.enPassantSquare + 7 == pos)) &&
                   !pieceWraps) {
            attacks |= (1ULL << game.board.state.enPassantSquare);
        }
    }

    return attacks;
}

uint64_t AttackHandler::getAllAttacks(Colour colour) {
    uint64_t attacks = 0ULL;
    uint64_t pieceBitBoard = game.board.state.occupancy(colour);

    while (pieceBitBoard) {
        int location = ctz64(pieceBitBoard);
        pieceBitBoard &= (pieceBitBoard - 1);

        attacks |= getAttacks(location);
    }

    return attacks;
}

uint64_t AttackHandler::getAttackers(int pos, Colour attackingColour) {
    uint64_t attackers = 0ULL;

    attackers |= getPawnAttackers(pos, attackingColour);
    attackers |= game.board.state.bitboards(attackingColour, Knight);

    for (int i = 0; i < 8; ++i) {
        RaysDirection direction = static_cast<RaysDirection>(i);

        int attackingPiecePos = game.board.getFirstBlocker(pos, direction);

        if (attackingPiecePos == -1)
            continue;

        Pieces pieceType = (Pieces)game.board.getPieceEnum(attackingPiecePos);

        if (game.board.getColour(attackingPiecePos) != attackingColour)
            continue;

        bool straightSliding = (direction == North || direction == East || direction == West || direction == South);

        if ((straightSliding && ((pieceType == Rook) || pieceType == Queen)) ||
            (!straightSliding && ((pieceType == Bishop || pieceType == Queen)))) {
            attackers |= (1ULL << attackingPiecePos);
        }
    }

    return attackers;
}

uint64_t AttackHandler::getPawnAttackers(int pos, Colour attackingColour) {
    if (!posInBounds(pos))
        return 0ULL;

    uint64_t attackers = 0ULL;
    uint64_t* pieceBitBoard = &game.board.state.bitboards(attackingColour, Pawn);

    int offsets[2] = {7, 9};

    for (int direction : offsets) {
        int pawnPos = attackingColour == Colour::White ? pos - direction : pos + direction;

        if (!posInBounds(pawnPos))
            continue;

        bool pieceWraps = pieceWrapsTheBoard(pos, pawnPos);

        if ((*pieceBitBoard & (1ULL << pawnPos)) && !pieceWraps) {
            attackers |= (1ULL << pawnPos);
        }
    }

    return attackers;
}

uint64_t AttackHandler::pawnControlledSquare(Colour controllingColour) {
    uint64_t pawnBitBoard = game.board.state.bitboards(controllingColour, Pawn);
    uint64_t* pawnAttacks = controllingColour == Colour::White ? whitePawnAttacks : blackPawnAttacks;
    uint64_t controlledSquares = 0ULL;

    while (pawnBitBoard) {
        int pawnPos = ctz64(pawnBitBoard);
        controlledSquares |= pawnAttacks[pawnPos];
        pawnBitBoard ^= (1ULL << pawnPos);
    }

    return controlledSquares;
}

uint64_t AttackHandler::getPinnedPieces(Colour colour, bool includeEnemies) {
    uint64_t pinnedPieces = 0ULL;
    uint64_t kingLocation = game.board.getKingLocation(colour);

    for (RaysDirection direction : ALL_DIRECTIONS) {
        uint64_t ray = rays[direction][kingLocation];

        int firstBlocker = game.board.getFirstBlocker(kingLocation, direction);
        if (firstBlocker == -1)
            continue;

        // Pinned piece must be a friendly piece
        if (game.board.getColour(firstBlocker) != game.board.getColour(kingLocation) && !includeEnemies)
            continue;

        int secondBlocker = game.board.getFirstBlocker(firstBlocker, direction);
        if (secondBlocker == -1)
            continue;

        // If the second piece is also a friendly, nothing is pinned
        if (game.board.getColour(secondBlocker) == game.board.getColour(kingLocation))
            continue;

        enum Pieces enemyPiece = (Pieces)game.board.getPieceEnum(secondBlocker);

        if ((enemyPiece == Bishop || enemyPiece == Queen) &&
            (direction == NorthEast || direction == NorthWest || direction == SouthEast || direction == SouthWest)) {
            pinnedPieces |= (1ULL << firstBlocker);

        } else if ((enemyPiece == Rook || enemyPiece == Queen) &&
                   (direction == North || direction == South || direction == East || direction == West)) {
            pinnedPieces |= (1ULL << firstBlocker);
        }
    }

    return pinnedPieces;
}

bool AttackHandler::isSquareAttacked(int pos, Colour attackingColour) {
    if (getPawnAttackers(pos, attackingColour))
        return true;
    uint64_t enemyKnights = game.board.state.bitboards(attackingColour, Knight);
    uint64_t enemyKing = game.board.state.bitboards(attackingColour, King);
    uint64_t enemyRooks = game.board.state.bitboards(attackingColour, Rook);
    uint64_t enemyBishops = game.board.state.bitboards(attackingColour, Bishop);
    uint64_t enemyQueens = game.board.state.bitboards(attackingColour, Queen);

    if (knightMoves[pos] & enemyKnights)
        return true;
    if (kingMoves[pos] & enemyKing)
        return true;

    for (RaysDirection direction : DIAGONAL_DIRECTIONS) {
        int attackerPos = game.board.getFirstBlocker(pos, direction);
        if (attackerPos == -1)
            continue;

        if ((1ULL << attackerPos) & (enemyBishops | enemyQueens))
            return true;
    }

    for (RaysDirection direction : ORTHOGONAL_DIRECTIONS) {
        int attackerPos = game.board.getFirstBlocker(pos, direction);
        if (attackerPos == -1)
            continue;

        if ((1ULL << attackerPos) & (enemyRooks | enemyQueens))
            return true;
    }

    return false;
}
