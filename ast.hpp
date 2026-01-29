#pragma once
#include <stdio.h>
#include <string>

enum ExprType
{
    EXPR_INT,
    EXPR_STRING,
    EXPR_CALL,
    EXPR_VAR,
    EXPR_BOOL
};

struct Expr
{
    ExprType type;
    int int_value;
    std::string string_value;
    bool bool_value;

    // call
    std::string func_name;   // "out"
    std::string module_name; // "io"
    Expr* arg;  
    
    // var
    std::string var_name;

};

enum StmtType
{
    STMT_RETURN,
    STMT_EXPR,
    STMT_ASSIGN, 
    STMT_DECL
};

enum VarType 
{
    TYPE_INT,
    TYPE_BOOL
};

struct Stmt
{
    StmtType type;
    Expr* value;

    // variable
    std::string var_name;
    VarType var_type;
};

void print_stmt(Stmt* s);
void print_expr(Expr* e, int indent);
