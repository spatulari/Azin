#pragma once
#include <vector>
#include "Lexer.hpp"
#include "ast.hpp"

struct Function;
struct Parser
{
    const std::vector<Token>* tokens;
    size_t pos;
};

Parser make_parser(const std::vector<Token>& tokens);

Stmt* parse_statement(Parser& p);
Expr* parse_expression(Parser& p);
Function* parse_program(Parser& p);