#include "lexer.hpp"
#include <cctype>
#include <unordered_map>

namespace azin 
{

    Lexer::Lexer(const std::string& source)
        : source(source) {}

    std::vector<Token> Lexer::tokenize() 
    {
        std::vector<Token> tokens;

        while (!isAtEnd()) {
            skipWhitespace();
            if (isAtEnd()) break;

            char c = advance();

            switch (c) 
            {
                // Single-character tokens
                case '(': tokens.push_back(makeToken(TokenType::LPAREN)); break;
                case ')': tokens.push_back(makeToken(TokenType::RPAREN)); break;
                case '{': tokens.push_back(makeToken(TokenType::LBRACE)); break;
                case '}': tokens.push_back(makeToken(TokenType::RBRACE)); break;
                case ';': tokens.push_back(makeToken(TokenType::SEMICOLON)); break;
                case ',': tokens.push_back(makeToken(TokenType::COMMA)); break;
                case '+': tokens.push_back(makeToken(TokenType::PLUS)); break;
                case '-': tokens.push_back(makeToken(TokenType::MINUS)); break;
                case '*': tokens.push_back(makeToken(TokenType::STAR)); break;
                case ':': tokens.push_back(makeToken(TokenType::COLON)); break;
                case '/':
                    if (peek() == '/') {
                        skipComment();
                    } else {
                        tokens.push_back(makeToken(TokenType::SLASH));
                    }
                    break;

                case '=':
                {
                    if (match('='))
                    {
                        tokens.push_back(makeToken(TokenType::EQEQ, "=="));
                    }
                    else
                    {
                        tokens.push_back(makeToken(TokenType::EQUAL, "="));
                    }
                    break;
                }
                case '!':
                    if (match('=')) {
                        tokens.push_back(makeToken(TokenType::NOTEQ, "!="));
                    } else {
                        tokens.push_back(makeToken(TokenType::UNKNOWN));
                    }
                    break;

                case '<':
                    if (match('=')) {
                        tokens.push_back(makeToken(TokenType::LTEQ, "<="));
                    } else {
                        tokens.push_back(makeToken(TokenType::LT, "<"));
                    }
                    break;


                case '>':
                    if (match('=')) {
                        tokens.push_back(makeToken(TokenType::GTEQ, ">="));
                    } else {
                        tokens.push_back(makeToken(TokenType::GT, ">"));
                    }
                    break;


                case '&': tokens.push_back(makeToken(TokenType::AMPERSAND)); break;
                case '|': tokens.push_back(makeToken(TokenType::PIPE)); break;

                default:
                    if (std::isalpha(c) || c == '_') {
                        position--; column--;
                        tokens.push_back(identifier());
                    }
                    else if (std::isdigit(c)) {
                        position--; column--;
                        tokens.push_back(number());
                    }
                    else {
                        tokens.push_back(makeToken(TokenType::UNKNOWN));
                    }
                    break;
            }
        }

        tokens.push_back(Token{TokenType::END_OF_FILE, "", line, column});
        return tokens;
    }

    char Lexer::peek() const {
        if (isAtEnd()) return '\0';
        return source[position];
    }

    char Lexer::peekNext() const {
        if (position + 1 >= source.length()) return '\0';
        return source[position + 1];
    }

    char Lexer::advance() {
        char c = source[position++];
        column++;

        if (c == '\n') {
            line++;
            column = 1;
        }

        return c;
    }

    bool Lexer::match(char expected) {
        if (isAtEnd()) return false;
        if (source[position] != expected) return false;

        position++;
        column++;
        return true;
    }

    bool Lexer::isAtEnd() const {
        return position >= source.length();
    }

    void Lexer::skipWhitespace() {
        while (!isAtEnd()) {
            char c = peek();
            if (c == ' ' || c == '\r' || c == '\t') {
                advance();
            }
            else if (c == '\n') {
                advance();
            }
            else {
                break;
            }
        }
    }

    void Lexer::skipComment() {
        while (!isAtEnd() && peek() != '\n') {
            advance();
        }
    }

    Token Lexer::makeToken(TokenType type) {
        return Token{type, std::string(1, source[position - 1]), line, column};
    }

    Token Lexer::makeToken(TokenType type, const std::string& lexeme) {
        return Token{type, lexeme, line, column};
    }

    Token Lexer::identifier() {
        std::size_t start = position;

        while (!isAtEnd() &&
              (std::isalnum(peek()) || peek() == '_')) {
            advance();
        }

        std::string text = source.substr(start, position - start);
        TokenType type = resolveKeyword(text);

        return Token{type, text, line, column};
    }

    Token Lexer::number() {
        std::size_t start = position;

        while (!isAtEnd() && std::isdigit(peek())) {
            advance();
        }

        std::string text = source.substr(start, position - start);
        return Token{TokenType::NUMBER, text, line, column};
    }

    TokenType Lexer::resolveKeyword(const std::string& text) {
        static std::unordered_map<std::string, TokenType> keywords = {
            {"return", TokenType::RETURN},
            {"if",     TokenType::IF},
            {"else",   TokenType::ELSE},
            {"while",  TokenType::WHILE},
            {"for",    TokenType::FOR},
            {"int",    TokenType::TYPE_INT},

            {"i8",  TokenType::TYPE_I8},
            {"i16", TokenType::TYPE_I16},
            {"i32", TokenType::TYPE_I32},
            {"i64", TokenType::TYPE_I64},

            {"u8",  TokenType::TYPE_U8},
            {"u16", TokenType::TYPE_U16},
            {"u32", TokenType::TYPE_U32},
            {"u64", TokenType::TYPE_U64},
            {"bool", TokenType::TYPE_BOOL},
            {"true", TokenType::TRUE},
            {"false", TokenType::FALSE},
            {"nore", TokenType::TYPE_NORE}


        };

        auto it = keywords.find(text);
        if (it != keywords.end()) {
            return it->second;
        }

        return TokenType::IDENTIFIER;
    }


} 
