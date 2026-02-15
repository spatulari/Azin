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

bool SemanticAnalyzer::areTypesCompatible(const Type& from, const Type& to)
{
    // Exact match
    if (from == to)
        return true;

    // Allow int → u64 / i64
    if (from.base == "int" &&
        (to.base == "u64" || to.base == "i64"))
        return true;

    if ((from.base == "char" && to.base == "int") ||
        (from.base == "int" && to.base == "char"))
        return true;

    return false;
}

void SemanticAnalyzer::analyze(Program& program)
{
    symbols.enterScope();
    bool foundMain = false;

    // ===== First pass: declare all functions globally =====
    for (auto& decl : program.decls)
    {
        if (std::holds_alternative<FunctionDecl>(decl))
        {
            auto& fn = std::get<FunctionDecl>(decl);

            Symbol sym;
            sym.kind = SymbolKind::Function;
            sym.type = fn.returnType;

            for (auto& p : fn.params)
                sym.paramTypes.push_back(p.type);

            if (!symbols.declare(fn.name, sym))
                throw std::runtime_error("Function redeclared: " + fn.name);
        }
    }

    // ===== Second pass: analyze function bodies =====
    for (auto& decl : program.decls)
    {
        if (std::holds_alternative<FunctionDecl>(decl))
        {
            auto& fn = std::get<FunctionDecl>(decl);
            analyzeFunction(fn);
        }
    }
    
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

    symbols.exitScope();
}


void SemanticAnalyzer::analyzeFunction(FunctionDecl& fn)
{
    currentFunctionReturnType = fn.returnType;
    foundReturnInCurrentFunction = false;   

    if (fn.isExtern)
    {
        return;
    }

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

    if (currentFunctionReturnType.base != "nore" && !foundReturnInCurrentFunction)
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
        if (var->isArray)
        {
            if (var->type.base != "char")
                throw std::runtime_error("Only char arrays supported for now");

            Type arrayType = var->type;
            arrayType.isArray = true;

            Symbol sym;
            sym.kind = SymbolKind::Variable;
            sym.type = arrayType;

            if (!symbols.declare(var->name, sym))
                throw std::runtime_error("Variable redeclared: " + var->name);

            return;
        }


        Type initType = analyzeExpression(var->initializer.get());

        if (initType != var->type)
            if (initType.base == "int" && var->type.base == "char")
            {
                // Allow arithmetic expressions
                if (dynamic_cast<BinaryExpr*>(var->initializer.get()))
                {
                    // OK
                }
                else if (auto lit = dynamic_cast<LiteralExpr*>(var->initializer.get()))
                {
                    int value = std::stoi(lit->value);
                    if (value < 0 || value > 127)
                        throw std::runtime_error("Literal out of char range");
                }
                else
                {
                    throw std::runtime_error("Unsafe int to char conversion");
                }
            }
            else if (initType != var->type)
            {
                throw std::runtime_error("Type mismatch in variable declaration");
            }


        Symbol sym;
        sym.kind = SymbolKind::Variable;
        sym.type = var->type;

        if (!symbols.declare(var->name, sym))
            throw std::runtime_error("Variable redeclared: " + var->name);


        if (var->isArray)
        {
            Type charType;
            charType.base = "char";

            if (var->type != charType)
                throw std::runtime_error("Cannot make an array on variable: " + var->name + " because it isn't char");
        }

    }
    else if (auto assign = dynamic_cast<AssignmentStmt*>(stmt))
    {
        Type targetType = analyzeExpression(assign->target.get());
        Type valueType  = analyzeExpression(assign->value.get());
        if (targetType == valueType)
        {
            // OK
        }
        else if (areTypesCompatible(valueType, targetType))
        {
            // OK (int -> char etc)
        }
        else
        {
            throw std::runtime_error("Type mismatch in assignment");
        }

    }
    else if (auto ret = dynamic_cast<ReturnStmt*>(stmt))
    {
        if (currentFunctionReturnType.base == "nore")
        {
            if (ret->value)
                throw std::runtime_error("nore function cannot return a value");

            foundReturnInCurrentFunction = true;
            return;
        }

        // non-nore function
        if (!ret->value)
            throw std::runtime_error("Non-nore function must return a value");

        Type valueType = analyzeExpression(ret->value.get());

        if (valueType != currentFunctionReturnType)
            throw std::runtime_error("Return type mismatch");

        foundReturnInCurrentFunction = true;
    }
    


    else if (auto ifstmt = dynamic_cast<IfStmt*>(stmt))
    {
        Type condType = analyzeExpression(ifstmt->condition.get());

        Type boolType;
        boolType.base = "bool";

        if (condType != boolType)
            throw std::runtime_error("Condition must be bool");

        analyzeBlock(ifstmt->thenBranch.get());

        if (ifstmt->elseBranch)
            analyzeBlock(ifstmt->elseBranch.get());
    }

    else if (auto wh = dynamic_cast<WhileStmt*>(stmt))
    {
        Type condType = analyzeExpression(wh->condition.get());

        Type boolType;
        boolType.base = "bool";

        if (condType != boolType)
            throw std::runtime_error("While condition must be bool");

        analyzeBlock(wh->body.get());
    }


    else if (auto exprStmt = dynamic_cast<ExpressionStmt*>(stmt))
    {
        analyzeExpression(exprStmt->expression.get());
    }
}

Type SemanticAnalyzer::analyzeExpression(Expr* expr)
{
    // ===== LITERAL =====
    if (auto lit = dynamic_cast<LiteralExpr*>(expr))
    {
        Type t;

        if (lit->value == "true" || lit->value == "false")
        {
            t.base = "bool";
            return t;
        }

        // char literal: 'a'
        if (lit->value.size() >= 3 &&
            lit->value.front() == '\'' &&
            lit->value.back() == '\'')
        {
            t.base = "char";
            return t;
        }

        // default: number → int
        t.base = "int";
        return t;
    }

    // ===== VARIABLE =====
    if (auto var = dynamic_cast<VarExpr*>(expr))
    {
        Symbol* sym = symbols.lookup(var->name);

        if (!sym)
            throw std::runtime_error("Undefined identifier: " + var->name);

        if (sym->kind == SymbolKind::Function)
            throw std::runtime_error("Function used as variable: " + var->name);

        return sym->type;
    }

    // ===== FUNCTION CALL =====
    if (auto call = dynamic_cast<CallExpr*>(expr))
    {
        std::string lookupName = call->callee;

        if (!call->moduleName.empty())
        {
            lookupName = call->moduleName + "__" + call->callee;
        }

        Symbol* sym = symbols.lookup(lookupName);


        if (!sym)
            throw std::runtime_error("Undefined function: " + call->callee);

        if (sym->kind != SymbolKind::Function)
            throw std::runtime_error("Variable used as function: " + call->callee);

        if (call->arguments.size() != sym->paramTypes.size())
            throw std::runtime_error("Incorrect argument count in call to: " + call->callee);

        for (size_t i = 0; i < call->arguments.size(); ++i)
        {
            Type argType = analyzeExpression(call->arguments[i].get());

            Type paramType = sym->paramTypes[i];

            if (argType == paramType)
            {
                // exact match
            }
            else if (argType.isArray &&
                    paramType.isPointer &&
                    argType.base == paramType.base)
            {
                // array decays to pointer
            }
            else if (!areTypesCompatible(argType, paramType))
            {
                throw std::runtime_error("Argument type mismatch in call to: " + call->callee);
            }


        }

        return sym->type;
    }


    // ===== UNARY =====
    if (auto unary = dynamic_cast<UnaryExpr*>(expr))
    {
        Type operandType = analyzeExpression(unary->operand.get());

        if (unary->op == "-")
        {
            if (operandType.base != "int")
                throw std::runtime_error("Unary minus only supported on int");

            Type t;
            t.base = "int";
            return t;
        }

        throw std::runtime_error("Unknown unary operator: " + unary->op);
    }


    // ===== BINARY =====
    if (auto bin = dynamic_cast<BinaryExpr*>(expr))
    {
        Type leftType  = analyzeExpression(bin->left.get());
        Type rightType = analyzeExpression(bin->right.get());

        if (!areTypesCompatible(leftType, rightType))
            throw std::runtime_error("Type mismatch in binary expression");


        // Comparison operators → bool
        if (bin->op == "==" || bin->op == "!=" ||
            bin->op == "<"  || bin->op == ">"  ||
            bin->op == "<=" || bin->op == ">=")
        {
            Type t;
            t.base = "bool";
            return t;
        }

        // Arithmetic → int only
        if (bin->op == "+" || bin->op == "-" ||
            bin->op == "*" || bin->op == "/" ||
            bin->op == "%")
        {
            if (leftType.base != "int" && leftType.base != "char")
                throw std::runtime_error("Arithmetic only supported on int/char");

            Type t;
            t.base = "int";
            return t;
        }

        throw std::runtime_error("Unknown binary operator: " + bin->op);
    }

    // ===== STRING LITERAL =====
    if (auto str = dynamic_cast<StringExpr*>(expr))
    {
        Type t;
        t.base = "char";
        t.isPointer = true;  // string = char*
        return t;
    }

    // ===== ARRAY INDEX =====
    if (auto index = dynamic_cast<IndexExpr*>(expr))
    {
        Type baseType = analyzeExpression(index->base.get());

        if (!baseType.isPointer && !baseType.isArray)
            throw std::runtime_error("Indexing non-array variable");


        Type indexType = analyzeExpression(index->index.get());

        if (indexType.base != "int")
            throw std::runtime_error("Array index must be int");

        Type elementType = baseType;
        elementType.isPointer = false;  // char* → char
        elementType.isArray = false;
        return elementType;

    }

    throw std::runtime_error("Unknown expression in semantic analysis");
}




}
