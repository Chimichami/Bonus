
#include <stdexcept>
#include <cstdlib>
#include <string>
#include "token.h"
#include "scanner.h"
#include "ast.h"
#include "parser.h"
using namespace std;

Parser::Parser(Scanner* s):scanner(s),current(nullptr),previous(nullptr){ advance(); }

bool Parser::advance(){ previous = current; current = scanner->nextToken(); return true; }
bool Parser::check(Token::Type t) const { return current && current->type==t; }
bool Parser::match(Token::Type t){ if (check(t)){ advance(); return true; } return false; }
bool Parser::isAtEnd() const { return current && current->type==Token::END; }
void Parser::consume(Token::Type t, const char* msg){ if (!match(t)) throw runtime_error(msg); }

Token* Parser::peek() {
    if (!look) look = scanner->nextToken();
    return look;
}

bool Parser::advance() {
    previous = current;
    if (look) { current = look; look = nullptr; }
    else { current = scanner->nextToken(); }
    return true;
}

Program* Parser::parseProgram(){
    auto* prog = new Program();
    prog->slist.push_back(parseStm());
    while (match(Token::SEMICOL)) {
        if (isAtEnd()) break;
        prog->slist.push_back(parseStm());
    }
    if (!isAtEnd()) throw runtime_error("Basura después del último statement");
    return prog;
}

Stm* Parser::parseStm(){
    if (match(Token::PRINT)) {
        consume(Token::LPAREN, "Se esperaba '(' tras print");
        CExp* e = parseCExp();
        consume(Token::RPAREN, "Se esperaba ')' al cerrar print(");
        return new PrintStm(e);
    }
    if (match(Token::ID)) {
        string name = previous->text;
        consume(Token::ASSIGN, "Se esperaba '=' en asignación");
        CExp* rhs = parseCExp();
        return new AssignStm(name, rhs);
    }
    throw runtime_error("Stmt inválido");
}

// ---------- CExp ----------
CExp* Parser::parseCExp() {
    // 1) Set literal obvio
    if (check(Token::LBRACE)) {
        return new CExp(parseSetExpr());
    }

    // 2) '(' podría ser (Expr) o (SetExpr)
    if (check(Token::LPAREN)) {
        Token* t1 = peek();
        // Si lo siguiente es '{', interpretamos como (SetExpr)
        if (t1 && t1->type == Token::LBRACE) {
            return new CExp(parseSetExpr());
        }
        // En caso contrario, lo tratamos como Expr
        return new CExp(parseExpr());
    }

    // 3) ID puede ser ambos; decide por el operador que sigue (sin consumir)
    if (check(Token::ID)) {
        Token* t1 = peek();
        if (t1 && (t1->type == Token::UNION ||
                   t1->type == Token::INTERSECT ||
                   t1->type == Token::DIFF)) {
            // Ej: id cup {...}, id cap id, id \ {..}
            return new CExp(parseSetExpr());
                   }
        // Por defecto, aritmética (id solo o seguido de +,-,*,/,),;,etc.)
        return new CExp(parseExpr());
    }

    // 4) NUM, '-', 'sqrt', etc. => aritmética
    return new CExp(parseExpr());
}

// ---------- Expr ----------
Exp* Parser::parseExpr(){
    Exp* left = parseTerm();
    while (match(Token::PLUS) || match(Token::MINUS)) {
        BinaryOp op = (previous->type==Token::PLUS)?PLUS_OP:MINUS_OP;
        Exp* right = parseTerm();
        left = new BinaryExp(left,right,op);
    }
    return left;
}

Exp* Parser::parseTerm(){
    Exp* left = parseFactor();
    while (match(Token::MUL) || match(Token::DIV)) {
        BinaryOp op = (previous->type==Token::MUL)?MUL_OP:DIV_OP;
        Exp* right = parseFactor();
        left = new BinaryExp(left,right,op);
    }
    return left;
}

Exp* Parser::parseFactor(){
    if (match(Token::MINUS)) {
        Exp* inner = parseFactor();
        return new BinaryExp(new NumberExp(0), inner, MINUS_OP);
    }
    if (match(Token::NUM))    return new NumberExp(std::atoi(previous->text.c_str()));
    if (match(Token::ID))     return new IdExp(previous->text);
    if (match(Token::SQRT)) { consume(Token::LPAREN,"Se esperaba '(' tras sqrt"); Exp* e=parseExpr(); consume(Token::RPAREN,"Falta ')'"); return new SqrtExp(e); }
    if (match(Token::LPAREN)) { Exp* e = parseExpr(); consume(Token::RPAREN,"Falta ')'"); return e; }
    throw runtime_error("Factor inválido");
}

// ---------- SetExpr ----------
SetExp* Parser::parseSetExpr(){
    SetExp* left = parseSetTerm();
    while (match(Token::UNION) || match(Token::INTERSECT) || match(Token::DIFF)) {
        SetOp op = (previous->type==Token::UNION)?UNION_OP : (previous->type==Token::INTERSECT)?INTERSECT_OP : DIFF_OP;
        SetExp* right = parseSetTerm();
        left = new SetBinaryExp(left,right,op);
    }
    return left;
}
SetExp* Parser::parseSetTerm(){ return parseSetFactor(); }

SetExp* Parser::parseSetFactor(){
    if (check(Token::LBRACE)) return parseSet();
    if (match(Token::ID))     return new SetIdExp(previous->text);
    if (match(Token::LPAREN)) { SetExp* inner = parseSetExpr(); consume(Token::RPAREN,"Falta ')' en (SetExpr)"); return new SetParenExp(inner); }
    throw runtime_error("SetFactor inválido");
}

SetExp* Parser::parseSet(){
    consume(Token::LBRACE,"Falta '{'");
    std::vector<CExp*> elems;
    if (!check(Token::RBRACE)) {
        elems.push_back(parseCExp());
        while (match(Token::COMMA)) elems.push_back(parseCExp());
    }
    consume(Token::RBRACE,"Falta '}'");
    return new SetLiteralExp(std::move(elems));
}
