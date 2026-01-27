#include "codegen.hpp"
#include <iostream>
#include <stdexcept>

void exec_expr(Expr* e)
{
    if (e->type == EXPR_INT)
    {
        // nothing to do for now
        return;
    }

    if (e->type == EXPR_STRING)
    {
        // nothing to do for now
        return;
    }

    if (e->type == EXPR_CALL)
    {
        // 🔥 THIS IS YOUR FIRST REAL CODEGEN RULE
        if (e->module_name == "io" && e->func_name == "out")
        {
            if (e->arg->type != EXPR_STRING)
                throw std::runtime_error("io.out expects string");

            std::cout << e->arg->string_value;
            return;
        }

        throw std::runtime_error("unknown function call");
    }
}

void exec_stmt(Stmt* s)
{
    if (s->type == STMT_EXPR)
    {
        exec_expr(s->value);
        return;
    }

    if (s->type == STMT_RETURN)
    {
        exec_expr(s->value);
        return;
    }
}
