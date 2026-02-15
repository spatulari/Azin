#pragma once

#include "ast.hpp"
#include <string>
#include <unordered_set>
#include <vector>

namespace azin
{

class ModuleLoader
{
public:
    Program loadProgramWithModules(const std::string& entryPath);

private:
    std::unordered_set<std::string> loadedModules;

    void loadFileRecursive(const std::string& path,
                           std::vector<TopLevelDecl>& mergedDecls);

    std::string readFile(const std::string& path);

    
    void loadFileRecursive(const std::string& path,
                                        std::vector<TopLevelDecl>& mergedDecls,
                                        bool isEntry);
};

}
