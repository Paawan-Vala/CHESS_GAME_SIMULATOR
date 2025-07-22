# CHESS_GAME_SIMULATOR

## Project Overview  
A simple two‑player chess game in C++11. Players alternate entering moves in the form `r1 c1 r2 c2` (rows and columns 0–7). The program enforces all official rules—castling, en passant, pawn promotion, and detection of check, checkmate, or stalemate. There is no computer opponent.

---

## Architecture  

- **ChessBoardInterface**  
  - `Piece* at(pair<int,int>)`  
  - `bool isSquareAttacked(pair<int,int>, bool)`  
  - `bool isCheck(bool)`  

- **Piece** (abstract)  
  - **State** (`protected`):  
    - `bool color_` (false=White, true=Black)  
    - `bool alive_`  
    - `pair<int,int> pos_`  
    - `bool first_move_`  
  - **Virtual API** (`public`):  
    - `char symbol() const`  
    - `unique_ptr<Piece> clone() const`  
    - `bool isValidMove(pair<int,int>, const ChessBoardInterface&) const`  
    - `bool attacksSquare(pair<int,int>, const ChessBoardInterface&) const`  
  - **Helpers** (`static`): `pathClear()`, `isOutOfBounds()`

- **Derived Pieces**  
  - `Blank` (empty square)  
  - `Pawn` (forward moves, captures, en passant, promotion)  
  - `Knight`, `Bishop`, `Rook`, `Queen` (standard moves)  
  - `King` (single‑step, castling)

- **Board** (implements ChessBoardInterface)  
  - Maintains `vector<vector<unique_ptr<Piece>>> grid_`  
  - Tracks `bool turnColor_`  
  - `playTurn()`: resets en passant flags, prints board, reads & validates move, simulates on copy, executes legal move  
  - Utility methods: `print()`, `movePiece()`, `findKing()`, `isCheckmate()`, `isStalemate()`, `clearEnPassantFlags()`

---

## Key OOP Principles  

- **Encapsulation**  
  Piece internals are `protected` or `private`; external code uses public accessors.  
- **Abstraction**  
  Pieces interact with the board solely through the `ChessBoardInterface`.  
- **Polymorphism**  
  The board stores base-class pointers and invokes derived-class behavior at runtime.  
- **Single Responsibility**  
  Each class handles its own logic: pieces manage move rules; `Board` manages game flow.

---

## Building & Running  

```bash
g++ -std=c++11 -Wall Chess.cpp -o chess
./chess
