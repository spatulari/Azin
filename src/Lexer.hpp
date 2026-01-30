#pragma once
#include <string>
#include <vector>
#include <cstddef>


enum TokenType 
{
    // Keywords
    TOK_INT,
    TOK_RETURN,
    TOK_FN,
    TOK_NORE,
    TOK_BOOL,
    TOK_CHAR,

    // Identifiers & literals
    TOK_IDENT,
    TOK_INT_LIT,
    TOK_STRING_LIT,
    TOK_BOOL_LIT,
    TOK_CHAR_LIT,

    // Symbols
    TOK_LPAREN,    // (
    TOK_RPAREN,    // )
    TOK_LBRACE,    // {
    TOK_RBRACE,    // }
    TOK_SEMI,      // ;
    TOK_COMMA,     // ,
    TOK_BANG,      // !
    TOK_LT,        // < 
    TOK_GT,        // >
    TOK_AT,        // @
    TOK_EQUAL,     // =
    TOK_STAR,      // *
    TOK_PLUS,      // +
    TOK_MINUS,     // -
    TOK_STAR_OP,   // *
    TOK_SLASH,     // / 

    // Other
    TOK_EOF
};


struct Token 
{
    TokenType type;
    std::string value;
};

class Lexer 
{
public:
    Lexer(const std::string& src);
    std::vector<Token> tokenize();

private:
    char peek();
    char advance();
    void skipWhitespace();
    char peekNext();

    std::string source;
    size_t pos;
};
