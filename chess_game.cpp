#include <iostream>
#include <vector>
#include <memory>
#include <utility>
#include <limits>
#include <stdexcept>
#include <cmath>
#include <cctype>
using namespace std;

static const int BOARD_SIZE = 8, MIN_IDX = 0, MAX_IDX = BOARD_SIZE - 1;

class Piece;
class ChessBoardInterface
{
public:
    virtual ~ChessBoardInterface() {}
    virtual Piece *at(pair<int, int> p) const = 0;
    virtual bool isSquareAttacked(pair<int, int> sq, bool byColor) const = 0;
    virtual bool isCheck(bool color) const = 0;
};

class Piece
{
protected:
    bool color_, alive_;
    pair<int, int> pos_;
    bool first_move_;

    static bool pathClear(pair<int, int> f, pair<int, int> t, const ChessBoardInterface &b)
    {
        int dr = (t.first > f.first ? 1 : (t.first < f.first ? -1 : 0));
        int dc = (t.second > f.second ? 1 : (t.second < f.second ? -1 : 0));
        int r = f.first + dr, c = f.second + dc;
        while (r != t.first || c != t.second)
        {
            Piece *p = b.at(make_pair(r, c));
            if (p && p->isAlive())
                return false;
            r += dr;
            c += dc;
        }
        return true;
    }

public:
    Piece(bool c, pair<int, int> p)
        : color_(c), alive_(true), pos_(p), first_move_(true) {}
    virtual ~Piece() {}

    bool color() const { return color_; }
    bool isAlive() const { return alive_; }
    pair<int, int> position() const { return pos_; }
    bool isFirstMove() const { return first_move_; }
    virtual char symbol() const = 0;

    void kill() { alive_ = false; }
    void markMoved() { first_move_ = false; }
    void setPosition(pair<int, int> p) { pos_ = p; }

    static bool isOutOfBounds(pair<int, int> p)
    {
        return p.first < MIN_IDX || p.first > MAX_IDX || p.second < MIN_IDX || p.second > MAX_IDX;
    }

    virtual unique_ptr<Piece> clone() const = 0;
    virtual bool isValidMove(pair<int, int> to, const ChessBoardInterface &b) const = 0;
    virtual bool attacksSquare(pair<int, int> to, const ChessBoardInterface &b) const = 0;
};

class Blank final : public Piece
{
public:
    Blank(pair<int, int> p) : Piece(false, p) { alive_ = false; }
    unique_ptr<Piece> clone() const override
    {
        return unique_ptr<Piece>(new Blank(*this));
    }
    bool isValidMove(pair<int, int>, const ChessBoardInterface &) const override
    {
        return false;
    }
    bool attacksSquare(pair<int, int>, const ChessBoardInterface &) const override
    {
        return false;
    }
    char symbol() const override { return '.'; }
};

class Rook final : public Piece
{
public:
    Rook(bool c, pair<int, int> p) : Piece(c, p) {}
    char symbol() const override { return 'R'; }
    unique_ptr<Piece> clone() const override
    {
        return unique_ptr<Piece>(new Rook(*this));
    }
    bool isValidMove(pair<int, int> to, const ChessBoardInterface &b) const override
    {
        if (!alive_ || isOutOfBounds(to) || to == pos_)
            return false;
        if ((pos_.first == to.first || pos_.second == to.second) && pathClear(pos_, to, b))
        {
            Piece *t = b.at(to);
            if (!t || !t->isAlive() || t->color() != color_)
                return true;
        }
        return false;
    }
    bool attacksSquare(pair<int, int> to, const ChessBoardInterface &b) const override
    {
        if (isOutOfBounds(to) || to == pos_)
            return false;
        return (pos_.first == to.first || pos_.second == to.second) && pathClear(pos_, to, b);
    }
};

class Knight final : public Piece
{
public:
    Knight(bool c, pair<int, int> p) : Piece(c, p) {}
    char symbol() const override { return 'N'; }
    unique_ptr<Piece> clone() const override
    {
        return unique_ptr<Piece>(new Knight(*this));
    }
    bool isValidMove(pair<int, int> to, const ChessBoardInterface &b) const override
    {
        if (!alive_ || isOutOfBounds(to))
            return false;
        int dr = abs(to.first - pos_.first), dc = abs(to.second - pos_.second);
        if ((dr == 2 && dc == 1) || (dr == 1 && dc == 2))
        {
            Piece *t = b.at(to);
            if (!t || !t->isAlive() || t->color() != color_)
                return true;
        }
        return false;
    }
    bool attacksSquare(pair<int, int> to, const ChessBoardInterface &) const override
    {
        if (isOutOfBounds(to))
            return false;
        int dr = abs(to.first - pos_.first), dc = abs(to.second - pos_.second);
        return (dr == 2 && dc == 1) || (dr == 1 && dc == 2);
    }
};

class Bishop final : public Piece
{
public:
    Bishop(bool c, pair<int, int> p) : Piece(c, p) {}
    char symbol() const override { return 'B'; }
    unique_ptr<Piece> clone() const override
    {
        return unique_ptr<Piece>(new Bishop(*this));
    }
    bool isValidMove(pair<int, int> to, const ChessBoardInterface &b) const override
    {
        if (!alive_ || isOutOfBounds(to) || to == pos_)
            return false;
        int dr = abs(to.first - pos_.first), dc = abs(to.second - pos_.second);
        if (dr == dc && pathClear(pos_, to, b))
        {
            Piece *t = b.at(to);
            if (!t || !t->isAlive() || t->color() != color_)
                return true;
        }
        return false;
    }
    bool attacksSquare(pair<int, int> to, const ChessBoardInterface &b) const override
    {
        if (isOutOfBounds(to) || to == pos_)
            return false;
        int dr = abs(to.first - pos_.first), dc = abs(to.second - pos_.second);
        return dr == dc && pathClear(pos_, to, b);
    }
};

class Queen final : public Piece
{
public:
    Queen(bool c, pair<int, int> p) : Piece(c, p) {}
    char symbol() const override { return 'Q'; }
    unique_ptr<Piece> clone() const override
    {
        return unique_ptr<Piece>(new Queen(*this));
    }
    bool isValidMove(pair<int, int> to, const ChessBoardInterface &b) const override
    {
        if (!alive_ || isOutOfBounds(to) || to == pos_)
            return false;
        int dr = abs(to.first - pos_.first), dc = abs(to.second - pos_.second);
        if ((dr == dc || dr == 0 || dc == 0) && pathClear(pos_, to, b))
        {
            Piece *t = b.at(to);
            if (!t || !t->isAlive() || t->color() != color_)
                return true;
        }
        return false;
    }
    bool attacksSquare(pair<int, int> to, const ChessBoardInterface &b) const override
    {
        if (isOutOfBounds(to) || to == pos_)
            return false;
        int dr = abs(to.first - pos_.first), dc = abs(to.second - pos_.second);
        return (dr == dc || dr == 0 || dc == 0) && pathClear(pos_, to, b);
    }
};

class King final : public Piece
{
public:
    King(bool c, pair<int, int> p) : Piece(c, p) {}
    char symbol() const override { return 'K'; }
    unique_ptr<Piece> clone() const override
    {
        return unique_ptr<Piece>(new King(*this));
    }
    bool isValidMove(pair<int, int> to, const ChessBoardInterface &b) const override
    {
        if (!alive_ || isOutOfBounds(to))
            return false;
        int dr = abs(to.first - pos_.first), dc = abs(to.second - pos_.second);
        if ((dr <= 1 && dc <= 1) && (dr + dc > 0))
        {
            Piece *t = b.at(to);
            if (!t || !t->isAlive() || t->color() != color_)
                return true;
        }
        if (first_move_ && dr == 0 && dc == 2 && !b.isCheck(color_))
        {
            int dir = (to.second > pos_.second ? 1 : -1),
                rookC = (dir > 0 ? MAX_IDX : MIN_IDX);
            Piece *rp = b.at(make_pair(pos_.first, rookC));
            if (rp && rp->isFirstMove() && dynamic_cast<Rook *>(rp))
            {
                if (pathClear(pos_, make_pair(pos_.first, rookC), b) && !b.isSquareAttacked(make_pair(pos_.first, pos_.second + dir), !color_))
                    return true;
            }
        }
        return false;
    }
    bool attacksSquare(pair<int, int> to, const ChessBoardInterface &) const override
    {
        if (isOutOfBounds(to))
            return false;
        int dr = abs(to.first - pos_.first), dc = abs(to.second - pos_.second);
        return (dr <= 1 && dc <= 1) && (dr + dc > 0);
    }
};

class Pawn final : public Piece
{
    bool enPassantAvailable_;

public:
    Pawn(bool c, pair<int, int> p) : Piece(c, p), enPassantAvailable_(false) {}
    char symbol() const override { return 'P'; }
    unique_ptr<Piece> clone() const override
    {
        Pawn *np = new Pawn(*this);
        np->enPassantAvailable_ = enPassantAvailable_;
        return unique_ptr<Piece>(np);
    }
    bool isValidMove(pair<int, int> to, const ChessBoardInterface &b) const override
    {
        if (!alive_ || isOutOfBounds(to))
            return false;
        int dir = color_ ? -1 : 1, dr = to.first - pos_.first, dc = to.second - pos_.second;
        Piece *tgt = b.at(to);
        if (dc == 0 && dr == dir && (!tgt || !tgt->isAlive()))
            return true;
        if (dc == 0 && dr == 2 * dir && first_move_)
        {
            auto mid = make_pair(pos_.first + dir, pos_.second);
            if ((!b.at(mid) || !b.at(mid)->isAlive()) && (!tgt || !tgt->isAlive()))
                return true;
        }
        if (abs(dc) == 1 && dr == dir)
        {
            if (tgt && tgt->isAlive() && tgt->color() != color_)
                return true;
            Piece *side = b.at(make_pair(pos_.first, to.second));
            if (side)
            {
                Pawn *ep = dynamic_cast<Pawn *>(side);
                if (ep && ep->color() != color_ && ep->enPassantAvailable_)
                    return true;
            }
        }
        return false;
    }
    bool attacksSquare(pair<int, int> to, const ChessBoardInterface &) const override
    {
        if (isOutOfBounds(to) || !alive_)
            return false;
        int dir = color_ ? -1 : 1, dr = to.first - pos_.first, dc = to.second - pos_.second;
        return abs(dc) == 1 && dr == dir;
    }
    void setEnPassantAvailable(bool v) { enPassantAvailable_ = v; }
};

class Board : public ChessBoardInterface
{
    vector<vector<unique_ptr<Piece>>> grid_;
    bool turnColor_;

public:
    Board() : turnColor_(false)
    {
        grid_.resize(BOARD_SIZE);
        for (int r = 0; r < BOARD_SIZE; r++)
        {
            grid_[r].resize(BOARD_SIZE);
            for (int c = 0; c < BOARD_SIZE; c++)
            {
                if (r == 0)
                {
                    if (c == 0 || c == 7)
                        grid_[r][c].reset(new Rook(false, {r, c}));
                    else if (c == 1 || c == 6)
                        grid_[r][c].reset(new Knight(false, {r, c}));
                    else if (c == 2 || c == 5)
                        grid_[r][c].reset(new Bishop(false, {r, c}));
                    else if (c == 3)
                        grid_[r][c].reset(new Queen(false, {r, c}));
                    else
                        grid_[r][c].reset(new King(false, {r, c}));
                }
                else if (r == 1)
                    grid_[r][c].reset(new Pawn(false, {r, c}));
                else if (r == 6)
                    grid_[r][c].reset(new Pawn(true, {r, c}));
                else if (r == 7)
                {
                    if (c == 0 || c == 7)
                        grid_[r][c].reset(new Rook(true, {r, c}));
                    else if (c == 1 || c == 6)
                        grid_[r][c].reset(new Knight(true, {r, c}));
                    else if (c == 2 || c == 5)
                        grid_[r][c].reset(new Bishop(true, {r, c}));
                    else if (c == 3)
                        grid_[r][c].reset(new Queen(true, {r, c}));
                    else
                        grid_[r][c].reset(new King(true, {r, c}));
                }
                else
                    grid_[r][c].reset(new Blank({r, c}));
            }
        }
    }
    Board(const Board &o) : turnColor_(o.turnColor_)
    {
        grid_.resize(BOARD_SIZE);
        for (int r = 0; r < BOARD_SIZE; r++)
        {
            grid_[r].resize(BOARD_SIZE);
            for (int c = 0; c < BOARD_SIZE; c++)
                grid_[r][c] = o.grid_[r][c]->clone();
        }
    }

    void print() const
    {
        const string sep = "  +----+----+----+----+----+----+----+----+";
        cout << "\n"
             << sep << "\n";
        for (int r = MAX_IDX; r >= MIN_IDX; --r)
        {
            cout << " " << r << " ";
            for (int c = MIN_IDX; c <= MAX_IDX; ++c)
            {
                Piece *p = at(make_pair(r, c));
                if (!p || !p->isAlive())
                {
                    cout << "|  . ";
                }
                else
                {
                    char col = p->color() ? 'b' : 'w';
                    char sym = toupper(p->symbol());
                    cout << "| " << col << sym << " ";
                }
            }
            cout << "|\n"
                 << sep << "\n";
        }
        cout << "    ";
        for (int c = 0; c < BOARD_SIZE; ++c)
        {
            cout << "  " << c << "  ";
        }
        cout << "\n\n";
    }

    bool playTurn()
    {
        print();
        cout << (turnColor_ ? "Black" : "White") << " to move.\n";
        if (isCheck(turnColor_))
            cout << "You are in check!\n";
        if (isCheckmate(turnColor_))
        {
            cout << (turnColor_ ? "Black" : "White") << " is checkmated. "
                 << (turnColor_ ? "White" : "Black") << " wins!\n";
            return false;
        }
        if (isStalemate(turnColor_))
        {
            cout << "Stalemate. Draw!\n";
            return false;
        }
        int r1, c1, r2, c2;
        while (true)
        {
            cout << "Enter move (r1 c1 r2 c2): ";
            if (!(cin >> r1 >> c1 >> r2 >> c2))
            {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Bad input format.\n";
                continue;
            }
            pair<int, int> from(r1, c1), to(r2, c2);
            if (Piece::isOutOfBounds(from) || Piece::isOutOfBounds(to))
            {
                cout << "Move out of bounds.\n";
                continue;
            }
            Piece *p = at(from);
            if (!p || !p->isAlive() || p->color() != turnColor_)
            {
                cout << "Invalid source.\n";
                continue;
            }
            if (!p->isValidMove(to, *this))
            {
                cout << "Illegal move for that piece.\n";
                continue;
            }
            Board copy = *this;
            copy.movePiece(from, to);
            if (copy.isCheck(turnColor_))
            {
                cout << "Leaves king in check.\n";
                continue;
            }
            movePiece(from, to);
            break;
        }
        turnColor_ = !turnColor_;
        return true;
    }

    Piece *at(pair<int, int> p) const override
    {
        if (Piece::isOutOfBounds(p))
            return nullptr;
        return grid_[p.first][p.second].get();
    }

    bool isSquareAttacked(pair<int, int> sq, bool byColor) const override
    {
        for (int r = 0; r < BOARD_SIZE; r++)
            for (int c = 0; c < BOARD_SIZE; c++)
            {
                Piece *p = at(make_pair(r, c));
                if (p && p->isAlive() && p->color() == byColor && p->attacksSquare(sq, *this))
                    return true;
            }
        return false;
    }

    bool isCheck(bool color) const override
    {
        pair<int, int> kp = findKing(color);
        return kp.first >= 0 && isSquareAttacked(kp, !color);
    }

    void movePiece(pair<int, int> from, pair<int, int> to)
    {
        for (int r = 0; r < BOARD_SIZE; r++)
            for (int c = 0; c < BOARD_SIZE; c++)
            {
                Piece *p = at(make_pair(r, c));
                Pawn *pp = dynamic_cast<Pawn *>(p);
                if (pp)
                    pp->setEnPassantAvailable(false);
            }
        Piece *p = at(from);
        if (!p)
            return;
        King *k = dynamic_cast<King *>(p);
        if (k)
        {
            if (abs(to.second - from.second) == 2)
            {
                int dir = (to.second > from.second ? 1 : -1);
                int rc = (dir > 0 ? MAX_IDX : MIN_IDX);
                movePiece(make_pair(from.first, rc),
                          make_pair(from.first, from.second + dir));
            }
        }
        Pawn *pawn = dynamic_cast<Pawn *>(p);
        if (pawn)
        {
            int dr = to.first - from.first, dc = to.second - from.second;
            if (pawn->isFirstMove() && abs(dr) == 2)
                pawn->setEnPassantAvailable(true);
            if (abs(dc) == 1 && !at(to)->isAlive())
            {
                grid_[from.first][to.second].reset(new Blank(make_pair(from.first, to.second)));
            }
        }
        auto &src = grid_[from.first][from.second];
        auto &dst = grid_[to.first][to.second];
        if (dst->isAlive())
            dst->kill();
        dst = move(src);
        dst->setPosition(to);
        dst->markMoved();
        src.reset(new Blank(from));
        pawn = dynamic_cast<Pawn *>(at(to));
        if (pawn)
        {
            if ((!pawn->color() && to.first == MAX_IDX) ||
                (pawn->color() && to.first == MIN_IDX))
            {
                bool c = pawn->color();
                grid_[to.first][to.second].reset(new Queen(c, to));
                cout << "Pawn promoted to Queen!\n";
            }
        }
    }

    pair<int, int> findKing(bool color) const
    {
        for (int r = 0; r < BOARD_SIZE; r++)
            for (int c = 0; c < BOARD_SIZE; c++)
            {
                Piece *p = at(make_pair(r, c));
                King *k = dynamic_cast<King *>(p);
                if (k && k->isAlive() && k->color() == color)
                    return make_pair(r, c);
            }
        return make_pair(-1, -1);
    }

    bool isCheckmate(bool color) const
    {
        if (!isCheck(color))
            return false;
        for (int r = 0; r < BOARD_SIZE; r++)
            for (int c = 0; c < BOARD_SIZE; c++)
            {
                Piece *p = at(make_pair(r, c));
                if (p && p->isAlive() && p->color() == color)
                {
                    for (int r2 = 0; r2 < BOARD_SIZE; r2++)
                        for (int c2 = 0; c2 < BOARD_SIZE; c2++)
                        {
                            pair<int, int> to(r2, c2);
                            if (p->isValidMove(to, *this))
                            {
                                Board copy = *this;
                                copy.movePiece(make_pair(r, c), to);
                                if (!copy.isCheck(color))
                                    return false;
                            }
                        }
                }
            }
        return true;
    }

    bool isStalemate(bool color) const
    {
        if (isCheck(color))
            return false;
        for (int r = 0; r < BOARD_SIZE; r++)
            for (int c = 0; c < BOARD_SIZE; c++)
            {
                Piece *p = at(make_pair(r, c));
                if (p && p->isAlive() && p->color() == color)
                {
                    for (int r2 = 0; r2 < BOARD_SIZE; r2++)
                        for (int c2 = 0; c2 < BOARD_SIZE; c2++)
                        {
                            pair<int, int> to(r2, c2);
                            if (p->isValidMove(to, *this))
                            {
                                Board copy = *this;
                                copy.movePiece(make_pair(r, c), to);
                                if (!copy.isCheck(color))
                                    return false;
                            }
                        }
                }
            }
        return true;
    }
};

int main()
{
    try
    {
        Board game;
        while (game.playTurn())
            ;
        cout << "Game over.\n";
    }
    catch (const exception &e)
    {
        cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
