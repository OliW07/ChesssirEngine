#include "debug.h"

#include <algorithm>
#include <bitset>
#include <cstdint>
#include <iostream>

#include "Types.h"

void visualiseBitBoard(const uint64_t& bitBoard) {
    std::bitset<64> boardBitSet(bitBoard);
    std::string bitString = boardBitSet.to_string();

    std::cout << "\n";

    for (int i = 0; i < 8; i++) {
        std::string ss = bitString.substr(8 * i, 8);

        std::reverse(ss.begin(), ss.end());

        for (char c : ss) {
            std::cout << c << " ";
        }

        std::cout << "\n";
    }

    std::cout << "\n";
}

void visualiseGraphicBoard(BoardState state) {
    char temp;

    std::cout << "\n";

    for (int rows = 7; rows >= 0; rows--) {
        for (int columns = 0; columns < 8; columns++) {
            Square square = 8 * rows + columns;

            // At each location, check all possible piece and if present add to array

            bool whitePawn = state.bitboards(Colour::White, Pawn) & (1ULL << square),
                 whiteBishop = state.bitboards(Colour::White, Bishop) & (1ULL << square),
                 whiteKnight = state.bitboards(Colour::White, Knight) & (1ULL << square),
                 whiteRook = state.bitboards(Colour::White, Rook) & (1ULL << square),
                 whiteKing = state.bitboards(Colour::White, King) & (1ULL << square),
                 whiteQueen = state.bitboards(Colour::White, Queen) & (1ULL << square),
                 blackPawn = state.bitboards(Colour::Black, Pawn) & (1ULL << square),
                 blackBishop = state.bitboards(Colour::Black, Bishop) & (1ULL << square),
                 blackKnight = state.bitboards(Colour::Black, Knight) & (1ULL << square),
                 blackRook = state.bitboards(Colour::Black, Rook) & (1ULL << square),
                 blackKing = state.bitboards(Colour::Black, King) & (1ULL << square),
                 blackQueen = state.bitboards(Colour::Black, Queen) & (1ULL << square);

            if (whitePawn)
                temp = 'P';
            else if (whiteBishop)
                temp = 'B';
            else if (whiteKnight)
                temp = 'N';
            else if (whiteRook)
                temp = 'R';
            else if (whiteQueen)
                temp = 'Q';
            else if (whiteKing)
                temp = 'K';
            else if (blackPawn)
                temp = 'p';
            else if (blackKnight)
                temp = 'n';
            else if (blackBishop)
                temp = 'b';
            else if (blackQueen)
                temp = 'q';
            else if (blackKing)
                temp = 'k';
            else if (blackRook)
                temp = 'r';
            else
                temp = '-';

            std::cout << temp << "   ";
        }

        std::cout << "\n\n";
    }

    std::cout << "\n";
}
