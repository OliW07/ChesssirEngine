#include <string>
#include <iostream>
#include <sstream>
#include <thread>
#include <fstream>

#include "board.h" 
#include "utils/uci.h"
#include "utils/log.h"
#include "utils/Types.h"
#include "moveGenerator.h"
#include "engine.h" 

void readLoop(Game& game){
    std::string line, command;

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
        else if (command == "quit") {
            game.chesssir.stopRequested = true;
            break;
        }

        debugFile.flush();
    }

    // The detached search thread will continue until it's done.
    // No need to join it here anymore.
}
