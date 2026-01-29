#include "parser.hpp"
#include <stdexcept>
#include <unordered_map>


static std::unordered_map<std::string, VarType> symbol_table;

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

    // return
    if (t.type == TOK_RETURN)
    {
        advance(p); // consume 'return'
        Expr* value = parse_expression(p);
        if (t.type == TOK_INT && value->type == EXPR_BOOL)
            throw std::runtime_error(
                "cannot assign bool to int"
            );

        if (peek(p).type != TOK_SEMI)
            throw std::runtime_error("expected ';' after return");

        advance(p); // consume ';'

        Stmt* s = new Stmt;
        s->type = STMT_RETURN;
        s->value = value;
        return s;
    }
    if (t.type == TOK_INT)
    {
        advance(p); // consume 'int'

        if (peek(p).type != TOK_IDENT)
            throw std::runtime_error("expected variable name after 'int'");

        std::string name = advance(p).value;

        if (peek(p).type != TOK_EQUAL)
            throw std::runtime_error("expected '=' after variable name");

        advance(p); // '='

        Expr* value = parse_expression(p);

        if (peek(p).type != TOK_SEMI)
            throw std::runtime_error("expected ';' after declaration");

        advance(p);

        Stmt* s = new Stmt; 
        s->type = STMT_DECL;
        s->var_name = name;
        if (symbol_table.find(name) != symbol_table.end())
        {
            throw std::runtime_error(
                "variable '" + name + "' already declared"
            );
        }
        s->value = value;
        symbol_table[name] = TYPE_INT;
        s->var_type = TYPE_INT;
        return s;
    }
    if (t.type == TOK_BOOL)
    {
        advance(p); // consume 'bool'

        if (peek(p).type != TOK_IDENT)
            throw std::runtime_error("expected variable name after 'bool'");

        std::string name = advance(p).value;
        
        if (peek(p).type != TOK_EQUAL)
            throw std::runtime_error("expected '=' after variable name");

        advance(p); // consume '='

        Expr* value = parse_expression(p);

        if (peek(p).type != TOK_SEMI)
            throw std::runtime_error("expected ';' after declaration");

        advance(p); // consume ';'

        Stmt* s = new Stmt;
        s->type = STMT_DECL;
        s->var_name = name;
        s->value = value;
        if (symbol_table.find(name) != symbol_table.end())
        {
            throw std::runtime_error(
                "variable '" + name + "' already declared"
            );
        }
        symbol_table[name] = TYPE_BOOL;
        s->var_type = TYPE_BOOL;
        if (value->type != EXPR_BOOL)
        {
            throw std::runtime_error("cannot assign non-bool to bool");
        }

        return s;
    }
    // variable assign
    if (t.type == TOK_IDENT &&
    p.pos + 1 < p.tokens->size() &&
    (*p.tokens)[p.pos + 1].type == TOK_EQUAL)
    {
        std::string name = advance(p).value; // IDENT
        advance(p); // '='

        Expr* value = parse_expression(p);

        if (peek(p).type != TOK_SEMI)
            throw std::runtime_error("expected ';' after assignment");

        advance(p);

        // build AST node
        Stmt* s = new Stmt;
        s->type = STMT_ASSIGN;   // NEW
        s->value = value;
        s->var_name = name;      // NEW
        return s;
    }

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

    if (t.type == TOK_BOOL_LIT)
    {
        advance(p);

        Expr* e = new Expr;
        e->type = EXPR_BOOL;
        e->bool_value = (t.value == "1");
        return e;
    }

    if (t.type == TOK_IDENT)
    {
        advance(p);
        std::string name = t.value;

        // function call?
        if (peek(p).type == TOK_AT)
        {
            advance(p); // '@'

            if (peek(p).type != TOK_IDENT)
                throw std::runtime_error("expected module name");

            std::string module = advance(p).value;

            if (peek(p).type != TOK_LPAREN)
                throw std::runtime_error("expected '('");

            advance(p);

            Expr* arg = parse_expression(p);

            if (peek(p).type != TOK_RPAREN)
                throw std::runtime_error("expected ')'");

            advance(p);

            Expr* e = new Expr;
            e->type = EXPR_CALL;
            e->func_name = name;
            e->module_name = module;
            e->arg = arg;
            return e;
        }

        // variable reference
        Expr* e = new Expr;
        e->type = EXPR_VAR;
        e->var_name = name;
        return e;
    }

    throw std::runtime_error("expected expression");
}

std::vector<Stmt*> parse_program(Parser& p)
{
    std::vector<Stmt*> stmts;
    symbol_table.clear();

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

