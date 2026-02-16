#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>

#include "lexer.hpp"
#include "parser.hpp"
#include "semantic.hpp"

using namespace azin;

static std::string readFile(const std::filesystem::path& p)
{
    std::ifstream file(p);
    if (!file)
        throw std::runtime_error("Cannot open file: " + p.string());

    std::stringstream buf;
    buf << file.rdbuf();
    return buf.str();
}

int main()
{
    std::filesystem::path testsDir = std::filesystem::path("tests") / "syntax";

    if (!std::filesystem::exists(testsDir))
    {
        std::cerr << "Tests directory not found: " << testsDir << "\n";
        return 2;
    }

    int total = 0;
    int passed = 0;

    for (auto &entry : std::filesystem::directory_iterator(testsDir))
    {
        if (!entry.is_regular_file()) continue;
        if (entry.path().extension() != ".az") continue;

        ++total;
        std::cout << "Running: " << entry.path().string() << " ... ";

        try {
            std::string src = readFile(entry.path());

            Lexer lexer(src);
            auto tokens = lexer.tokenize();

            Parser parser(tokens);
            Program program = parser.parse();

            SemanticAnalyzer sem;
            sem.analyze(program);

            std::cout << "OK\n";
            ++passed;
        }
        catch (const std::exception &e)
        {
            std::cout << "FAIL - " << e.what() << "\n";
        }
    }

    std::cout << "\nPassed " << passed << " / " << total << " tests.\n";
    return (passed == total) ? 0 : 1;
}
