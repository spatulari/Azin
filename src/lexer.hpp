#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace azin
{

    enum class TokenType {
        // Literals
        IDENTIFIER,
        NUMBER,

        // Control keywords
        RETURN,
        IF,
        ELSE,
        WHILE,
        FOR,

        // Types
        TYPE_INT,
        TYPE_NORE,
        TYPE_I8,
        TYPE_I16,
        TYPE_I32,
        TYPE_I64,
        TYPE_U8,
        TYPE_U16,
        TYPE_U32,
        TYPE_U64,
        TYPE_BOOL,

        // idk what to call
        TRUE,
        FALSE,



        // Delimiters
        LPAREN,
        RPAREN,
        LBRACE,
        RBRACE,
        SEMICOLON,
        COMMA,

        // Operators
        EQUAL,
        PLUS,
        MINUS,
        STAR,
        SLASH,
        AMPERSAND,
        PIPE,
        COLON,


        // Comparison
        EQEQ,
        NOTEQ,
        LT,
        GT,
        LTEQ,
        GTEQ,

        END_OF_FILE,
        UNKNOWN
    };

    struct Token {
        TokenType type;
        std::string lexeme;
        std::size_t line;
        std::size_t column;
    };

    class Lexer {
    public:
        explicit Lexer(const std::string& source);

        std::vector<Token> tokenize();

    private:
        const std::string source;

        std::size_t position = 0;
        std::size_t line = 1;
        std::size_t column = 1;

        // Core scanning
        char advance();
        char peek() const;
        char peekNext() const;
        bool match(char expected);
        bool isAtEnd() const;

        // Skipping
        void skipWhitespace();
        void skipComment();

        // Token creation
        Token makeToken(TokenType type);
        Token makeToken(TokenType type, const std::string& lexeme);

        // Specialized scanners
        Token identifier();
        Token number();

        // Keyword resolver
        TokenType resolveKeyword(const std::string& text);
    };

}
