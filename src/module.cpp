#include "module.hpp"
#include "lexer.hpp"
#include "parser.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <unordered_set>
#include <filesystem>

namespace azin
{

std::string ModuleLoader::readFile(const std::string& path)
{
    std::ifstream file(path);
    if (!file)
        throw std::runtime_error("Cannot open file: " + path);

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

Program ModuleLoader::loadProgramWithModules(const std::string& entryPath)
{
    std::vector<TopLevelDecl> mergedDecls;

    // Entry file is not mangled
    loadFileRecursive(entryPath, mergedDecls, true);

    Program program;
    program.decls = std::move(mergedDecls);

    return program;
}


static void mangleCallsInExpr(Expr* expr,
                              const std::string& moduleName,
                              const std::unordered_set<std::string>& localFunctions);

static void mangleCallsInStmt(Stmt* stmt,
                             const std::string& moduleName,
                             const std::unordered_set<std::string>& localFunctions);

static void mangleCallsInBlock(BlockStmt* block,
                        const std::string& moduleName,
                        const std::unordered_set<std::string>& localFunctions)
{
    for (auto& stmt : block->statements)
    {
        mangleCallsInStmt(stmt.get(), moduleName, localFunctions);
    }
}

static void mangleCallsInStmt(Stmt* stmt,
                             const std::string& moduleName,
                             const std::unordered_set<std::string>& localFunctions)
{
    if (!stmt) return;

    if (auto exprStmt = dynamic_cast<ExpressionStmt*>(stmt))
    {
        if (exprStmt->expression)
            mangleCallsInExpr(exprStmt->expression.get(), moduleName, localFunctions);
        return;
    }

    if (auto var = dynamic_cast<VarDeclStmt*>(stmt))
    {
        if (var->initializer)
            mangleCallsInExpr(var->initializer.get(), moduleName, localFunctions);
        return;
    }

    if (auto ret = dynamic_cast<ReturnStmt*>(stmt))
    {
        if (ret->value)
            mangleCallsInExpr(ret->value.get(), moduleName, localFunctions);
        return;
    }

    if (auto assign = dynamic_cast<AssignmentStmt*>(stmt))
    {
        if (assign->target)
            mangleCallsInExpr(assign->target.get(), moduleName, localFunctions);
        if (assign->value)
            mangleCallsInExpr(assign->value.get(), moduleName, localFunctions);
        return;
    }

    if (auto wh = dynamic_cast<WhileStmt*>(stmt))
    {
        if (wh->condition)
            mangleCallsInExpr(wh->condition.get(), moduleName, localFunctions);
        if (wh->body)
            mangleCallsInBlock(wh->body.get(), moduleName, localFunctions);
        return;
    }

    if (auto ifs = dynamic_cast<IfStmt*>(stmt))
    {
        if (ifs->condition)
            mangleCallsInExpr(ifs->condition.get(), moduleName, localFunctions);
        if (ifs->thenBranch)
            mangleCallsInBlock(ifs->thenBranch.get(), moduleName, localFunctions);
        if (ifs->elseBranch)
            mangleCallsInBlock(ifs->elseBranch.get(), moduleName, localFunctions);
        return;
    }

    if (auto block = dynamic_cast<BlockStmt*>(stmt))
    {
        mangleCallsInBlock(block, moduleName, localFunctions);
        return;
    }
}

static void mangleCallsInExpr(Expr* expr,
                              const std::string& moduleName,
                              const std::unordered_set<std::string>& localFunctions)
{
    if (!expr) return;

    if (auto call = dynamic_cast<CallExpr*>(expr))
    {
        // Only remap unqualified calls to local functions
        if (call->moduleName.empty() && localFunctions.count(call->callee))
        {
            call->callee = moduleName + "__" + call->callee;
        }

        // Recurse into arguments
        for (auto& arg : call->arguments)
            mangleCallsInExpr(arg.get(), moduleName, localFunctions);

        return;
    }

    if (auto bin = dynamic_cast<BinaryExpr*>(expr))
    {
        if (bin->left) mangleCallsInExpr(bin->left.get(), moduleName, localFunctions);
        if (bin->right) mangleCallsInExpr(bin->right.get(), moduleName, localFunctions);
        return;
    }

    if (auto un = dynamic_cast<UnaryExpr*>(expr))
    {
        if (un->operand) mangleCallsInExpr(un->operand.get(), moduleName, localFunctions);
        return;
    }

    if (auto idx = dynamic_cast<IndexExpr*>(expr))
    {
        if (idx->base) mangleCallsInExpr(idx->base.get(), moduleName, localFunctions);
        if (idx->index) mangleCallsInExpr(idx->index.get(), moduleName, localFunctions);
        return;
    }

    if (auto var = dynamic_cast<VarExpr*>(expr))
    {
        // nothing to do
        return;
    }

    if (auto lit = dynamic_cast<LiteralExpr*>(expr))
    {
        // nothing to do
        return;
    }

    if (auto str = dynamic_cast<StringExpr*>(expr))
    {
        // nothing to do
        return;
    }

}


void ModuleLoader::loadFileRecursive(const std::string& path,
                                     std::vector<TopLevelDecl>& mergedDecls,
                                     bool isEntry)
{
    if (loadedModules.count(path))
        return;

    loadedModules.insert(path);

    std::cout << "\n--- Loading Module: " << path << " ---\n";

    std::string source = readFile(path);

    Lexer lexer(source);
    auto tokens = lexer.tokenize();

    Parser parser(tokens, path);
    Program program = parser.parse();

    // Extract module name from filename (use filesystem to handle paths reliably)
    std::filesystem::path modulePath(path);
    std::string moduleName = modulePath.stem().string();


    std::unordered_set<std::string> localFunctions;

    for (auto& decl : program.decls)
    {
        if (std::holds_alternative<FunctionDecl>(decl))
        {
            auto& fn = std::get<FunctionDecl>(decl);

            if (!fn.isExtern)
                localFunctions.insert(fn.name);
        }
    }


    for (auto& decl : program.decls)
    {
        if (std::holds_alternative<UseDecl>(decl))
        {
            const auto& use = std::get<UseDecl>(decl);

            // Recursive load (modules are NOT entry)
            loadFileRecursive(use.path, mergedDecls, false);
        }
        else if (std::holds_alternative<FunctionDecl>(decl))
        {
            auto& fn = std::get<FunctionDecl>(decl);

            if (!isEntry && !fn.isExtern)
            {
                if (fn.body)
                    mangleCallsInBlock(fn.body.get(), moduleName, localFunctions);

                fn.name = moduleName + "__" + fn.name;
            }

            mergedDecls.push_back(std::move(decl));
        }

        else
        {
            mergedDecls.push_back(std::move(decl));
        }
    }
}

}
