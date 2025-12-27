#include "tranpositionTable.h"

uint16_t packMove(const Move& m) {
   return (uint16_t)m.from | 
           ((uint16_t)m.to << 6) | 
           ((uint16_t)m.promotionPiece << 12) | 
           ((uint16_t)m.nullMove << 15);
}

Move unpackMove(uint16_t data) {
    Move m;
    m.from = data & 0x3F;
    m.to = (data >> 6) & 0x3F;
    m.promotionPiece = (Pieces)((data >> 12) & 7);
    m.nullMove = (data >> 15) & 1;
    return m;
}

void TranspositionTable::clear(){
    for(size_t i = 0; i <= mask; i++){
        //Only clear the hash to save time
        entries[i].zhash = 0;
    }
}

void TranspositionTable::write(uint64_t zhash, uint8_t depth, uint8_t age, int16_t eval, Move move, NodeType type){
    TTEntry &slot = entries[zhash & mask];

    if(slot.zhash == 0 || slot.depth <= depth || slot.age != age){
        slot.zhash = zhash;
        slot.depth = depth;
        slot.age = age;
        slot.eval = eval;
        slot.bestMove = packMove(move);
        slot.type = type;
    }
}

bool TranspositionTable::probe(uint64_t zhash, TTEntry &out){
    TTEntry &slot = entries[zhash & mask];
    
    if(slot.zhash != zhash) return false;

    out = slot;
    return true;

}


