#include <string>
#include <iostream>
#include <sstream>
#include <thread>

#include "utils/uci.h"
#include "utils/Types.h"
#include "moveGenerator.h"
#include "engine.h" 

void readLoop(){
    std::string line, command;
    Game game;

    std::thread searchThread;

    while (std::getline(std::cin, line)) {
        std::stringstream ss(line);
        ss >> command;

        if (command == "uci") {
            std::cout << "id name chessirEngine 1.0" << std::endl;
            std::cout << "id author Oliver White" << std::endl;
            std::cout << "uciok" << std::endl;
        } 
        else if (command == "isready") {
            std::cout << "readyok" << std::endl;
        } 
        else if (command == "ucinewgame") {
    
        } 
        else if (command == "position") {
            std::string token, type;
            ss >> type;
            
            std::string fen = "";
            if (type == "startpos") {
                fen = STARTING_FEN;
                std::string temp; 

            } else if (type == "fen") {
                    
                while (ss >> token && token != "moves") {
                        fen += token + " ";
                }
                
            }

            MoveList moves;
            
            bool readingMoves = false;
            
            // Re-stream the line to catch "moves" part easily
            std::stringstream lineStream(line);
            std::string word;
            while(lineStream >> word) {
                if (word == "moves") {
                    readingMoves = true;
                    continue;
                }
                if (readingMoves) moves.add(convertAlgebraicNotationToMove(word));
            }
            
            game.setPosition(fen, moves);
        } 
        else if (command == "go") {
            std::string token;

            game.chesssir.stopRequested = false;

            while (ss >> token) {
                if (token == "wtime") ss >> game.info.wtime;
                else if (token == "btime") ss >> game.info.btime;
                else if (token == "winc")  ss >> game.info.winc;
                else if (token == "binc")  ss >> game.info.binc;
                else if (token == "movestogo") ss >> game.info.movestogo;
                else if (token == "depth") ss >> game.info.depth;
                else if (token == "movetime") ss >> game.info.movetime;
                else if (token == "infinite") game.info.infinite = true;
            }
                    
            if(searchThread.joinable()) searchThread.join();
                searchThread = std::thread(&Engine::search,&game.chesssir);
            
        } 
        else if (command == "stop") {
            game.chesssir.stopRequested = true; 
            if(searchThread.joinable()) searchThread.join();

        } 
        else if (command == "quit") {
            game.chesssir.stopRequested = true;
            if(searchThread.joinable()) searchThread.join();
            break;
        }
    }
}
