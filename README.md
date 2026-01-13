# Chessir
### High Peformance C++20 Chess Engine

A UCI compatible chess engine built with a focus on low latency algorithms with a modern, sleek design. 

---

## Features

- **Bitboard board representation** ultra fast move generation with O(1) hardware accelerated bitwise operations.
- **Optimised algorithms** with alpha-beta pruning and zobrist hashing in transposition tables, evaluating > <u>500K nodes per second</u>
- **Test-Driven development** with comprehensive unit tests through the GTest framework
- **UCI protocol integration** allowing compatibility with all major chess platforms including chess.com and lichess.org.
---

## Architecture & Design

1. **Hybrid board representation solution** 
   The core of a chess engine is millions of calculations of moving pieces between an 8 x 8 chess board. Instead of a naive 8 x 8 array of pieces, the primary board representation uses a series of `uint64_t` allocating one bit per square. 
   - Why: This allows a single CPU operation (e.g. << or &) to perform multiple useful operations, such as generating all the pawn pushes or king attacks. This leverages bit-level parellelism, mimicking  the SIMD technique (single instruction, multiple data) by performing multiple logical operations on all 64 squares at the same time.
   - Visualisation:
    ```bash 
    White Pawns (0x000000000000FF00)
    8  [ 0  0  0  0  0  0  0  0 ]
    7  [ 0  0  0  0  0  0  0  0 ]
    6  [ 0  0  0  0  0  0  0  0 ]
    5  [ 0  0  0  0  0  0  0  0 ]
    4  [ 0  0  0  0  0  0  0  0 ]
    3  [ 0  0  0  0  0  0  0  0 ]
    2  [ 1  1  1  1  1  1  1  1 ]
    1  [ 0  0  0  0  0  0  0  0 ]
        a  b  c  d  e  f  g  h
    ```
     - Drawbacks of the sole bitboard approach include requiring O(n) to search through bits to find the exact locations of pieces, which is essential to move generation (think `for each piece do`) which is why additionally, the use of [Piece Lists](https://www.chessprogramming.org/Piece-Lists) and [Mailboxes](https://www.chessprogramming.org/Mailbox) are used for board representation. Although three simulataneuos representations seems over kill, updating each only ever requires a single operation, and the combination of the three allow any operation / query to <u> always run in constant time </u>

1. **State Management: Zobrist Hashing** 
To handle hashing positions to be stored in the Transposition Table, Zobrist Hashing was implemented, where for each piece a `uint64_t` is randomly allocated and then XORed together.
- Efficiency: Instead of re-hashing the full board after each move, O(n), the nature of the Zobrist Hash allows a single XOR operation for an O(1) incremental update.
- Impact: This allows the engine to recognise previously searched positions, and save time re-searching, resulting in an average increase of <u> 120% more nodes per second </u> even after the cost of probing the table.
- Drawbacks: The number of possible chess positions is extremely large, much larger than could ever be used to index a hashmap, and a `uint64_t` cannot fully guarentee there are no hash collisions (when two completely different positions happen to give the same hash) however, at less than 10<sup>-6</sup>, this is stastically insigifcant and thus justifies the performance benefits.

2. **Build system & Test-Driven Development**
- Utilising CMake's build configuration ensure seemless building across platforms, and allowing a clean modularized code strucutre.
- GTest unit testing to ensure any changes / further development can never subtly affect other modules. This is vital for a complex project such as a chess engine as many bugs can easily be introduced which may stay undetected in the code base. Following the TDD principals, tests are written first, and code after, every piece of code is unit tested, such as move generation using a standardised [Perft](https://www.chessprogramming.org/Perft) test, ensuring the engine can only ever play legal moves.

---

## Performance benchmarks

Performance is measured with a combination of Nodes Per Second (NPS) and total number of branches required to search.

| Feature | NPS (Before) | NPS (After) | Performance
| ------- | ------------ | ----------- |------------
|Unoptimised search | ~210K | - | Baseline
|Incremental hashing & eval | ~210K | ~260K | 24% increase
| Transposition table | ~260K | ~ 700K | 170% increase
| Alpha beta pruning | - | - | ~90% branch reduction

> \*_Note performance is an estimate, actual performance can vary based on your system_

--- 

## Verification & Correctness

To ensure the integrity of the move generation in all types of positions, the engine is validated against the standardized **Perft** (Performance Test) benchmarks. This is a test that verifies the number of leaf nodes that can be reached from a given depth, which can be verified against known, correct values.

### Perft Results (Standard Start Position)

| Depth | Result (Chessir) | Expected | Status |
| :--- | :--- | :--- | :---
| 1 | 20 | 20 | ✅ |
|2 | 400 | 400 | ✅ |
| 3 | 8902 | 8902 | ✅ |
|4 | 197281 | 197281 |  ✅|
|5 | 4865609 |4865609 | ✅|

> **Full verification**: I utilized a suite of "tricky" test positions from [The Chess Programming Wiki](https://www.chessprogramming.org/Perft_Results) at high depths to validate complex edge cases such as en passant, promotions, and castling.


## Upcoming
- **Magic bitboards**: Implementing faster sliding piece move generation
- **Lock free parallel search** using C++20 thread for mulit-core high performance
