#pragma once

#include <memory>
#include <string>
#include <vector>

namespace azin
{

    struct Expr
    {
        virtual ~Expr() = default;
    };

    struct Stmt
    {
        virtual ~Stmt() = default;
    };

    // ===== EXPRESSIONS =====

    struct LiteralExpr : Expr
    {
        std::string value;

        explicit LiteralExpr(const std::string& value)
            : value(value) {}
    };

    // ===== STATEMENTS =====

    struct ReturnStmt : Stmt
    {
        std::unique_ptr<Expr> value;

        explicit ReturnStmt(std::unique_ptr<Expr> value)
            : value(std::move(value)) {}
    };

    struct BlockStmt : Stmt
    {
        std::vector<std::unique_ptr<Stmt>> statements;
    };

    // PARAM
    struct Param
    {
        std::string type;
        std::string name;
    };

    // ===== FUNCTION =====

    struct FunctionDecl
    {
        std::string returnType;
        std::string name;
        std::vector<Param> params;
        std::unique_ptr<BlockStmt> body;
    };

    // ===== PROGRAM ROOT =====

    struct Program
    {
        std::vector<FunctionDecl> functions;
    };

    

    // ===== Binary Expressions =====
    struct BinaryExpr : Expr
    {
        std::unique_ptr<Expr> left;
        std::string op;
        std::unique_ptr<Expr> right;

        BinaryExpr(std::unique_ptr<Expr> left,
                const std::string& op,
                std::unique_ptr<Expr> right)
            : left(std::move(left)), op(op), right(std::move(right)) {}
    };

    struct VarDeclStmt : Stmt
    {
        std::string type;
        std::string name;
        std::unique_ptr<Expr> initializer;

        VarDeclStmt(const std::string& type,
                    const std::string& name,
                    std::unique_ptr<Expr> initializer)
            : type(type),
            name(name),
            initializer(std::move(initializer)) {}
    };

    struct AssignmentStmt : Stmt
    {
        std::string name;
        std::unique_ptr<Expr> value;

        AssignmentStmt(const std::string& name,
                    std::unique_ptr<Expr> value)
            : name(name), value(std::move(value)) {}
    };

    struct IfStmt : Stmt
    {
        std::unique_ptr<Expr> condition;
        std::unique_ptr<BlockStmt> thenBranch;
        std::unique_ptr<BlockStmt> elseBranch; // optional

        IfStmt(std::unique_ptr<Expr> condition,
            std::unique_ptr<BlockStmt> thenBranch,
            std::unique_ptr<BlockStmt> elseBranch)
            : condition(std::move(condition)),
            thenBranch(std::move(thenBranch)),
            elseBranch(std::move(elseBranch)) {}
    };

    struct VarExpr : Expr
    {
        std::string name;

        explicit VarExpr(const std::string& name)
            : name(name) {}
    };


    struct CallExpr : Expr
    {
        std::string callee;
        std::vector<std::unique_ptr<Expr>> arguments;

        CallExpr(std::string callee,
                std::vector<std::unique_ptr<Expr>> args)
            : callee(std::move(callee)),
            arguments(std::move(args)) {}
    };



    struct ExpressionStmt : Stmt
    {
        std::unique_ptr<Expr> expression;

        explicit ExpressionStmt(std::unique_ptr<Expr> expr)
            : expression(std::move(expr)) {}
    };



}
