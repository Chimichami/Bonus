#ifndef VISITOR_H
#define VISITOR_H
#include "ast.h"
#include <unordered_map>

struct EvalVisitor : Visitor {
    std::unordered_map<std::string, Value> mem;

    Value visit(NumberExp*) override;
    Value visit(IdExp*) override;
    Value visit(BinaryExp*) override;
    Value visit(SqrtExp*) override;

    Value visit(SetIdExp*) override;
    Value visit(SetParenExp*) override;
    Value visit(SetBinaryExp*) override;
    Value visit(SetLiteralExp*) override;

    void visit(AssignStm*) override;
    void visit(PrintStm*) override;
};

#endif
