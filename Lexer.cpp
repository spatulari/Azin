#include "lexer.hpp"
#include <cctype>

Lexer::Lexer(const std::string& src)
    : source(src), pos(0) {}

char Lexer::peek() {
    if (pos >= source.size()) return '\0';
    return source[pos];
}

char Lexer::peekNext()
{
    if (pos + 1 >= source.size()) return '\0';
    return source[pos + 1];
}

char Lexer::advance() 
{
    if (pos >= source.size()) return '\0';
    return source[pos++];
}

void Lexer::skipWhitespace() 
{
    while (std::isspace(peek())) advance();
}

std::vector<Token> Lexer::tokenize() 
{
    std::vector<Token> tokens;

    while (true) 
    {
        skipWhitespace();

        if (peek() == '/' && peekNext() == '/')
        {
            // consume '//'
            advance();
            advance();

            // skip until end of line or EOF
            while (peek() != '\n' && peek() != '\0')
                advance();

            continue; // restart token loop
        }

        char c = peek();
        if (c == '\0') 
        {
            tokens.push_back({TOK_EOF, ""});
            break;
        }
        //TODO: make this work with floats and hex etc etc
        else if (isdigit(peek()))
        {
            std::vector<char> num_tmp;

            while (isdigit(peek()))
            {
                num_tmp.push_back(advance());
            }

            std::string num_final(num_tmp.begin(), num_tmp.end());
            tokens.push_back({TOK_INT_LIT, num_final});
        }
        // TODO: Unhardcode checking keywordsd etc etc
        else if (isalpha(peek()))
        {
            std::vector<char> c_tmp;

            while (isalpha(peek()))
            {
                c_tmp.push_back(advance());
            }

            std::string c_final(c_tmp.begin(), c_tmp.end());

            if (c_final == "int")
            {
                tokens.push_back({TOK_INT, c_final});
            }
            else if (c_final == "return")
            {
                tokens.push_back({TOK_RETURN, c_final});
            }
            else if (c_final == "bool")
            {
                tokens.push_back({TOK_BOOL, c_final});
            }
            else if (c_final == "true")
            {
                tokens.push_back({TOK_BOOL_LIT, "1"});
            }
            else if (c_final == "false")
            {
                tokens.push_back({TOK_BOOL_LIT, "0"});
            }
            else
            {
                tokens.push_back({TOK_IDENT, c_final});
            }
        }  
        else if (c == ';') {tokens.push_back({TOK_SEMI, std::string(1, c)}); advance();}
        else if (c == '(') {tokens.push_back({TOK_LPAREN, std::string(1, c)}); advance();}
        else if (c == ')') {tokens.push_back({TOK_RPAREN, std::string(1, c)}); advance();}
        else if (c == '!') {tokens.push_back({TOK_BANG, std::string(1, c)}); advance();}
        else if (c == '@') {tokens.push_back({TOK_AT, std::string(1, c)}); advance();}
        else if (c == '=') {tokens.push_back({TOK_EQUAL, std::string(1, c)}); advance();}

        else if (c == '"')
        {
            std::vector<char> slt;  // stands for str_lit_tmp 
            advance();
            while(peek() != '"')
            {
                slt.push_back(advance());
            }
            advance();
            std::string slt_final(slt.begin(), slt.end());
            tokens.push_back({TOK_STRING_LIT, slt_final});
        }
         

    }

    return tokens;
}
