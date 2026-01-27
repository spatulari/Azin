#include "parser.hpp"
#include <stdexcept>

Parser make_parser(const std::vector<Token>& tokens)
{
    Parser p;
    p.tokens = &tokens;
    p.pos = 0;
    return p;
}

static Token peek(Parser& p)
{
    return (*p.tokens)[p.pos];
}

static Token advance(Parser& p)
{
    return (*p.tokens)[p.pos++];
}

Stmt* parse_statement(Parser& p)
{
    Token t = peek(p);

    // ----- return statement -----
    if (t.type == TOK_RETURN)
    {
        advance(p); // consume 'return'
        Expr* value = parse_expression(p);

        if (peek(p).type != TOK_SEMI)
            throw std::runtime_error("expected ';' after return");

        advance(p); // consume ';'

        Stmt* s = new Stmt;
        s->type = STMT_RETURN;
        s->value = value;
        return s;
    }

    // ----- expression statement -----
    Expr* e = parse_expression(p);

    if (peek(p).type != TOK_SEMI)
        throw std::runtime_error("expected ';' after expression");

    advance(p); // consume ';'

    Stmt* s = new Stmt;
    s->type = STMT_EXPR;
    s->value = e;
    return s;
}


Expr* parse_expression(Parser& p)
{
    Token t = peek(p);

    if (t.type == TOK_INT_LIT)
    {
        advance(p);

        Expr* e = new Expr;
        e->type = EXPR_INT;
        e->int_value = std::stoi(t.value);
        return e;
    }

    if (t.type == TOK_STRING_LIT)
    {
        advance(p);

        Expr* e = new Expr;
        e->type = EXPR_STRING;
        e->string_value = t.value;
        return e;
    }

    if (t.type == TOK_IDENT)
    {
        advance(p);
        std::string func = t.value;

        if (peek(p).type != TOK_AT)
            throw std::runtime_error("expected '@' after function name");

        advance(p); 

        if (peek(p).type != TOK_IDENT)
            throw std::runtime_error("expected module name after '@'");

        std::string module = advance(p).value;

        if (peek(p).type != TOK_LPAREN)
            throw std::runtime_error("expected '('");

        advance(p); 

        Expr* arg = parse_expression(p);

        if (peek(p).type != TOK_RPAREN)
            throw std::runtime_error("expected ')'");

        advance(p);

        // build AST node
        Expr* e = new Expr;
        e->type = EXPR_CALL;
        e->func_name = func;
        e->module_name = module;
        e->arg = arg;
        return e;
    }

    throw std::runtime_error("expected expression");
}

std::vector<Stmt*> parse_program(Parser& p)
{
    std::vector<Stmt*> stmts;

    while (true)
    {
        if (p.pos >= p.tokens->size())
            break;

        if ((*p.tokens)[p.pos].type == TOK_EOF)
            break;

        stmts.push_back(parse_statement(p));
    }

    return stmts;
}

