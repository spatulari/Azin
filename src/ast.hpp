#pragma once
#include <stdio.h>
#include <string>
#include <vector>

enum ExprType
{
    EXPR_INT,
    EXPR_STRING,
    EXPR_CALL,
    EXPR_VAR,
    EXPR_BOOL,
    EXPR_CHAR,
    EXPR_BINOP
};


enum BinOp
{
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV
};

struct Expr
{
    ExprType type;
    int int_value;
    std::string string_value;
    bool bool_value;
    char char_value;


    // call
    std::string func_name;   // "out"
    std::string module_name; // "io"
    Expr* arg;  
    
    // var
    std::string var_name;

    // binary op
    BinOp op;
    Expr* left;
    Expr* right;

};

enum StmtType
{
    STMT_RETURN,
    STMT_EXPR,
    STMT_ASSIGN, 
    STMT_DECL,
    STMT_USE

};

enum VarType 
{
    TYPE_INT,
    TYPE_BOOL,
    TYPE_CHAR_PTR,
    TYPE_CHAR,
    TYPE_NORE
};

struct Stmt
{
    StmtType type;
    Expr* value;

    // variable
    std::string var_name;
    VarType var_type;

    std::string module_name;
};

struct Function
{
    std::string name;
    VarType return_type;   // TYPE_INT or TYPE_NORE
    std::vector<Stmt*> globals;   // top-level
    std::vector<Stmt*> body; 
};


void print_stmt(Stmt* s);
void print_expr(Expr* e, int indent);
