#include "module.hpp"
#include "lexer.hpp"
#include "parser.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <unordered_set>

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


void mangleCallsInBlock(BlockStmt* block,
                        const std::string& moduleName,
                        const std::unordered_set<std::string>& localFunctions)
{
    for (auto& stmt : block->statements)
    {
        if (auto exprStmt = dynamic_cast<ExpressionStmt*>(stmt.get()))
        {
            if (auto call = dynamic_cast<CallExpr*>(exprStmt->expression.get()))
            {
                if (localFunctions.count(call->callee))
                {
                    call->callee = moduleName + "__" + call->callee;
                }

            }
        }
        else if (auto var = dynamic_cast<VarDeclStmt*>(stmt.get()))
        {
            if (var->initializer)
            {
                if (auto call = dynamic_cast<CallExpr*>(var->initializer.get()))
                {
                    if (localFunctions.count(call->callee))
                    {
                        call->callee = moduleName + "__" + call->callee;
                    }

                }
            }
        }
        else if (auto ret = dynamic_cast<ReturnStmt*>(stmt.get()))
        {
            if (ret->value)
            {
                if (auto call = dynamic_cast<CallExpr*>(ret->value.get()))
                {
                    if (localFunctions.count(call->callee))
                    {
                        call->callee = moduleName + "__" + call->callee;
                    }

                }
            }
        }
        else if (auto wh = dynamic_cast<WhileStmt*>(stmt.get()))
        {
            if (wh->body)
                mangleCallsInBlock(wh->body.get(), moduleName, localFunctions);

        }
        else if (auto ifs = dynamic_cast<IfStmt*>(stmt.get()))
        {
            if (ifs->thenBranch)
                mangleCallsInBlock(ifs->thenBranch.get(), moduleName, localFunctions);

            if (ifs->elseBranch)
                mangleCallsInBlock(ifs->elseBranch.get(), moduleName, localFunctions);
        }
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

    Parser parser(tokens);
    Program program = parser.parse();

    // Extract module name from filename
    std::string moduleName = path.substr(0, path.find('.'));


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
