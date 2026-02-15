#pragma once

#include "lexer.hpp"
#include "ast.hpp"
#include <vector>
#include <memory>
#include <stdexcept>

namespace azin
{

    class Parser
    {
    public:
        explicit Parser(const std::vector<Token>& tokens);

        Program parse();

    private:
        const std::vector<Token>& tokens;
        std::size_t current = 0;

        // ===== Top Level =====
        TopLevelDecl  parseTopLevel();
        FunctionDecl parseFunction();
        void validateMain(const Program& program);

        // ===== Blocks & Statements =====
        std::unique_ptr<BlockStmt> parseBlock();
        std::unique_ptr<Stmt> parseStatement();
        std::unique_ptr<ReturnStmt> parseReturn();
        std::unique_ptr<Stmt> parseIf();
        std::unique_ptr<Stmt> parseVarDecl();
        std::unique_ptr<Stmt> parseAssignment();
        std::unique_ptr<Stmt> parseExpressionStatement();
        Type parseType();

        // ===== Expressions =====
        std::unique_ptr<Expr> parseExpression();
        std::unique_ptr<Expr> parsePrimary();
        std::unique_ptr<Expr> parseTerm();
        std::unique_ptr<Expr> parseFactor();
        std::unique_ptr<Expr> parseEquality();
        std::unique_ptr<Expr> parseComparison();
        std::unique_ptr<Stmt> parseWhile();
        std::unique_ptr<Expr> parseUnary();
        FunctionDecl parseExtern();


        // ====== Other =====
        std::string currentFunctionReturnType;
        UseDecl parseUse();

        



        // ===== Utilities =====
        const Token& peek() const;
        const Token& previous() const;
        const Token& peekNext() const;
        const Token& advance();
        bool match(TokenType type);
        bool check(TokenType type) const;
        bool isAtEnd() const;

        Token consume(TokenType type, const std::string& message);
        std::runtime_error error(const std::string& message) const;
    };

}
