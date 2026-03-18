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
        } else if (token == "fen") {
            string fen;
            while (ss >> token && token != "moves") {
                fen += token + " ";
            }
            UCI::board.init(fen);
        }
        
        // if token is now "moves", replay them
        ss >> token; // "startpos" or "fen"
        if (token == "moves") {
            string lastMove = "";
            while (ss >> token) {
                Move move = utils::stringToMove(UCI::board, token);
                UCI::board.makeMove(move);
            }
        }
    }
    
    void handleGo(string line) {
        Move best = Engine::getBestMove(UCI::board);
        UCI::board.makeMove(best);
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