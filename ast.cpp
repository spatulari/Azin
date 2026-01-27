#include "ast.hpp"
#include <iostream>

static void print_indent(int indent)
{
    for (int i = 0; i < indent; i++)
        std::cout << "  ";
}

void print_expr(Expr* e, int indent)
{
    print_indent(indent);

    if (e->type == EXPR_INT)
    {
        std::cout << "IntLiteral(" << e->int_value << ")\n";
        return;
    }

    if (e->type == EXPR_STRING)
    {
        std::cout << "StringLiteral(\"" << e->string_value << "\")\n";
        return;
    }

    if (e->type == EXPR_CALL)
    {
        std::cout << "CallExpr "
                  << e->func_name << "@"
                  << e->module_name << "\n";

        print_expr(e->arg, indent + 1);
        return;
    }
}

void print_stmt(Stmt* s)
{
    if (s->type == STMT_RETURN)
    {
        std::cout << "ReturnStmt\n";
        print_expr(s->value, 1);
        return;
    }

    if (s->type == STMT_EXPR)
    {
        std::cout << "ExprStmt\n";
        print_expr(s->value, 1);
        return;
    }
}
