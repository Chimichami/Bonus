#include <iostream>
#include <cstring>
#include <fstream>
#include "token.h"
#include "scanner.h"

using namespace std;

// -----------------------------
// Constructor
// -----------------------------
Scanner::Scanner(const char* s): input(s), first(0), current(0) { 
    }

// -----------------------------
// Función auxiliar
// -----------------------------

bool is_white_space(char c) {
    return c == ' ' || c == '\n' || c == '\r' || c == '\t';
}

// -----------------------------
// nextToken: obtiene el siguiente token
// -----------------------------


Token* Scanner::nextToken() {
    Token* token;

    // Saltar espacios en blanco
    while (current < input.length() && is_white_space(input[current])) 
        current++;

    // Fin de la entrada
    if (current >= input.length()) 
        return new Token(Token::END);

    char c = input[current];

    first = current;

    // Números
    if (isdigit(c)) {
        int first = current++;
        while (current < (int)input.size() && isdigit(input[current])) current++;
        return new Token(Token::NUM, input, first, current - first);
    }
    // ID
    if (isalpha(c)) {
        int first = current++;
        while (current < (int)input.size() && isalnum(input[current])) current++;
        string lex = input.substr(first, current - first);
        if (lex == "print") return new Token(Token::PRINT, input, first, lex.size());
        if (lex == "sqrt")  return new Token(Token::SQRT , input, first, lex.size()); // opcional
        if (lex == "cup")   return new Token(Token::UNION, input, first, lex.size());
        if (lex == "cap")   return new Token(Token::INTERSECT, input, first, lex.size());
        return new Token(Token::ID, input, first, lex.size());
    }
    // Operadores
    if (strchr("+/-*();=,{}\\", c)) {
        switch (c) {
            case '+': current++; return new Token(Token::PLUS, c);
            case '-': current++; return new Token(Token::MINUS, c);
            case '*': {
                // soporta ** si quieres potencia
                if (current + 1 < (int)input.size() && input[current+1] == '*') {
                    int first = current; current += 2;
                    return new Token(Token::POW, input, first, 2);
                }
                current++; return new Token(Token::MUL, c);
            }
            case '/': current++; return new Token(Token::DIV, c);
            case '(': current++; return new Token(Token::LPAREN, c);
            case ')': current++; return new Token(Token::RPAREN, c);
            case '=': current++; return new Token(Token::ASSIGN, c);
            case ';': current++; return new Token(Token::SEMICOL, c);
            case '{': current++; return new Token(Token::LBRACE, c);
            case '}': current++; return new Token(Token::RBRACE, c);
            case ',': current++; return new Token(Token::COMMA, c);
            case '\\': current++; return new Token(Token::DIFF, c);
        }
    }

    // Carácter inválido
    else {
        token = new Token(Token::ERR, c);
        current++;
    }

    return token;
}




// -----------------------------
// Destructor
// -----------------------------
Scanner::~Scanner() { }

// -----------------------------
// Función de prueba
// -----------------------------

void ejecutar_scanner(Scanner* scanner, const string& InputFile) {
    Token* tok;

    // Crear nombre para archivo de salida
    string OutputFileName = InputFile;
    size_t pos = OutputFileName.find_last_of(".");
    if (pos != string::npos) {
        OutputFileName = OutputFileName.substr(0, pos);
    }
    OutputFileName += "_tokens.txt";

    ofstream outFile(OutputFileName);
    if (!outFile.is_open()) {
        cerr << "Error: no se pudo abrir el archivo " << OutputFileName << endl;
        return;
    }

    outFile << "Scanner\n" << endl;

    while (true) {
        tok = scanner->nextToken();

        if (tok->type == Token::END) {
            outFile << *tok << endl;
            delete tok;
            outFile << "\nScanner exitoso" << endl << endl;
            outFile.close();
            return;
        }

        if (tok->type == Token::ERR) {
            outFile << *tok << endl;
            delete tok;
            outFile << "Caracter invalido" << endl << endl;
            outFile << "Scanner no exitoso" << endl << endl;
            outFile.close();
            return;
        }

        outFile << *tok << endl;
        delete tok;
    }
}
