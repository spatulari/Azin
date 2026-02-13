#pragma once

#include "ast.hpp"
#include <string>

namespace azin
{

class CodegenC
{
public:
    static std::string generate(const Program& program);

private:
    // Core generators
    static std::string generateFunction(const FunctionDecl& fn);
    static std::string generateStatement(const Stmt* stmt);
    static std::string generateExpression(const Expr* expr);

    // Indentation helpers
    static std::string indent();
    static void increaseIndent();
    static void decreaseIndent();

    // Indentation state
    static int indentLevel;


    static std::string mapTypeToC(const std::string& type);
};

}
