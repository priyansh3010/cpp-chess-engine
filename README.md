# ♟️ Chess Engine

A competitive chess engine written in **C++**, built from scratch with a focus on performance, correctness, and competitive strength. Implements the full UCI protocol for compatibility with chess GUIs and bot-vs-bot testing platforms.

---

## 🚀 Features

- **Minimax with Alpha-Beta Pruning** - classical game tree search, significantly reducing the number of nodes evaluated
- **Iterative Deepening** - progressively deepens search within time constraints, allowing best-effort moves even under tight clocks
- **Quiescence Search** - extends search at leaf nodes for captures/checks, avoiding the horizon effect
- **MVV-LVA Move Ordering** - Most Valuable Victim / Least Valuable Attacker heuristic to explore the best moves first and improve pruning efficiency
- **Piece-Square Tables** - positional evaluation bonuses to guide piece development and strategic play
- **Bitboard Representation** - 64-bit integers encode board state for fast, cache-friendly move generation and evaluation
- **Zobrist Hashing** - incremental hash updates for efficient transposition detection
- **Threefold Repetition Detection** - correct handling of draw conditions per FIDE rules
- **Killer Moves** - seeing quiet moves that create an overwhelming threat to the opponents position
- **UCI Protocol Support** - fully compatible with UCI-based GUIs (e.g. Arena, Cute Chess) and testing tools

---

## 📊 Performance

| Metric | Value |
|---|---|
| Search Speed | **5–10M nodes/second** |
| Move Generation | Verified via perft testing |
| Protocol | UCI-compliant |

---

## 🧪 Testing & Validation

Move generation correctness is validated using **perft testing** - a standard technique that counts all leaf nodes at a given depth and compares against known-good values. This ensures there are no illegal move generation bugs or missed edge cases (castling, en passant, promotions).

---

## 🛠️ Tech Stack

| Tool | Purpose |
|---|---|
| C++ | Core engine implementation |
| Bitboards | Board representation & move generation |
| UCI Protocol | GUI/bot interface standard |
| Git | Version control |

---

## 📁 Project Structure

```
chess-engine/
├── src/
│   ├── main.cpp            # Entry point; initialises engine and starts UCI loop
│   ├── types.h             # Core type aliases and enums (Color, Piece, Square, Bitboard, etc.)
│   ├── move.h              # Move struct definition (including all necessary flags)
│   ├── moveinfo.h          # Auxiliary move metadata for unmake (captured piece, castling rights, en passant)
│   ├── board.cpp/.h        # Board state representation, Zobrist hashing, make/unmake move
│   ├── movegen.cpp/.h      # Bitboard-based legal/pseudo-legal move generation
│   ├── evaluation.cpp/.h   # Static evaluation — material, game-phase piece-square tables
│   ├── engine.cpp/.h       # Search — minimax, alpha-beta pruning, iterative deepening, move ordering, quinscence search, killer moves
│   ├── perft.cpp/.h        # Perft testing — node counting to verify movegen correctness
│   ├── uci.cpp/.h          # UCI protocol handler — parses position, go, ucinewgame, etc.
│   └── utils.cpp/.h        # Shared helpers — string to move and vice versa parsing
└── README.md
└── build.bat
```

---

## ⚙️ Build & Run

```bash
# Clone the repo
git clone https://github.com/priyansh3010/cpp-chess-engine.git
cd chess-engine

# Build with build.bat
build

# Run the engine (UCI mode)
./engine.exe
```

The engine communicates via standard UCI commands over stdin/stdout. Connect it to any UCI-compatible GUI or use the `position` / `go` commands directly.

---

## 🔗 References

- [Chess Programming Wiki](https://www.chessprogramming.org/Main_Page)
- [UCI Protocol Specification](https://www.shredderchess.com/download/div/uci.zip)
- [Stockfish Perft Results](https://www.chessprogramming.org/Perft_Results)
