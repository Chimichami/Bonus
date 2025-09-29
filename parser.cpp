#include <iostream>
#include <stdexcept>
#include "token.h"
#include "scanner.h"
#include "ast.h"
#include "parser.h"

using namespace std;

// =============================
// Métodos de la clase Parser
// =============================

Parser::Parser(Scanner* sc) : scanner(sc) {
    previous = nullptr;
    current = scanner->nextToken();
    if (current->type == Token::ERR) {
        throw runtime_error("Error léxico");
    }
}

bool Parser::match(Token::Type ttype) {
    if (check(ttype)) {
        advance();
        return true;
    }
    return false;
}

bool Parser::check(Token::Type ttype) {
    if (isAtEnd()) return false;
    return current->type == ttype;
}

bool Parser::advance() {
    if (!isAtEnd()) {
        Token* temp = current;
        if (previous) delete previous;
        current = scanner->nextToken();
        previous = temp;

        if (check(Token::ERR)) {
            throw runtime_error("Error lexico");
        }
        return true;
    }
    return false;
}

bool Parser::isAtEnd() {
    return (current->type == Token::END);
}


// =============================
// Reglas gramaticales
// =============================

Program* Parser::parseProgram() {
    Program * programa = new Program();
    programa->slist.push_back(parseStm());
    while(match(Token::SEMICOL)) {
        programa->slist.push_back(parseStm());
    }

    if (!isAtEnd()) {
        throw runtime_error("Error sintáctico");
    }
    cout << "Parseo exitoso" << endl;
    return programa;
}

Stm* Parser::parseStm() {
    Stm* stm;
    Exp* e;
    string nombre;
    if(match(Token::PRINT)) {
        match(Token::LPAREN);
        e = parseCE();
        match(Token::RPAREN);
        return new PrintStm(e);
    }
    else if(match(Token::ID)) {
        nombre = previous->text;
        match(Token::ASSIGN);
        e = parseCE();
        return new AssignStm(nombre, e);
    }
    return stm;
}

Exp* Parser::parseCExp(int signal /* = -1 */) {
    // based on first elements of non terminal
    if (check(NUM) || signal == 0) { // arithmetic expr
        return parseExp();
    }
    else if (check(LCORCH) || signal == 1) { // set expr
        return parseSetExp();
    }
    else if (check(LPAR)) { // here we dont know!! its on both first sets of arithmetic
        size_t saved_pos = current; // current pos of parse, we are gonna do N lookahead XD
        signal = -1;
        do {
            if (match(NUM) || match(PLUS) || match(MINUS) || match(MUL) || match(DIV)) {
                signal = 0;
                break;
            }
            else if (match(LCORCH) || match(UNION) || match(INTERSECT) || match(DIFF)) {
                signal = 1;
                break;
            }
            else {
                advance(); // next token regardless we need to see an operator
            }
        } while (true);

        current = saved_pos;
        return parseCExp(signal);
    }
    else if (match(ID)) { // next token must be operator or its alone
        if (check(PLUS) || check(MINUS)) { // arithmetic
            current--;
            return parseCExp(0);
        }
        else if (check(UNION) || check(INTERSECT) || check(DIFF)) { // set
            current--;
            return parseCExp(1);
        }
        // si no hay operador después del ID, aquí puedes decidir qué hacer;
        // lo dejo como no-resuelto para respetar tu lógica original.
    }

    // fallback por si nada calza (puedes cambiarlo por manejo de error)
    return parseExp();
}

Exp* Parser::parseExp() {
    Exp* left;
    Exp* right;

    left = parseTerm();

    while (match(PLUS) || match(MINUS)) {
        auto op = BinaryOp(previous()->lexema);  // token consumido por match()
        right = parseTerm();
        left = new BinaryExp(left, right, op);
    }

    return left;
}



Exp* Parser::parseTerm() {
    Exp* left;
    Exp* right;

    left = parseFactor();

    while (match(MUL) || match(DIV)) {
        auto op = BinaryOp(previous()->lexema);  // token del operador recién consumido
        right = parseFactor();
        left = new BinaryExp(left, right, op);
    }

    return left;
}


Exp* Parser::parseFactor() {
    if (match(NUM)) {
        return new NumExp(std::atoi(previous()->lexema.c_str()));
    }

    if (match(LPAR)) {
        Exp* left = parseExp();
        match(RPAR);
        return left;
    }

    if (match(ID)) {
        return new IDExp(previous()->lexema);
    }

    // opcional: manejo de error / fallback
    return nullptr;
}

