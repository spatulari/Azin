#pragma once
#include <stdio.h>
#include <string>

enum ExprType
{
    EXPR_INT,
    EXPR_STRING,
    EXPR_CALL
};

struct Expr
{
    ExprType type;
    int int_value;
    std::string string_value;

    std::string func_name;   // "out"
    std::string module_name; // "io"
    Expr* arg;               // only 1 arg for now
};

enum StmtType
{
    STMT_RETURN,
    STMT_EXPR
};

struct Stmt
{
    StmtType type;
    Expr* value;
};

void print_stmt(Stmt* s);
void print_expr(Expr* e, int indent);
