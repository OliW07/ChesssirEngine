#include "moveGenerator.h"

#include <stdexcept>

#include "Types.h"
#include "bitops.h"
#include "board.h"
#include "evaluate.h"
#include "precompute.h"

using namespace precomputedData;
using namespace ChessEngine::Utils;

uint64_t MoveGenerator::getPseudoLegalMoves(const Square square) {
    if (game.board.isSquareEmpty(square))
        return 0ULL;

    uint64_t pseudoLegalMoves = game.attackHandler.getAttacks(square);
    bool isWhite = game.board.getColour(square) == Colour::White;

    enum Pieces pieceType = (Pieces)game.board.getPieceEnum(square);

    // Add (non attack) pawn moves
    if (pieceType == Pawn) {
        uint64_t* pawnMoves = isWhite ? &whitePawnMoves[square] : &blackPawnMoves[square];

        // Pawns can't move onto their own pieces or friendly pieces
        pseudoLegalMoves |= (*pawnMoves & ~game.board.state.occupancy(Colour::Both));

        int oneStepIndex = isWhite ? square + 8 : square - 8;

        // Check if the single push is blocked, if so, the double push is also blocked
        if (oneStepIndex >= 0 && oneStepIndex < 64) {
            if ((1ULL << oneStepIndex) & game.board.state.occupancy(Colour::Both)) {
                int twoStepIndex = isWhite ? square + 16 : square - 16;
                if (twoStepIndex >= 0 && twoStepIndex < 64) {
                    pseudoLegalMoves &= ~(1ULL << twoStepIndex);
                }
            }
        }
    }

    // Add castling

    if (pieceType == King && game.board.state.castlingRights > 0) {
        if (isWhite && (game.board.state.castlingRights & 8) &&
            !game.attackHandler.isSquareAttacked(5, Colour::Black) &&
            !game.attackHandler.isSquareAttacked(6, Colour::Black) && game.board.isSquareEmpty(5) &&
            game.board.isSquareEmpty(6)) {
            // White kingside castling square
            pseudoLegalMoves |= (1ULL << 6);
        }
        if (isWhite && (game.board.state.castlingRights & 4) &&
            !game.attackHandler.isSquareAttacked(3, Colour::Black) &&
            !game.attackHandler.isSquareAttacked(2, Colour::Black) && game.board.isSquareEmpty(3) &&
            game.board.isSquareEmpty(2) && game.board.isSquareEmpty(1)) {
            // White queenside castling square
            pseudoLegalMoves |= (1ULL << 2);
        }
        if (!isWhite && (game.board.state.castlingRights & 2) &&
            !game.attackHandler.isSquareAttacked(61, Colour::White) &&
            !game.attackHandler.isSquareAttacked(62, Colour::White) && game.board.isSquareEmpty(61) &&
            game.board.isSquareEmpty(62)) {
            // Black kingside castling square
            pseudoLegalMoves |= (1ULL << 62);
        }
        if (!isWhite && (game.board.state.castlingRights & 1) &&
            !game.attackHandler.isSquareAttacked(58, Colour::White) &&
            !game.attackHandler.isSquareAttacked(59, Colour::White) && game.board.isSquareEmpty(57) &&
            game.board.isSquareEmpty(58) && game.board.isSquareEmpty(59)) {
            // Black queenside castling square
            pseudoLegalMoves |= (1ULL << 58);
        }
    }

    return pseudoLegalMoves;
}

uint64_t MoveGenerator::applyLegalMoveValidation(const Square square, uint64_t moves) {
    uint64_t legalMoves = moves;

    enum Pieces pieceType = (Pieces)game.board.getPieceEnum(square);

    bool isWhite = game.board.getColour(square) == Colour::White;
    Colour colour = isWhite ? Colour::White : Colour::Black;
    Colour enemyColour = invertColour(colour);

    int kingLocation = game.board.getKingLocation(colour);

    if (pieceType == King) {
        // For each move, check the square is not controlled by the enemy and disallow king from castling whilst in
        // check
        uint64_t movesClone = legalMoves;

        while (movesClone) {
            int move = ctz64(movesClone);
            movesClone &= (movesClone - 1);

            if (game.attackHandler.isSquareAttacked(move, enemyColour))
                legalMoves ^= (1ULL << move);

            if (abs(convertLocationToColumns(move) - convertLocationToColumns(square)) > 1 &&
                (game.attackHandler.isSquareAttacked(square, enemyColour)))
                legalMoves ^= (1ULL << move);
        }

        // For each sliding piece attacker, remove the squares that the king hides inline with the attacking ray

        uint64_t attackers = game.attackHandler.getAttackers(kingLocation, enemyColour);

        while (attackers) {
            int attackerLoc = ctz64(attackers);
            attackers &= (attackers - 1);

            enum Pieces attackerType = (Pieces)game.board.getPieceEnum(attackerLoc);

            if (attackerType == Knight || attackerType == King || attackerType == Pawn)
                continue;

            RaysDirection direction = squaresToDirection(attackerLoc, square);
            legalMoves &= ~rays[direction][attackerLoc];
        }

    }

    else if (game.attackHandler.isSquareAttacked(kingLocation, enemyColour)) {
        uint64_t attackers = game.attackHandler.getAttackers(kingLocation, enemyColour);

        // If the king is attacked more than once, the king is the only piece that can move;
        if (countOnes(attackers) > 1)
            return 0ULL;

        int attackerLocation = ctz64(attackers);

        uint64_t blockingCapturingRay;

        if (game.board.getPieceEnum(attackerLocation) == Knight) {
            blockingCapturingRay = attackers;
        } else {
            blockingCapturingRay = game.board.getRay(kingLocation, attackerLocation);
        }
        int enpassantVictimLoc = isWhite ? game.board.state.enPassantSquare - 8 : game.board.state.enPassantSquare + 8;
        if (pieceType == Pawn && ((1ULL << game.board.state.enPassantSquare) & legalMoves) &&
            (1ULL << attackerLocation) & (1ULL << enpassantVictimLoc)) {
            // Pawns capturing enpassant may capture if it is the one attacking piece
            legalMoves &= blockingCapturingRay;
            legalMoves |= (1ULL << game.board.state.enPassantSquare);

        } else {
            legalMoves &= blockingCapturingRay;
        }
    }
    // If the piece is pinned, limit its movement to the ray between itself and the king
    if (game.attackHandler.getPinnedPieces(colour) & (1ULL << square)) {
        int kingLocation = game.board.getKingLocation(colour);

        RaysDirection direction = squaresToDirection(kingLocation, square);

        legalMoves &= rays[direction][kingLocation];
    }

    // Taking enpassant removes two pieces, disallow if both pieces are pinned
    if (pieceType == Pawn) {
        uint64_t movesClone = legalMoves;

        while (movesClone) {
            int move = ctz64(movesClone);
            movesClone &= (movesClone - 1);

            int enpassantVictimLoc = isWhite ? move - 8 : move + 8;

            // If the move is diagonal and to an empty square it is enpassantenpassantVictimLoc
            if ((convertLocationToColumns(square) - convertLocationToColumns(move) != 0) &&
                (convertLocationToRows(square) - convertLocationToRows(move) != 0) &&
                !((1ULL << move) & game.board.state.occupancy(Colour::Both))) {
                // Temporarily remove the pawn, to check if the enpassant victim is pinned
                uint64_t* pawnBitBoard = isWhite ? &game.board.state.bitboards(Colour::White, Pawn)
                                                 : &game.board.state.bitboards(Colour::Black, Pawn);

                uint64_t pawnMask = (1ULL << square);

                *pawnBitBoard ^= pawnMask;
                game.board.state.occupancy(Colour::Both) ^= pawnMask;
                game.board.state.occupancy(colour) ^= pawnMask;

                if ((game.attackHandler.getPinnedPieces(colour, true) & (1ULL << enpassantVictimLoc)) &&
                    (convertLocationToRows(square) == convertLocationToRows(kingLocation))) {
                    legalMoves ^= (1ULL << move);
                }

                // Add back the pawn we removed

                *pawnBitBoard ^= pawnMask;
                game.board.state.occupancy(Colour::Both) ^= pawnMask;
                game.board.state.occupancy(colour) ^= pawnMask;
            }
        }
    }

    return legalMoves;
}

uint64_t MoveGenerator::getLegalMoves(const Square square) {
    // Legal moves, except for pawn moves to promotion squares, handled seperately

    if (square < 0 || square > 63)
        throw std::runtime_error("The position provided, is not in range 0-63");

    uint64_t legalMoves = applyLegalMoveValidation(square, getPseudoLegalMoves(square));

    enum Pieces pieceType = (Pieces)game.board.getPieceEnum(square);

    // Remove pawns promting, handling seperately with multiple outcomes

    if (pieceType == Pawn) {
        int promotionRank = game.board.getColour(square) == Colour::White ? 7 : 0;
        legalMoves &= ~(rankMasks[promotionRank]);
    }

    return legalMoves;
}

uint64_t MoveGenerator::getPromotionMoves(const Square square) {
    Pieces pieceType = (Pieces)game.board.getPieceEnum(square);
    if (pieceType != Pawn)
        return 0ULL;

    int promotionRank = game.board.getColour(square) == Colour::White ? 7 : 0;

    uint64_t promotionMoves = getPseudoLegalMoves(square) & rankMasks[promotionRank];

    return applyLegalMoveValidation(square, promotionMoves);
}

MoveList MoveGenerator::getAllMoves() {
    MoveList moves;

    Colour activeColour = (Colour)game.board.state.whiteToMove;

    for (int i = 0; i < game.board.state.pieceList.pieceCount[(size_t)activeColour]; i++) {
        int pieceLoc = game.board.state.pieceList.list[(size_t)activeColour][i];
        uint64_t legalMoves = getLegalMoves(pieceLoc);
        uint64_t promotionMoves = getPromotionMoves(pieceLoc);

        Move move;
        move.from = pieceLoc;

        while (legalMoves) {
            move.to = ctz64(legalMoves);
            legalMoves &= (legalMoves - 1);
            moves.add(move);

            setCaptureScore(move, game.board);
        }

        while (promotionMoves) {
            move.to = ctz64(promotionMoves);
            promotionMoves &= (promotionMoves - 1);

            for (Pieces promotionPiece : {Rook, Knight, Bishop, Queen}) {
                move.promotionPiece = promotionPiece;
                moves.add(move);

                setCaptureScore(move, game.board);
            }
        }
    }

    return moves;
}

MoveList MoveGenerator::getAllCaptures() {
    MoveList moves;

    Colour activeColour = (Colour)game.board.state.whiteToMove;

    for (int i = 0; i < game.board.state.pieceList.pieceCount[(size_t)activeColour]; i++) {
        int pieceLoc = game.board.state.pieceList.list[(size_t)activeColour][i];
        uint64_t legalMoves = getLegalMoves(pieceLoc);
        uint64_t promotionMoves = getPromotionMoves(pieceLoc);

        Move move;
        move.from = pieceLoc;

        while (legalMoves) {
            move.to = ctz64(legalMoves);
            legalMoves &= (legalMoves - 1);

            bool isEnPassant = game.board.state.enPassantSquare != -1 && move.to == game.board.state.enPassantSquare;

            if ((game.board.state.occupancy(Colour::Both) & (1ULL << move.to)) || isEnPassant) {
                moves.add(move);
                setCaptureScore(move, game.board);
                if (isEnPassant) {
                    move.orderScore = PieceValues[Pawn] * 10 - PieceValues[Pawn];
                }
            }
        }

        while (promotionMoves) {
            move.to = ctz64(promotionMoves);
            promotionMoves &= (promotionMoves - 1);

            for (Pieces promotionPiece : {Rook, Knight, Bishop, Queen}) {
                move.promotionPiece = promotionPiece;

                if (game.board.state.occupancy(Colour::Both) & (1ULL << move.to)) {
                    moves.add(move);
                    setCaptureScore(move, game.board);
                }
            }
        }
    }

    return moves;
}

void setCaptureScore(Move& move, Board& board) {
    if (board.state.mailBox[move.to] != 0) {
        int victimValue = PieceValues[board.getPieceEnum(move.to)];
        int attackerValue = PieceValues[board.getPieceEnum(move.from)];
        move.orderScore = victimValue * 10 - attackerValue;
    }
}
