// parser.cpp (cuerpo resumido y directo)

Parser::Parser(Scanner* s):scanner(s),current(nullptr),previous(nullptr){ advance(); }

bool Parser::advance(){ previous = current; current = scanner->nextToken(); return true; }
bool Parser::check(Token::Type t) const { return current && current->type==t; }
bool Parser::match(Token::Type t){ if (check(t)){ advance(); return true; } return false; }
bool Parser::isAtEnd() const { return current && current->type==Token::END; }
void Parser::consume(Token::Type t, const char* msg){ if (!match(t)) throw runtime_error(msg); }

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
CExp* Parser::parseCExp(){
    // FIRST(CExp) = FIRST(Expr) ∪ FIRST(SetExpr)
    // Desambiguación por token actual
    if (check(Token::NUM) || check(Token::LPAREN) || check(Token::ID) || check(Token::MINUS) || check(Token::SQRT)) {
        // puede ser aritmética o set (si LPAREN/ID), probamos lookahead mínimo:
        if (check(Token::LPAREN)) {
            // podría ser (Expr) o (SetExpr): probamos a parsear Expr primero,
            // pero una forma limpia es duplicar factor; aquí mantenemos simple:
            // usaremos una estrategia: si tras '(' viene '{' o palabra de set,
            // usamos SetExpr; si viene NUM/ID/(' o un op aritmético, Expr.
        }
        // Regla práctica: si próximo token claro de set es '{' o keyword cap/cup/\ -> SetExpr
    }
    if (check(Token::LBRACE)) {
        return new CExp(parseSetExpr());
    }

    // Si empieza por ID, podría ser ambos; miramos 1 token más:
    if (check(Token::ID)) {
        // Heurística: si luego aparece cap/cup/\ o ')' o ';' sin aritméticos,
        // igual funciona con SetExpr por SetFactor -> id
        // Para mantenerlo determinista, prefiero: intentar SetExpr solo si
        // el siguiente token entre {UNION,INTERSECT,DIFF,RPAREN,SEMICOL,COMMA}
        // —pero Expr también acepta id solo. Ambas son válidas.
        // Elegimos Expr por defecto salvo que veamos op de conjunto.
        // Implementación simple:
        Token* save = current; // ya es ID
        advance(); // consume ID
        bool isSet = check(Token::UNION) || check(Token::INTERSECT) || check(Token::DIFF);
        // rollback
        current = save;
        previous = nullptr;
        if (isSet) return new CExp(parseSetExpr());
        else       return new CExp(parseExpr());
    }

    // Por defecto: Expr
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
