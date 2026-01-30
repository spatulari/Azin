#include "parser.hpp"
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>


static std::unordered_map<std::string, VarType> symbol_table;
static std::unordered_set<std::string> imported_modules;


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

static Expr* parse_primary(Parser& p);

static std::vector<Stmt*> parse_block(Parser& p)
{
    std::vector<Stmt*> stmts;

    if (peek(p).type != TOK_LBRACE)
        throw std::runtime_error("expected '{'");

    advance(p); // '{'

    while (peek(p).type != TOK_RBRACE)
    {
        stmts.push_back(parse_statement(p));
    }

    advance(p); // '}'
    return stmts;
}

static bool is_function(Parser& p)
{
    if (peek(p).type != TOK_INT && peek(p).type != TOK_NORE)
        return false;

    if ((*p.tokens)[p.pos + 1].type != TOK_IDENT)
        return false;

    if ((*p.tokens)[p.pos + 2].type != TOK_LPAREN)
        return false;

    return true;
}

static Function* parse_function(Parser& p)
{
    symbol_table.clear();

    VarType ret_type;

    if (peek(p).type == TOK_INT)
    {
        ret_type = TYPE_INT;
        advance(p);
    }
    else if (peek(p).type == TOK_NORE)
    {
        ret_type = TYPE_NORE;
        advance(p);
    }
    else
    {
        throw std::runtime_error("expected function return type");
    }

    if (peek(p).type != TOK_IDENT)
        throw std::runtime_error("expected function name");

    std::string name = advance(p).value;

    if (peek(p).type != TOK_LPAREN)
        throw std::runtime_error("expected '('");
    advance(p);

    if (peek(p).type != TOK_RPAREN)
        throw std::runtime_error("functions take no parameters yet");
    advance(p);

    std::vector<Stmt*> body = parse_block(p);

    if (ret_type == TYPE_INT)
    {
        bool has_return = false;

        for (Stmt* s : body)
        {
            if (s->type == STMT_RETURN && s->value != nullptr)
                has_return = true;
        }

        if (!has_return)
            throw std::runtime_error("int function must return a value");
    }

    Function* f = new Function;
    f->name = name;
    f->return_type = ret_type;
    f->body = body;

    return f;
}



Stmt* parse_statement(Parser& p)
{
    Token t = peek(p);

    if (t.type == TOK_INT && 
    p.pos + 2 < p.tokens->size() &&
    (*p.tokens)[p.pos + 2].type == TOK_LPAREN)
    {
        throw std::runtime_error("function declarations not allowed here");
    }

    
    // return
    if (t.type == TOK_RETURN)
    {
        advance(p); // consume 'return'

        Expr* value = nullptr;

        if (peek(p).type != TOK_SEMI)
        {
            value = parse_expression(p);
        }

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
    if (t.type == TOK_CHAR)
    {
        advance(p); // consume 'char'

        bool is_ptr = false;

        if (peek(p).type == TOK_STAR)
        {
            is_ptr = true;
            advance(p); // consume '*'
        }

        if (peek(p).type != TOK_IDENT)
            throw std::runtime_error("expected variable name");

        std::string name = advance(p).value;

        if (peek(p).type != TOK_EQUAL)
            throw std::runtime_error("expected '='");

        advance(p); // '='

        Expr* value = parse_expression(p);

        if (peek(p).type != TOK_SEMI)
            throw std::runtime_error("expected ';'");

        advance(p); // ';'

        if (symbol_table.count(name))
            throw std::runtime_error("variable already declared");

        Stmt* s = new Stmt;
        s->type = STMT_DECL;
        s->var_name = name;
        s->value = value;

        if (is_ptr)
        {
            if (value->type != EXPR_STRING)
                throw std::runtime_error("char* can only be assigned string literals");

            s->var_type = TYPE_CHAR_PTR;
            symbol_table[name] = TYPE_CHAR_PTR;
        }
        else
        {
            if (value->type != EXPR_CHAR)
                throw std::runtime_error("char can only be assigned char literals");

            s->var_type = TYPE_CHAR;
            symbol_table[name] = TYPE_CHAR;
        }

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
        s->type = STMT_ASSIGN;   
        s->value = value;
        s->var_name = name;      
        return s;
    }
    if (t.type == TOK_BANG)
    {
        advance(p); // '!'

        if (peek(p).type != TOK_IDENT || peek(p).value != "use")
            throw std::runtime_error("expected 'use' after '!'"
            );

        advance(p); // 'use'

        if (peek(p).type != TOK_LT)
            throw std::runtime_error("expected '<' after !use");
        advance(p);

        if (peek(p).type != TOK_IDENT)
            throw std::runtime_error("expected module name");

        std::string module = advance(p).value;

        if (peek(p).type != TOK_GT)
            throw std::runtime_error("expected '>'");
        advance(p);

        if (imported_modules.find(module) != imported_modules.end())
            throw std::runtime_error(
                "module '" + module + "' already imported"
            );

        imported_modules.insert(module);

        Stmt* s = new Stmt;
        s->type = STMT_USE;
        s->module_name = module;
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
    Expr* left = parse_primary(p);

    while (true)
    {
        Token t = peek(p);

        BinOp op;
        if (t.type == TOK_PLUS)       op = OP_ADD;
        else if (t.type == TOK_MINUS) op = OP_SUB;
        else if (t.type == TOK_STAR)  op = OP_MUL;
        else if (t.type == TOK_SLASH) op = OP_DIV;
        else break;

        advance(p); // operator

        Expr* right = parse_primary(p);

        Expr* e = new Expr;
        e->type = EXPR_BINOP;
        e->op = op;
        e->left = left;
        e->right = right;

        left = e;
    }

    return left;
}


static Expr* parse_primary(Parser& p)
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

    if (t.type == TOK_CHAR_LIT)
    {
        advance(p);

        Expr* e = new Expr;
        e->type = EXPR_CHAR;
        e->char_value = t.value[0];
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

        if (peek(p).type == TOK_AT)
        {
            advance(p); // '@'

            if (peek(p).type != TOK_IDENT)
                throw std::runtime_error("expected module name");

            std::string module = advance(p).value;

            if (imported_modules.find(module) == imported_modules.end())
                throw std::runtime_error(
                    "module '" + module + "' not imported"
                );

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

        Expr* e = new Expr;
        e->type = EXPR_VAR;
        e->var_name = name;
        return e;
    }

    throw std::runtime_error("expected expression");
}



Function* parse_program(Parser& p)
{
    Function* main_fn = nullptr;
    std::vector<Stmt*> globals;

    while (peek(p).type != TOK_EOF)
    {
        if (is_function(p))
        {
            Function* fn = parse_function(p);

            if (fn->name != "main")
                throw std::runtime_error("only 'main' function is allowed");

            main_fn = fn;
        }
        else
        {
            // top-level statement
            if (main_fn)
                throw std::runtime_error("statements must appear before main");

            globals.push_back(parse_statement(p));
        }
    }

    if (!main_fn)
        throw std::runtime_error("missing main function");

    main_fn->globals = globals;
    return main_fn;
}
