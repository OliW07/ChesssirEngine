#ifndef TT_H
#define TT_H

#include <Types.h>
#include <memory>

enum class NodeType : uint8_t {Exact, Upperbound, Lowerbound};


struct alignas(16) TTEntry {
    uint64_t zhash;
    uint8_t depth;
    uint8_t age;
    int16_t eval;
    uint16_t bestMove;
    NodeType type;
};


Move unpackMove(uint16_t data); 
uint16_t packMove(const Move& m); 

class TranspositionTable {
private:
    size_t mask;
public:

    
    std::unique_ptr<TTEntry[]> entries;

    TranspositionTable(int SIZE_MB){

        int maxEntries = SIZE_MB * 1024 * 1024 / sizeof(TTEntry);
        
        //Round size down to powers of 2 for quick operations
        maxEntries = 1ULL << (63 - __builtin_clzll(maxEntries));
        mask = maxEntries - 1;
        entries = std::make_unique_for_overwrite<TTEntry[]>(maxEntries);
    }

    void write(uint64_t zhash, uint8_t depth, uint8_t age, int16_t eval, Move move, NodeType type);
    void clear();
    bool probe(uint64_t zhash, TTEntry &out);
};




#endif
