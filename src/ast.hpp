#pragma once

#include <memory>
#include <string>
#include <vector>
#include <ostream>
#include <variant>

namespace azin
{
    struct Type
    {
        std::string base;   // "int", "char", etc
        bool isPointer = false;
        bool isArray = false;

        bool operator==(const Type& other) const
        {
            return base == other.base &&
                isPointer == other.isPointer &&
                isArray == other.isArray;
        }

        bool operator!=(const Type& other) const
        {
            return !(*this == other);
        }
    };

    inline std::ostream& operator<<(std::ostream& os, const Type& t)
    {
        os << t.base;

        if (t.isPointer)
            os << "*";

        if (t.isArray)
            os << "[]";

        return os;
    }


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
        Type type;
        std::string name;

        Param(const Type& type, const std::string& name)
            : type(type), name(name) {}
    };

    // FUNCTION 

    struct FunctionDecl
    {
        Type returnType;
        std::string name;
        std::vector<Param> params;
        std::unique_ptr<BlockStmt> body;
        bool isExtern = false;
    };


    // Binary Expressions 
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
        Type type;
        std::string name;
        std::unique_ptr<Expr> initializer;

        bool isArray = false;
        int arraySize = -1;

        VarDeclStmt(const Type& type,
                    const std::string& name,
                    std::unique_ptr<Expr> initializer,
                    bool isArray = false,
                    int arraySize = -1)
            : type(type),
            name(name),
            initializer(std::move(initializer)),
            isArray(isArray),
            arraySize(arraySize) {}


    };

    struct AssignmentStmt : Stmt
    {
        std::unique_ptr<Expr> target;
        std::unique_ptr<Expr> value;

        AssignmentStmt(std::unique_ptr<Expr> target,
                    std::unique_ptr<Expr> value)
            : target(std::move(target)),
            value(std::move(value)) {}
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
        std::string callee;        // function name
        std::string moduleName;    // empty if not qualified
        std::vector<std::unique_ptr<Expr>> arguments;

        CallExpr(std::string callee,
                std::vector<std::unique_ptr<Expr>> args,
                std::string moduleName = "")
            : callee(std::move(callee)),
            moduleName(std::move(moduleName)),
            arguments(std::move(args)) {}
    };



    struct ExpressionStmt : Stmt
    {
        std::unique_ptr<Expr> expression;

        explicit ExpressionStmt(std::unique_ptr<Expr> expr)
            : expression(std::move(expr)) {}
    };

    struct StringExpr : Expr
    {
        std::string value;

        explicit StringExpr(const std::string& value)
            : value(value) {}
    };
    struct IndexExpr : Expr
    {
        std::unique_ptr<Expr> base;
        std::unique_ptr<Expr> index;

        IndexExpr(std::unique_ptr<Expr> base,
                std::unique_ptr<Expr> index)
            : base(std::move(base)),
            index(std::move(index)) {}
    };

    struct UseDecl
    {
        std::string path;
    };

    using TopLevelDecl = std::variant<FunctionDecl, UseDecl>;

    struct Program
    {
        std::vector<TopLevelDecl> decls;
    };

    struct WhileStmt : Stmt
    {   
        std::unique_ptr<Expr> condition;
        std::unique_ptr<BlockStmt> body;

        WhileStmt(std::unique_ptr<Expr> condition,
                std::unique_ptr<BlockStmt> body)
            : condition(std::move(condition)),
            body(std::move(body)) {}
    };

    struct UnaryExpr : Expr
    {
        std::string op;
        std::unique_ptr<Expr> operand;

        UnaryExpr(const std::string& op,
                std::unique_ptr<Expr> operand)
            : op(op), operand(std::move(operand)) {}
    };


}
