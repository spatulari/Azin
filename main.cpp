#include "asmgen.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cstdlib>   // system()

bool debug = false;

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cerr << "Usage: azc <file.az>\n";
        return 1;
    }
    //FIXME: Debug mode doesnt get triggered for some reason
    if (argv[2] == "-d") {debug = true; std::cout << "DEBUG MODE"; }

    std::string input = argv[1];

    if (input.size() < 3 || input.substr(input.size() - 3) != ".az")
    {
        std::cerr << "Error: input must be a .az file\n";
        return 1;
    }

    std::string base = input.substr(0, input.size() - 3);

    std::ifstream file(input);
    if (!file)
    {
        std::cerr << "Error: could not open file: " << input << "\n";
        return 1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string src = buffer.str();

    if (src.empty())
    {
        std::cerr << "Error: input file is empty\n";
        return 1;
    }


    std::ofstream dlog("debug.log");    // dlog = debug log
    if (!dlog)
    {
        std::cerr << "Error: could not open debug.log";
    }


    Lexer lexer(src);
    auto tokens = lexer.tokenize();

    if (debug)
    {
        for (const auto& token : tokens)
        {
            dlog << token.type << " : " << token.value << '\n';
        }
    }


    Parser parser = make_parser(tokens);
    auto program = parse_program(parser);

    gen_program(program, base + ".asm"); // emits base.asm 

    std::string cmd =
        "nasm -f elf64 " + base + ".asm -o " + base + ".o && "
        "gcc -no-pie " + base + ".o -o " + base;

    int ret = system(cmd.c_str());

    if (ret != 0)
    {
        std::cerr << "Error: assembling or linking failed\n";
        return 1;
    }

    std::remove((base + ".o").c_str());
    std::remove((base + ".asm").c_str());

    std::cout << "[OK] built " << base << "\n";
    return 0;
}
