class Parser {
    Scanner* scanner;
    Token *current, *previous;

    bool match(Token::Type t);
    bool check(Token::Type t) const;
    bool advance();
    bool isAtEnd() const;

public:
    Parser(Scanner* s);

    Program* parseProgram();
    Stm* parseStm();

    // CExp
    CExp* parseCExp();

    // Expr
    Exp* parseExpr();
    Exp* parseTerm();
    Exp* parseFactor();

    // SetExpr
    SetExp* parseSetExpr();
    SetExp* parseSetTerm();
    SetExp* parseSetFactor();
    SetExp* parseSet();

    // util
    void consume(Token::Type t, const char* msg);
};
