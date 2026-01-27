#pragma once
#include <vector>
#include "Lexer.hpp"
#include "ast.hpp"

struct Parser
{
    const std::vector<Token>* tokens;
    size_t pos;
};

Parser make_parser(const std::vector<Token>& tokens);

Stmt* parse_statement(Parser& p);
Expr* parse_expression(Parser& p);
std::vector<Stmt*> parse_program(Parser& p);