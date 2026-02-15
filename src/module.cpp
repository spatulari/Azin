#include "module.hpp"
#include "lexer.hpp"
#include "parser.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>

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

    loadFileRecursive(entryPath, mergedDecls);

    Program program;
    program.decls = std::move(mergedDecls);

    return program;
}

void ModuleLoader::loadFileRecursive(const std::string& path,
                                     std::vector<TopLevelDecl>& mergedDecls)
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

    for (auto& decl : program.decls)
    {
        if (std::holds_alternative<UseDecl>(decl))
        {
            const auto& use = std::get<UseDecl>(decl);
            loadFileRecursive(use.path, mergedDecls);
        }
        else
        {
            mergedDecls.push_back(std::move(decl));
        }
    }
}

}
