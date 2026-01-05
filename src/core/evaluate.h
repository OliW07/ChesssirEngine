#ifndef EVALUATE_H
#define EVALUATE_H

#include "utils/Types.h"
#include "board.h"


void setFullEval(Board &board);

int generateFullEval(Board &board);

inline constexpr std::array<int, 7> PieceValues = []{
    std::array<int, 7> values{};
    
    values[static_cast<size_t>(Pieces::Pawn)]   = 100;
    values[static_cast<size_t>(Pieces::Knight)] = 320;
    values[static_cast<size_t>(Pieces::Bishop)] = 330;
    values[static_cast<size_t>(Pieces::Rook)]   = 500;
    values[static_cast<size_t>(Pieces::Queen)]  = 900;
    values[static_cast<size_t>(Pieces::King)]   = 0;
    values[static_cast<size_t>(Pieces::None)]   = 0;

    return values;
}();



#endif
