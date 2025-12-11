#ifndef ENGINE_H
#define ENGINE_H

#include "utils/Types.h"
#include "board.h"


class Engine {
    public:

        Move bestMove(Board &boardInstance, int maxDepth);
        int miniMax(Board &boardInstance, int maxDepth);

};
#endif
