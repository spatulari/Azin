#include "codegen.hpp"

#include <iostream>   // cout, cerr
#include <fstream>    // ifstream
#include <sstream>    // stringstream
#include <string>     // std::string
#include <vector>     // later for tokens
#include <cctype>     // isalpha, isdigit, isspace
#include "Lexer.hpp"
#include "Parser.hpp"


int main(int argc, char** argv)
{
    if (argc != 2) //IMPORTANT: If i ever want more than 1 arg, then argc < 2
    {
        std::cerr << "Usage: azin <file>\n";
        return 1;
    }

    std::ifstream file(argv[1]);

    
    if (!file) {
        std::cerr << "Error: could not open file: " << argv[1] << "\n";
        return 1;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string src = buffer.str();

    if (src.empty())
    {
        std::cerr << "Error: " << argv[1] << " is empty\n";
        return 1;
    }

    std::cout << src << "\n\n";

    Lexer lexer(src);
    std::vector<Token> tokens = lexer.tokenize();
    for (const auto& tok : tokens) 
    {
        std::cout << "Token type: " << tok.type << " | value: \"" << tok.value << "\"\n";
    }
    Parser parser = make_parser(tokens);

    std::vector<Stmt*> program = parse_program(parser);

    for (Stmt* s : program)
    {
        print_stmt(s);
    }

    std::cout << "\nOutput:\n\n";

    for (Stmt* s : program)
    {
        exec_stmt(s);
    }

    return 0;
    
}