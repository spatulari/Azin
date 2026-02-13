#include "semantic.hpp"
#include <stdexcept>

namespace azin {

// ===== SymbolTable =====

void SymbolTable::enterScope() {
    scopes.emplace_back();
}

void SymbolTable::exitScope() {
    scopes.pop_back();
}

bool SymbolTable::declare(const std::string& name, const Symbol& symbol) {
    if (scopes.empty())
        enterScope();

    auto& current = scopes.back();

    if (current.find(name) != current.end())
        return false;

    current[name] = symbol;
    return true;
}

Symbol* SymbolTable::lookup(const std::string& name) {
    for (int i = scopes.size() - 1; i >= 0; --i) {
        auto it = scopes[i].find(name);
        if (it != scopes[i].end())
            return &it->second;
    }
    return nullptr;
}

void SemanticAnalyzer::analyze(Program& program)
{
    symbols.enterScope();

    // First declare all functions globally
    for (auto& fn : program.functions)
    {
        Symbol sym;
        sym.kind = SymbolKind::Function;
        sym.type = fn.returnType;

        for (auto& p : fn.params)
            sym.paramTypes.push_back(p.type);

        if (!symbols.declare(fn.name, sym))
            throw std::runtime_error("Function redeclared: " + fn.name);
    }

    for (auto& fn : program.functions)
        analyzeFunction(fn);

    symbols.exitScope();
}

void SemanticAnalyzer::analyzeFunction(FunctionDecl& fn)
{
    currentFunctionReturnType = fn.returnType;
    foundReturnInCurrentFunction = false;

    symbols.enterScope();

    for (auto& param : fn.params)
    {
        Symbol sym;
        sym.kind = SymbolKind::Variable;
        sym.type = param.type;

        if (!symbols.declare(param.name, sym))
            throw std::runtime_error("Parameter redeclared: " + param.name);
    }

    analyzeBlock(fn.body.get());

    if (currentFunctionReturnType != "nore" && !foundReturnInCurrentFunction)
        throw std::runtime_error("Missing return in function: " + fn.name);

    symbols.exitScope();
}


void SemanticAnalyzer::analyzeBlock(BlockStmt* block)
{
    symbols.enterScope();

    for (auto& stmt : block->statements)
        analyzeStatement(stmt.get());

    symbols.exitScope();
}

void SemanticAnalyzer::analyzeStatement(Stmt* stmt)
{
    if (auto var = dynamic_cast<VarDeclStmt*>(stmt))
    {
        std::string initType = analyzeExpression(var->initializer.get());

        if (initType != var->type)
            throw std::runtime_error("Type mismatch in variable declaration: " + var->name);

        Symbol sym;
        sym.kind = SymbolKind::Variable;
        sym.type = var->type;

        if (!symbols.declare(var->name, sym))
            throw std::runtime_error("Variable redeclared: " + var->name);
    }
    else if (auto assign = dynamic_cast<AssignmentStmt*>(stmt))
    {
        Symbol* sym = symbols.lookup(assign->name);
        if (!sym)
            throw std::runtime_error("Undefined variable: " + assign->name);

        std::string valueType = analyzeExpression(assign->value.get());

        if (valueType != sym->type)
            throw std::runtime_error("Type mismatch in assignment to: " + assign->name);
    }
    else if (auto ret = dynamic_cast<ReturnStmt*>(stmt))
    {
        if (currentFunctionReturnType == "nore")
        {
            if (ret->value)
                throw std::runtime_error("nore function cannot return a value");

            foundReturnInCurrentFunction = true;
            return;
        }

        // non-nore function
        if (!ret->value)
            throw std::runtime_error("Non-nore function must return a value");

        std::string valueType = analyzeExpression(ret->value.get());

        if (valueType != currentFunctionReturnType)
            throw std::runtime_error("Return type mismatch");

        foundReturnInCurrentFunction = true;
    }


    else if (auto ifstmt = dynamic_cast<IfStmt*>(stmt))
    {
        std::string condType = analyzeExpression(ifstmt->condition.get());

        if (condType != "bool")
            throw std::runtime_error("Condition must be bool");

        analyzeBlock(ifstmt->thenBranch.get());

        if (ifstmt->elseBranch)
            analyzeBlock(ifstmt->elseBranch.get());
    }
    else if (auto exprStmt = dynamic_cast<ExpressionStmt*>(stmt))
    {
        analyzeExpression(exprStmt->expression.get());
    }
}

std::string SemanticAnalyzer::analyzeExpression(Expr* expr)
{
    if (auto lit = dynamic_cast<LiteralExpr*>(expr))
    {
        if (lit->value == "true" || lit->value == "false")
            return "bool";

        return "int"; // for now all numbers are int
    }

    if (auto var = dynamic_cast<VarExpr*>(expr))
    {
        Symbol* sym = symbols.lookup(var->name);
        if (!sym)
            throw std::runtime_error("Undefined identifier: " + var->name);

        if (sym->kind == SymbolKind::Function)
            throw std::runtime_error("Function used as variable: " + var->name);

        return sym->type;
    }

    if (auto call = dynamic_cast<CallExpr*>(expr))
    {
        Symbol* sym = symbols.lookup(call->callee);

        if (!sym)
            throw std::runtime_error("Undefined function: " + call->callee);

        if (sym->kind != SymbolKind::Function)
            throw std::runtime_error("Variable used as function: " + call->callee);

        if (call->arguments.size() != sym->paramTypes.size())
            throw std::runtime_error("Incorrect argument count in call to: " + call->callee);

        for (size_t i = 0; i < call->arguments.size(); ++i)
        {
            std::string argType = analyzeExpression(call->arguments[i].get());

            if (argType != sym->paramTypes[i])
                throw std::runtime_error("Argument type mismatch in call to: " + call->callee);
        }

        return sym->type;
    }

    if (auto bin = dynamic_cast<BinaryExpr*>(expr))
    {
        std::string leftType = analyzeExpression(bin->left.get());
        std::string rightType = analyzeExpression(bin->right.get());

        if (leftType != rightType)
            throw std::runtime_error("Type mismatch in binary expression");

        // Comparison operators
        if (bin->op == "==" || bin->op == "!=" ||
            bin->op == "<"  || bin->op == ">"  ||
            bin->op == "<=" || bin->op == ">=")
        {
            return "bool";
        }

        // Arithmetic operators
        if (bin->op == "+" || bin->op == "-" ||
            bin->op == "*" || bin->op == "/")
        {
            if (leftType != "int")
                throw std::runtime_error("Arithmetic only supported on int");

            return "int";
        }

        throw std::runtime_error("Unknown binary operator: " + bin->op);
    }




    throw std::runtime_error("Unknown expression in semantic analysis");
}



}
