#include "attackHandler.h"

#include <stdexcept>

#include "Types.h"
#include "bitops.h"
#include "board.h"
#include "moveGenerator.h"
#include "precompute.h"

using namespace precomputedData;
using namespace ChessEngine::Utils;

uint64_t AttackHandler::getAttacks(const Square square) {
    uint64_t PieceAttacks = 0ULL;
    uint64_t target = 1ULL << square;

    if (game.board.isSquareEmpty(square))
        return 0ULL;

    enum Pieces pieceType = (Pieces)game.board.getPieceEnum(square);
    Colour colour = game.board.getColour(square);

    switch (pieceType) {
        case Pawn:
            PieceAttacks =
                game.board.getColour(square) == Colour::White ? whitePawnAttacks[square] : blackPawnAttacks[square];
            // The pawn can only attack (diagonally) is there is an enemy piece there
            PieceAttacks &= game.board.state.occupancy(invertColour(colour));
            break;
        case Knight:
            PieceAttacks = knightMoves[square];
            break;
        case Bishop:
            PieceAttacks = bishopMoves[square];
            break;
        case Rook:
            PieceAttacks = rookMoves[square];
            break;
        case King:
            PieceAttacks = kingMoves[square];
            break;
        case Queen:
            PieceAttacks = queenMoves[square];
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

        uint64_t blockers1Direction = blockers & rays[direction][square];

        if (!blockers1Direction)
            continue;

        int firstBlockerSquare = game.board.getFirstBlocker(square, direction);

        if (firstBlockerSquare == -1)
            continue;

        blockingRays |= rays[direction][firstBlockerSquare];
    }

    uint64_t attacks = (PieceAttacks) & (~game.board.getFriendlyPieces(square)) & (~blockingRays);

    if (pieceType == Pawn && game.board.state.enPassantSquare != -1) {
        bool pieceWraps = pieceWrapsTheBoard(game.board.state.enPassantSquare, square);

        // The pawn must be diagonally next to the enpassant square to move there
        if ((game.board.getColour(square) == Colour::White &&
             ((game.board.state.enPassantSquare - 9 == square) || (game.board.state.enPassantSquare - 7 == square))) &&
            !pieceWraps) {
            attacks |= (1ULL << game.board.state.enPassantSquare);
        } else if (game.board.getColour(square) == Colour::Black &&
                   ((game.board.state.enPassantSquare + 9 == square) ||
                    (game.board.state.enPassantSquare + 7 == square)) &&
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

uint64_t AttackHandler::getAttackers(Square square, Colour attackingColour) {
    uint64_t attackers = 0ULL;

    attackers |= getPawnAttackers(square, attackingColour);
    attackers |= knightMoves[square] & game.board.state.bitboards(attackingColour, Knight);

    for (int i = 0; i < 8; ++i) {
        RaysDirection direction = static_cast<RaysDirection>(i);

        int attackingPieceSquare = game.board.getFirstBlocker(square, direction);

        if (attackingPieceSquare == -1)
            continue;

        Pieces pieceType = (Pieces)game.board.getPieceEnum(attackingPieceSquare);

        if (game.board.getColour(attackingPieceSquare) != attackingColour)
            continue;

        bool straightSliding = (direction == North || direction == East || direction == West || direction == South);

        if ((straightSliding && ((pieceType == Rook) || pieceType == Queen)) ||
            (!straightSliding && ((pieceType == Bishop || pieceType == Queen)))) {
            attackers |= (1ULL << attackingPieceSquare);
        }
    }

    return attackers;
}

uint64_t AttackHandler::getPawnAttackers(Square square, Colour attackingColour) {
    if (!squareInBounds(square))
        return 0ULL;

    uint64_t attackers = 0ULL;
    uint64_t* pieceBitBoard = &game.board.state.bitboards(attackingColour, Pawn);

    int offsets[2] = {7, 9};

    for (int direction : offsets) {
        int pawnSquare = attackingColour == Colour::White ? square - direction : square + direction;

        if (!squareInBounds(pawnSquare))
            continue;

        bool pieceWraps = pieceWrapsTheBoard(square, pawnSquare);

        if ((*pieceBitBoard & (1ULL << pawnSquare)) && !pieceWraps) {
            attackers |= (1ULL << pawnSquare);
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

bool AttackHandler::isSquareAttacked(Square square, Colour attackingColour) {
    if (getPawnAttackers(square, attackingColour))
        return true;
    uint64_t enemyKnights = game.board.state.bitboards(attackingColour, Knight);
    uint64_t enemyKing = game.board.state.bitboards(attackingColour, King);
    uint64_t enemyRooks = game.board.state.bitboards(attackingColour, Rook);
    uint64_t enemyBishops = game.board.state.bitboards(attackingColour, Bishop);
    uint64_t enemyQueens = game.board.state.bitboards(attackingColour, Queen);

    if (knightMoves[square] & enemyKnights)
        return true;
    if (kingMoves[square] & enemyKing)
        return true;

    for (RaysDirection direction : DIAGONAL_DIRECTIONS) {
        int attackerSquare = game.board.getFirstBlocker(square, direction);
        if (attackerSquare == -1)
            continue;

        if ((1ULL << attackerSquare) & (enemyBishops | enemyQueens))
            return true;
    }

    for (RaysDirection direction : ORTHOGONAL_DIRECTIONS) {
        int attackerSquare = game.board.getFirstBlocker(square, direction);
        if (attackerSquare == -1)
            continue;

        if ((1ULL << attackerSquare) & (enemyRooks | enemyQueens))
            return true;
    }

    return false;
}
