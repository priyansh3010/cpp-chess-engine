#pragma once
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include "board.h"
#include "engine.h"
#include "move.h"
#include "moveinfo.h"
#include "movegen.h"
#include "types.h"
#include "uci.h"
#include "utils.h"

namespace {
    void handleUCI() {
        cout << "id name pribot" << endl;
        cout << "id name pri3010" << endl;
        cout << "uciok" << endl;
    }
    
    void handleIsReady() {
        cout << "readyok" << endl;
    }
    
    void handleNewGame() {
        UCI::init();
    }
    
    void handlePosition(string line) {
        istringstream ss(line);
        string token;
        ss >> token; // consume "position"

        ss >> token; // "startpos" or "fen"
        if (token == "startpos") {
            UCI::init();
            ss >> token; // consume "moves"
        } else if (token == "fen") {
            string fen;
            while (ss >> token && token != "moves") {
                fen += token + " ";
            }
            UCI::board.init(fen);
            // token is already "moves" here, don't read again
        }

        if (token == "moves") {
            while (ss >> token) {
                Move move = utils::stringToMove(UCI::board, token);
                UCI::board.makeMove(move);
            }
        }
    }
    
    // parse go uci command
    void handleGo(string line) {
        // time controls
        int wtime = 0, btime = 0, winc = 0, binc = 0;
        
        // set value properly for all time controls passed
        istringstream ss(line);
        string token;
        while (ss >> token) {
            if (token == "wtime") ss >> wtime;
            else if (token == "btime") ss >> btime;
            else if (token == "winc") ss >> winc;
            else if (token == "binc") ss >> binc;
        }
        
        // calculate allocated time
        int myTime = (UCI::board.sideToMove == WHITE) ? wtime : btime;
        int myInc  = (UCI::board.sideToMove == WHITE) ? winc  : binc;
        
        int allocatedMs = (myTime / 30) + (myInc / 2);
        
        Move best = Engine::getBestMove(UCI::board, allocatedMs);
        cout << "bestmove " << utils::moveToString(best) << endl;
    }
}

namespace UCI {
    Board board;
    void init() {
        board.init();
    }
    void loop() {
        std::string line;
        while (std::getline(std::cin, line)) {
            if (line == "uci") handleUCI();
            else if (line == "isready") handleIsReady();
            else if (line == "ucinewgame") handleNewGame();
            else if (line.starts_with("position")) handlePosition(line);
            else if (line.starts_with("go")) handleGo(line);
            else if (line == "quit") break;
        }
    }
}