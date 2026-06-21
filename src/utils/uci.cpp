#include <string>
#include <iostream>
#include <sstream>
#include <thread>
#include <fstream>
#include <chrono>

#include "board.h" 
#include "utils/uci.h"
#include "utils/log.h"
#include "utils/Types.h"
#include "moveGenerator.h"
#include "engine.h" 
#include "perft.h"

void readLoop(Game& game){
    std::string line;

    std::thread searchThread;
    std::ofstream debugFile("engine_log.txt", std::ios::app);
    debugFile << "--- Engine Started ---" << std::endl;

    std::setvbuf(stdout, NULL, _IONBF, 0);

    while (std::getline(std::cin, line)) {

        if (!line.empty() && line.back() == '\r') line.pop_back();
            
        if (line.empty()) continue;
        debugFile << "Received: " << line << std::endl;

        std::stringstream ss(line);
        std::string command;
        if (!(ss >> command)) continue;
        

        if (command == "uci") {
            log_uci("id name chessirEngine 1.0", game.chesssir.uci_mutex);
            log_uci("id author Oliver White", game.chesssir.uci_mutex);
            log_uci("uciok", game.chesssir.uci_mutex);
            debugFile << "Sent: uciok" << std::endl;
        } 
        else if (command == "isready") {
            log_uci("readyok", game.chesssir.uci_mutex);
            debugFile << "Sent: readyok" << std::endl;
            
        } 
        else if (command == "position") {
            std::string type, token;
            ss >> type;
            
            std::string fen = "";
            if (type == "startpos") {
                fen = STARTING_FEN;
                ss >> token;
            } else if (type == "fen") {
                for (int i = 0; i < 6; i++) {
                    std::string part;
                    ss >> part;
                    fen += part + (i < 5 ? " " : "");
                }
                ss >> token;
            }

            MoveList moves;
            std::string moveStr;
            while (ss >> moveStr) {
                moves.add(convertAlgebraicNotationToMove(moveStr));
                game.ply++;
            }
            game.board.resetPosition(); 
            game.setPosition(fen, moves);

        } 
        else if (command == "go") {

            log_uci("info starting thinking", game.chesssir.uci_mutex);

            game.chesssir.stopRequested = true;

            // Wait for any previous search to stop. This is now handled by stopRequested flag.
            // A short sleep can be a simple way to allow the old thread to finish its current task.
            std::this_thread::sleep_for(std::chrono::milliseconds(10));

            std::string token;
            game.chesssir.stopRequested = false;
            while (ss >> token) {
                if (token == "wtime") ss >> game.info.wtime;
                else if (token == "btime") ss >> game.info.btime;
                else if (token == "winc")  ss >> game.info.winc;
                else if (token == "binc")  ss >> game.info.binc;
                else if (token == "depth") ss >> game.info.depth;
                else if (token == "movetime") ss >> game.info.movetime;
                else if (token == "infinite") game.info.infinite = true;
            }
            
            searchThread = std::thread(&Engine::writeBestMove, &game.chesssir);
            debugFile << "Search thread started." << std::endl;
            searchThread.detach();
            
        } 
        else if (command == "stop") {
            game.chesssir.stopRequested = true; 

        } 
        else if (command == "bench") {
            std::vector<std::string> positions = {
                STARTING_FEN,
                "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
                "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -",
                "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1"
            };

            int depth = 5;
            ss >> depth;

            uint64_t totalNodes = 0;
            auto benchStart = std::chrono::steady_clock::now();

            for (size_t i = 0; i < positions.size(); i++) {
                Game benchGame;
                benchGame.setPosition(positions[i], {});

                auto posStart = std::chrono::steady_clock::now();
                uint64_t nodes = perftSearch(benchGame, depth);
                auto posEnd = std::chrono::steady_clock::now();

                auto posMs = std::chrono::duration_cast<std::chrono::milliseconds>(posEnd - posStart).count();
                totalNodes += nodes;

                std::cout << "Position " << (i + 1) << ": "
                          << nodes << " nodes, "
                          << posMs << " ms, "
                          << (nodes * 1000 / (posMs > 0 ? posMs : 1)) << " nps" << std::endl;
            }

            auto benchEnd = std::chrono::steady_clock::now();
            auto benchMs = std::chrono::duration_cast<std::chrono::milliseconds>(benchEnd - benchStart).count();

            std::cout << "Total:   "
                      << totalNodes << " nodes, "
                      << benchMs << " ms, "
                      << (totalNodes * 1000 / (benchMs > 0 ? benchMs : 1)) << " nps" << std::endl;

        }
        else if (command == "quit") {
            game.chesssir.stopRequested = true;
            break;
        }

        debugFile.flush();
    }

    // The detached search thread will continue until it's done.
    // No need to join it here anymore.
}
