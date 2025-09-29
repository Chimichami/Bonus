#include <iostream>
#include <cmath>
#include "visitor.h"

static int asInt(const Value& v){
    if (v.kind != Value::INT) throw std::runtime_error("Se esperaba entero");
    return v.i;
}
static std::set<int> asSet(const Value& v){
    if (v.kind != Value::SET) throw std::runtime_error("Se esperaba conjunto");
    return v.s;
}
static void expectInt(const Value& v){ if (v.kind!=Value::INT) throw std::runtime_error("Elemento de set debe ser entero"); }

Value NumberExp::accept(Visitor* v){ return v->visit(this); }
Value IdExp::accept(Visitor* v){ return v->visit(this); }
Value BinaryExp::accept(Visitor* v){ return v->visit(this); }
Value SqrtExp::accept(Visitor* v){ return v->visit(this); }
Value SetIdExp::accept(Visitor* v){ return v->visit(this); }
Value SetParenExp::accept(Visitor* v){ return v->visit(this); }
Value SetBinaryExp::accept(Visitor* v){ return v->visit(this); }
Value SetLiteralExp::accept(Visitor* v){ return v->visit(this); }
Value CExp::accept(Visitor* v){ return a? a->accept(v) : s->accept(v); }

void AssignStm::accept(Visitor* v){ v->visit(this); }
void PrintStm::accept(Visitor* v){ v->visit(this); }

// ---- aritmética
Value EvalVisitor::visit(NumberExp* e){ return Value::fromInt(e->value); }

Value EvalVisitor::visit(IdExp* e){
    auto it = mem.find(e->name);
    if (it==mem.end()) return Value::fromInt(0); // o error si prefieres
    return it->second;
}

Value EvalVisitor::visit(BinaryExp* e){
    int L = asInt(e->left->accept(this));
    int R = asInt(e->right->accept(this));
    switch (e->op){
        case PLUS_OP:  return Value::fromInt(L+R);
        case MINUS_OP: return Value::fromInt(L-R);
        case MUL_OP:   return Value::fromInt(L*R);
        case DIV_OP:   if (R==0) throw std::runtime_error("División por cero"); else return Value::fromInt(L/R);
        case POW_OP:   return Value::fromInt((int)std::pow(L,R));
    }
    return Value::fromInt(0);
}

Value EvalVisitor::visit(SqrtExp* e){
    int v = asInt(e->inner->accept(this));
    if (v<0) throw std::runtime_error("sqrt de negativo");
    return Value::fromInt((int)std::sqrt((double)v));
}

// ---- conjuntos
Value EvalVisitor::visit(SetIdExp* e){
    auto it = mem.find(e->name);
    if (it==mem.end()) return Value::fromSet({});
    if (it->second.kind != Value::SET) throw std::runtime_error("Id no es conjunto");
    return it->second;
}
Value EvalVisitor::visit(SetParenExp* e){ return e->inner->accept(this); }

Value EvalVisitor::visit(SetLiteralExp* e){
    std::set<int> acc;
    for (auto ce : e->elems) {
        Value v = ce->accept(this);
        expectInt(v);
        acc.insert(v.i);
    }
    return Value::fromSet(std::move(acc));
}

Value EvalVisitor::visit(SetBinaryExp* e){
    std::set<int> A = asSet(e->left->accept(this));
    std::set<int> B = asSet(e->right->accept(this));
    std::set<int> R;
    switch (e->op){
        case UNION_OP:
            R = A; R.insert(B.begin(), B.end()); break;
        case INTERSECT_OP:
            for (int x: A) if (B.count(x)) R.insert(x); break;
        case DIFF_OP:
            for (int x: A) if (!B.count(x)) R.insert(x); break;
    }
    return Value::fromSet(std::move(R));
}

// ---- stmts
void EvalVisitor::visit(AssignStm* s){
    Value v = s->rhs->accept(this);
    mem[s->id] = v;
}

static void printValue(const Value& v){
    if (v.kind==Value::INT) { std::cout << v.i << "\n"; }
    else {
        std::cout << "{";
        bool first = true;
        for (int x: v.s){ if(!first) std::cout<<","; std::cout<<x; first=false; }
        std::cout << "}\n";
    }
}
void EvalVisitor::visit(PrintStm* s){
    Value v = s->e->accept(this);
    printValue(v);
}
