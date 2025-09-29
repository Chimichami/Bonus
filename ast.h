#ifndef AST_H
#define AST_H
#include <vector>
#include <set>
#include <string>

struct Value {
    enum Kind { INT, SET } kind;
    int i = 0;
    std::set<int> s;

    static Value fromInt(int v){ Value x; x.kind=INT; x.i=v; return x; }
    static Value fromSet(std::set<int> v){ Value x; x.kind=SET; x.s=std::move(v); return x; }
};

struct Visitor; // fwd

// ---- expresiones aritméticas
enum BinaryOp { PLUS_OP, MINUS_OP, MUL_OP, DIV_OP, POW_OP };

struct Exp { virtual ~Exp(){}; virtual Value accept(Visitor* v)=0; };
struct NumberExp : Exp { int value; NumberExp(int v):value(v){} Value accept(Visitor* v) override; };
struct IdExp     : Exp { std::string name; IdExp(std::string n):name(std::move(n)){} Value accept(Visitor* v) override; };
struct BinaryExp : Exp { Exp* left; Exp* right; BinaryOp op; BinaryExp(Exp*l,Exp*r,BinaryOp o):left(l),right(r),op(o){} Value accept(Visitor* v) override; };
struct SqrtExp   : Exp { Exp* inner; SqrtExp(Exp* e):inner(e){} Value accept(Visitor* v) override; }; // opcional

// ---- expresiones de conjunto
enum SetOp { UNION_OP, INTERSECT_OP, DIFF_OP };

struct SetExp { virtual ~SetExp(){}; virtual Value accept(Visitor* v)=0; };
struct SetIdExp     : SetExp { std::string name; SetIdExp(std::string n):name(std::move(n)){} Value accept(Visitor* v) override; };
struct SetParenExp  : SetExp { SetExp* inner; SetParenExp(SetExp* i):inner(i){} Value accept(Visitor* v) override; };
struct SetBinaryExp : SetExp { SetExp* left; SetExp* right; SetOp op; SetBinaryExp(SetExp*l,SetExp*r,SetOp o):left(l),right(r),op(o){} Value accept(Visitor* v) override; };

// Set literal: elementos son CExp (seman.: deben ser INT)
struct CExp; // fwd
struct SetLiteralExp : SetExp {
    std::vector<CExp*> elems;
    explicit SetLiteralExp(std::vector<CExp*> es):elems(std::move(es)){}
    Value accept(Visitor* v) override;
};

// ---- CExp wrapper (elige rama aritmética o de conjunto)
struct CExp {
    Exp* a = nullptr;      // si no es null => Expr
    SetExp* s = nullptr;   // si no es null => SetExpr
    explicit CExp(Exp* e): a(e) {}
    explicit CExp(SetExp* z): s(z) {}
    Value accept(Visitor* v); // delega
};

// ---- sentencias y programa
struct Stm { virtual ~Stm(){}; virtual void accept(Visitor* v)=0; };
struct AssignStm : Stm { std::string id; CExp* rhs; AssignStm(std::string i, CExp* r):id(std::move(i)),rhs(r){} void accept(Visitor* v) override; };
struct PrintStm  : Stm { CExp* e; PrintStm(CExp* x):e(x){} void accept(Visitor* v) override; };
struct Program   { std::vector<Stm*> slist; };

struct Visitor {
    virtual Value visit(NumberExp*)=0;
    virtual Value visit(IdExp*)=0;
    virtual Value visit(BinaryExp*)=0;
    virtual Value visit(SqrtExp*)=0;
    virtual Value visit(SetIdExp*)=0;
    virtual Value visit(SetParenExp*)=0;
    virtual Value visit(SetBinaryExp*)=0;
    virtual Value visit(SetLiteralExp*)=0;

    // helpers para stmts
    virtual void visit(AssignStm*)=0;
    virtual void visit(PrintStm*)=0;
};

#endif
