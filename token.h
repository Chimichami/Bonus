#ifndef TOKEN_H
#define TOKEN_H

#include <string>
#include <ostream>

using namespace std;

class Token {
public:
    enum Type {
        // aritmética
        PLUS, MINUS, MUL, DIV,
        // paréntesis
        LPAREN, RPAREN,
        // números e identificadores
        NUM, ID,
        // sentencia
        PRINT, ASSIGN, SEMICOL,
        // potencia y sqrt (opcional, puedes quitar si no lo usas)
        POW, SQRT,
        // conjuntos
        LBRACE, RBRACE, COMMA,   // { } ,
        UNION, INTERSECT, DIFF,  // cup cap \
        // misceláneo
        ERR, END
    };

    Type type;
    string text;

    Token(Type type);
    Token(Type type, char c);
    Token(Type type, const string& source, int first, int len);

    friend ostream& operator<<(ostream& outs, const Token& tok);
    friend ostream& operator<<(ostream& outs, const Token* tok);
};

#endif // TOKEN_H