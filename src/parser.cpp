#include "parser.hpp"
#include <stdexcept>

namespace azin
{

// Constructor

Parser::Parser(const std::vector<Token>& tokens)
    : tokens(tokens) {}


// Entry Point

Program Parser::parse()
{
    Program program;

    while (!isAtEnd())
    {
        program.functions.push_back(parseTopLevel());
    }

    validateMain(program);

    return program;
}

// Top-Level Parsing

FunctionDecl Parser::parseTopLevel()
{
    return parseFunction();
}

Token Parser::parseType()
{
    if (match(TokenType::TYPE_INT) ||
        match(TokenType::TYPE_NORE) ||
        match(TokenType::TYPE_I8) ||
        match(TokenType::TYPE_I16) ||
        match(TokenType::TYPE_I32) ||
        match(TokenType::TYPE_I64) ||
        match(TokenType::TYPE_U8) ||
        match(TokenType::TYPE_U16) ||
        match(TokenType::TYPE_U32) ||
        match(TokenType::TYPE_U64) ||
        match(TokenType::TYPE_BOOL))
    {
        return previous();
    }


    throw error("Expected type");
}


// Function Parsing

FunctionDecl Parser::parseFunction()
{

    Token returnType = parseType();
    currentFunctionReturnType = returnType.lexeme;
    Token name = consume(TokenType::IDENTIFIER, "Expected function name");

    consume(TokenType::LPAREN, "Expected '('");

    std::vector<Param> params;

    if (!check(TokenType::RPAREN))
    {
        do {
            Token type = parseType();
            Token name = consume(TokenType::IDENTIFIER, "Expected parameter name");

            params.push_back({ type.lexeme, name.lexeme });

        } while (match(TokenType::COMMA));
    }

    consume(TokenType::RPAREN, "Expected ')'");


    auto body = parseBlock();
    return FunctionDecl{
        returnType.lexeme,
        name.lexeme,
        std::move(params),
        std::move(body)
    };

}

// Block Parsing

std::unique_ptr<BlockStmt> Parser::parseBlock()
{
    consume(TokenType::LBRACE, "Expected '{' to start block");

    auto block = std::make_unique<BlockStmt>();

    while (!check(TokenType::RBRACE) && !isAtEnd())
    {
        block->statements.push_back(parseStatement());
    }

    consume(TokenType::RBRACE, "Expected '}' after block");

    return block;
}


// Statement Parsing

std::unique_ptr<Stmt> Parser::parseStatement()
{
    if (match(TokenType::IF))
        return parseIf();

    if (match(TokenType::RETURN))
        return parseReturn();

    if (check(TokenType::TYPE_INT) ||
        check(TokenType::TYPE_NORE) ||
        check(TokenType::TYPE_I8) ||
        check(TokenType::TYPE_I16) ||
        check(TokenType::TYPE_I32) ||
        check(TokenType::TYPE_I64) ||
        check(TokenType::TYPE_U8) ||
        check(TokenType::TYPE_U16) ||
        check(TokenType::TYPE_U32) ||
        check(TokenType::TYPE_U64) ||
        check(TokenType::TYPE_BOOL))
    {
        return parseVarDecl();
    }

    if (check(TokenType::IDENTIFIER))
    {
        if (peekNext().type == TokenType::EQUAL)
            return parseAssignment();

        if (peekNext().type == TokenType::LPAREN)
            return parseExpressionStatement();
    }


    throw error("Unknown statement");
}

std::unique_ptr<Stmt> Parser::parseExpressionStatement()
{
    auto expr = parseExpression();
    consume(TokenType::SEMICOLON, "Expected ';' after expression");

    return std::make_unique<ExpressionStmt>(std::move(expr));
}


std::unique_ptr<Stmt> Parser::parseVarDecl()
{
    Token typeToken = parseType();
    Token name = consume(TokenType::IDENTIFIER, "Expected variable name");

    consume(TokenType::EQUAL, "Expected '=' in variable declaration");

    auto initializer = parseExpression();

    consume(TokenType::SEMICOLON, "Expected ';' after variable declaration");

    return std::make_unique<VarDeclStmt>(
        typeToken.lexeme,
        name.lexeme,
        std::move(initializer)
    );
}


std::unique_ptr<Stmt> Parser::parseAssignment()
{
    Token name = consume(TokenType::IDENTIFIER, "Expected variable name");

    consume(TokenType::EQUAL, "Expected '=' in assignment");

    auto value = parseExpression();

    consume(TokenType::SEMICOLON, "Expected ';' after assignment");

    return std::make_unique<AssignmentStmt>(
        name.lexeme,
        std::move(value)
    );
}


std::unique_ptr<ReturnStmt> Parser::parseReturn()
{
    if (check(TokenType::SEMICOLON))
    {
        consume(TokenType::SEMICOLON, "Expected ';'");
        return std::make_unique<ReturnStmt>(nullptr);
    }

    auto value = parseExpression();
    consume(TokenType::SEMICOLON, "Expected ';' after return value");

    if (currentFunctionReturnType == "nore")
    {
        if (value != nullptr)
            throw error("nore function cannot return a value");
    }
    else
    {
        if (value == nullptr)
            throw error("Non-nore function must return a value");
    }


    return std::make_unique<ReturnStmt>(std::move(value));
}


std::unique_ptr<Stmt> Parser::parseIf()
{
    consume(TokenType::LPAREN, "Expected '(' after if");
    auto condition = parseExpression();
    consume(TokenType::RPAREN, "Expected ')' after condition");

    auto thenBranch = parseBlock();

    std::unique_ptr<BlockStmt> elseBranch = nullptr;

    if (match(TokenType::ELSE))
    {
        elseBranch = parseBlock();
    }

    return std::make_unique<IfStmt>(
        std::move(condition),
        std::move(thenBranch),
        std::move(elseBranch)
    );
}



// Expression Parsing

std::unique_ptr<Expr> Parser::parseExpression()
{
    return parseEquality();
}

std::unique_ptr<Expr> Parser::parseEquality()
{
    auto left = parseComparison();

    while (match(TokenType::EQEQ) || match(TokenType::NOTEQ))
    {
        std::string op = previous().lexeme;
        auto right = parseComparison();

        left = std::make_unique<BinaryExpr>(
            std::move(left),
            op,
            std::move(right)
        );
    }

    return left;
}

std::unique_ptr<Expr> Parser::parseComparison()
{
    auto left = parseTerm();

    while (match(TokenType::LT)  || match(TokenType::GT) ||
           match(TokenType::LTEQ)|| match(TokenType::GTEQ))
    {
        std::string op = previous().lexeme;
        auto right = parseTerm();

        left = std::make_unique<BinaryExpr>(    
            std::move(left),
            op,
            std::move(right)
        );
    }

    return left;
}



std::unique_ptr<Expr> Parser::parseTerm()
{
    auto left = parseFactor();

    while (match(TokenType::PLUS) || match(TokenType::MINUS))
    {
        std::string op = previous().lexeme;
        auto right = parseFactor();

        left = std::make_unique<BinaryExpr>(
            std::move(left),
            op,
            std::move(right)
        );
    }

    return left;
}

std::unique_ptr<Expr> Parser::parseFactor()
{
    auto left = parsePrimary();

    while (match(TokenType::STAR) || match(TokenType::SLASH))
    {
        std::string op = previous().lexeme;
        auto right = parsePrimary();

        left = std::make_unique<BinaryExpr>(
            std::move(left),
            op,
            std::move(right)
        );
    }

    return left;
}

std::unique_ptr<Expr> Parser::parsePrimary()
{
    if (match(TokenType::NUMBER))
        return std::make_unique<LiteralExpr>(previous().lexeme);


    if (match(TokenType::TRUE))
    return std::make_unique<LiteralExpr>("true");

    if (match(TokenType::FALSE))
        return std::make_unique<LiteralExpr>("false");


    if (match(TokenType::IDENTIFIER))
    {
        std::string name = previous().lexeme;

        if (match(TokenType::LPAREN))
        {
            std::vector<std::unique_ptr<Expr>> args;

            if (!check(TokenType::RPAREN))
            {
                do {
                    args.push_back(parseExpression());
                } while (match(TokenType::COMMA));
            }

            consume(TokenType::RPAREN, "Expected ')'");
            return std::make_unique<CallExpr>(name, std::move(args));
        }

        return std::make_unique<VarExpr>(name);
    }



    if (match(TokenType::LPAREN))
    {
        auto expr = parseExpression();
        consume(TokenType::RPAREN, "Expected ')'");
        return expr;
    }


    throw error("Invalid expression");
}

// Validation

void Parser::validateMain(const Program& program)
{
    bool foundMain = false;

    for (const auto& fn : program.functions)
    {
        if (fn.name == "main")
        {
            if (fn.returnType != "int")
                throw std::runtime_error("main must return int");

            foundMain = true;
        }
    }

    if (!foundMain)
        throw std::runtime_error("No main function found");
}


// Utility Helpers
const Token& Parser::peek() const
{
    return tokens[current];
}

const Token& Parser::peekNext() const
{
    if (current + 1 >= tokens.size())
        return tokens.back();

    return tokens[current + 1];
}


const Token& Parser::previous() const
{
    return tokens[current - 1];
}

bool Parser::isAtEnd() const
{
    return peek().type == TokenType::END_OF_FILE;
}

const Token& Parser::advance()
{
    if (!isAtEnd()) current++;
    return previous();
}

bool Parser::check(TokenType type) const
{
    if (isAtEnd()) return false;
    return peek().type == type;
}

bool Parser::match(TokenType type)
{
    if (check(type))
    {
        advance();
        return true;
    }
    return false;
}

Token Parser::consume(TokenType type, const std::string& message)
{
    if (check(type))
        return advance();

    throw error(message);
}

std::runtime_error Parser::error(const std::string& message) const
{
    return std::runtime_error(
        "Parser error at line " +
        std::to_string(peek().line) +
        ": " + message
    );
}

}
