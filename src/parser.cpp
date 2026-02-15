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
        program.decls.push_back(std::move(parseTopLevel()));
    }

    return program;
}

// Top-Level Parsing

TopLevelDecl Parser::parseTopLevel()
{
    if (match(TokenType::USE_DIRECTIVE))
        return parseUse();

    return parseFunction();
}


Type Parser::parseType()
{
    Type t;

    if (match(TokenType::TYPE_INT))
        t.base = "int";
    else if (match(TokenType::TYPE_CHAR))
        t.base = "char";
    else if (match(TokenType::TYPE_BOOL))
        t.base = "bool";
    else if (match(TokenType::TYPE_NORE))
        t.base = "nore";
    else if (match(TokenType::TYPE_I8))
    t.base = "i8";
    else if (match(TokenType::TYPE_I16))
        t.base = "i16";
    else if (match(TokenType::TYPE_I32))
        t.base = "i32";
    else if (match(TokenType::TYPE_I64))
        t.base = "i64";
    else if (match(TokenType::TYPE_U8))
        t.base = "u8";
    else if (match(TokenType::TYPE_U16))
        t.base = "u16";
    else if (match(TokenType::TYPE_U32))
        t.base = "u32";
    else if (match(TokenType::TYPE_U64))
        t.base = "u64";

    else
        throw error("Expected type");

    // handle pointers
    while (match(TokenType::STAR))
        t.isPointer = true;

    return t;
}

UseDecl Parser::parseUse()
{
    Token path = consume(TokenType::STRING, "Expected file path after !use");
    // consume(TokenType::SEMICOLON, "Expected ';' after !use");

    return UseDecl{ path.lexeme };
}



FunctionDecl Parser::parseExtern()
{
    Type returnType = parseType();
    Token name = consume(TokenType::IDENTIFIER, "Expected function name");

    consume(TokenType::LPAREN, "Expected '('");

    std::vector<Param> params;

    if (!check(TokenType::RPAREN))
    {
        do {
            Type type = parseType();
            Token name = consume(TokenType::IDENTIFIER, "Expected parameter name");

            bool isArray = false;

            if (match(TokenType::LBRACKET))
            {
                consume(TokenType::RBRACKET, "Expected ']'");
                isArray = true;
            }

            if (isArray)
                type.isPointer = true;

            params.push_back({ type, name.lexeme });


        } while (match(TokenType::COMMA));
    }

    consume(TokenType::RPAREN, "Expected ')'");
    consume(TokenType::SEMICOLON, "Expected ';' after extern declaration");

    FunctionDecl fn{
        returnType,
        name.lexeme,
        std::move(params),
        nullptr
    };

    fn.isExtern = true;
    return fn;
}

std::unique_ptr<Expr> Parser::parseUnary()
{
    if (match(TokenType::MINUS))
    {
        std::string op = previous().lexeme;
        auto operand = parseUnary();  // recursion
        return std::make_unique<UnaryExpr>(op, std::move(operand));
    }

    return parsePrimary();
}


// Function Parsing

FunctionDecl Parser::parseFunction()
{
    if (match(TokenType::EXTERN))
        return parseExtern();

    Type returnType = parseType();
    currentFunctionReturnType = returnType.base;

    Token name = consume(TokenType::IDENTIFIER, "Expected function name");

    
    consume(TokenType::LPAREN, "Expected '('");
    std::vector<Param> params;

    if (!check(TokenType::RPAREN))
    {
        do {
            Type type = parseType();
            Token name = consume(TokenType::IDENTIFIER, "Expected parameter name");

            bool isArray = false;

            if (match(TokenType::LBRACKET))
            {
                consume(TokenType::RBRACKET, "Expected ']'");
                type.isPointer = true;
            }

            params.push_back({ type, name.lexeme });

        } while (match(TokenType::COMMA));
    }


    consume(TokenType::RPAREN, "Expected ')'");


    auto body = parseBlock();
    return FunctionDecl{
        returnType,
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

    if (match(TokenType::WHILE))
        return parseWhile();


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
        check(TokenType::TYPE_BOOL) ||
        check(TokenType::TYPE_CHAR))
    {
        return parseVarDecl();
    }

    if (check(TokenType::IDENTIFIER))
    {
        auto expr = parseExpression();

        if (match(TokenType::EQUAL))
        {
            auto value = parseExpression();
            consume(TokenType::SEMICOLON, "Expected ';'");

            return std::make_unique<AssignmentStmt>(
                std::move(expr),
                std::move(value)
            );
        }

        consume(TokenType::SEMICOLON, "Expected ';'");
        return std::make_unique<ExpressionStmt>(std::move(expr));
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
    Type typeToken = parseType();
    Token name = consume(TokenType::IDENTIFIER, "Expected variable name");

    bool isArray = false;
    int arraySize = -1;

    if (match(TokenType::LBRACKET))
    {
        Token sizeToken = consume(TokenType::NUMBER, "Expected array size");
        arraySize = std::stoi(sizeToken.lexeme);
        consume(TokenType::RBRACKET, "Expected ']'");
        isArray = true;
        typeToken.isArray = true;
    }

    std::unique_ptr<Expr> initializer = nullptr;

    if (!isArray)
    {
        consume(TokenType::EQUAL, "Expected '=' in variable declaration");
        initializer = parseExpression();
    }

    consume(TokenType::SEMICOLON, "Expected ';' after variable declaration");

    return std::make_unique<VarDeclStmt>(
        typeToken,
        name.lexeme,
        std::move(initializer),
        isArray,
        arraySize
    );

}


std::unique_ptr<Stmt> Parser::parseAssignment()
{
    Token name = consume(TokenType::IDENTIFIER, "Expected variable name");

    std::unique_ptr<Expr> target = std::make_unique<VarExpr>(name.lexeme);

    consume(TokenType::EQUAL, "Expected '=' in assignment");

    auto value = parseExpression();

    consume(TokenType::SEMICOLON, "Expected ';' after assignment");

    return std::make_unique<AssignmentStmt>(
        std::move(target),
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

std::unique_ptr<Stmt> Parser::parseWhile()
{
    consume(TokenType::LPAREN, "Expected '(' after while");

    auto condition = parseExpression();

    consume(TokenType::RPAREN, "Expected ')' after condition");

    auto body = parseBlock();

    return std::make_unique<WhileStmt>(
        std::move(condition),
        std::move(body)
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
    auto left = parseUnary();

    while (match(TokenType::STAR) || match(TokenType::SLASH) || match(TokenType::PERCENT))
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
        std::string moduleName = "";

        if (match(TokenType::AT))
        {
            Token module = consume(TokenType::IDENTIFIER,
                "Expected module name after '@'");
            moduleName = module.lexeme;
        }

        std::unique_ptr<Expr> expr = std::make_unique<VarExpr>(name);

        // Function call
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

            expr = std::make_unique<CallExpr>(
                name,
                std::move(args),
                moduleName
            );
        }

        // Array indexing (can chain)
        while (match(TokenType::LBRACKET))
        {
            auto index = parseExpression();
            consume(TokenType::RBRACKET, "Expected ']'");
            expr = std::make_unique<IndexExpr>(std::move(expr), std::move(index));
        }

        return expr;
    }

    if (match(TokenType::LPAREN))
    {
        auto expr = parseExpression();
        consume(TokenType::RPAREN, "Expected ')'");
        return expr;
    }

    if (match(TokenType::STRING))
    {
        return std::make_unique<StringExpr>(previous().lexeme);
    }

    if (match(TokenType::CHAR_LITERAL))
    {
        return std::make_unique<LiteralExpr>("'" + previous().lexeme + "'");
    }


    throw error("Invalid expression");
}

// Validation

void Parser::validateMain(const Program& program)
{
    bool foundMain = false;

    for (const auto& decl : program.decls)
    {
        if (std::holds_alternative<FunctionDecl>(decl))
        {
            const auto& fn = std::get<FunctionDecl>(decl);
            if (fn.name == "main")
            {
                if (fn.returnType.base != "int")
                    throw std::runtime_error("main must return int");

                foundMain = true;
            }
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
