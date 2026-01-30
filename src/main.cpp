// LOC ~= 1374

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
        std::cerr << "Usage: azc <file.az> [-d]\n";
        return 1;
    }

    std::string input = argv[1];
    if (argc >= 3 && std::string(argv[2]) == "-d")
    {
        debug = true;
        std::cerr << "DEBUG MODE\n";
    }
    else if (argc >= 2 && std::string(argv[1]) == "-d")
    {
        debug = true;
        std::cerr << "DEBUG MODE\n";
        if (argc < 3)
        {
            std::cerr << "Usage: azc <file.az> [-d] or azc -d <file.az>\n";
            return 1;
        }
        input = argv[2];
    }

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

    std::ofstream dlog;
    if (debug)
    {
        dlog.open("debug.log");
        if (!dlog)
        {
            std::cerr << "Error: could not open debug.log\n";
            return 1;
        }
        dlog << "DEBUG LOG\n";
        dlog << "Input file: " << input << "\n";
        dlog << "Source code:\n" << src << "\n\n";
    }

    Lexer lexer(src);
    auto tokens = lexer.tokenize();

    if (debug)
    {
        dlog << "Tokens:\n";
        for (const auto& token : tokens)
        {
            dlog << "Token: " << token.type << " : '" << token.value << "'\n";
        }
        dlog << "\n";
    }

    Parser parser = make_parser(tokens);
    Function* fn = parse_program(parser);


    if (debug)
    {
        dlog << "Parsed AST:\n";
        dlog << "Number of statements: " << fn->body.size() << "\n";
        for (size_t i = 0; i < fn->body.size(); ++i)
        {
            dlog << "Statement " << i << ": ";
            Stmt* s = fn->body[i];
            if (s->type == STMT_RETURN) dlog << "RETURN\n";
            else if (s->type == STMT_EXPR) dlog << "EXPR\n";
            else if (s->type == STMT_ASSIGN) dlog << "ASSIGN to " << s->var_name << "\n";
            else if (s->type == STMT_DECL) dlog << "DECL " << s->var_name << "\n";
            else if (s->type == STMT_USE) dlog << "USE " << s->module_name << "\n";
            else dlog << "UNKNOWN\n";
        }
        dlog << "\n";
    }

    gen_program(fn, base + ".asm");

    if (debug)
    {
        std::ifstream asmfile(base + ".asm");
        if (asmfile)
        {
            std::stringstream asm_buffer;
            asm_buffer << asmfile.rdbuf();
            dlog << "Generated Assembly:\n" << asm_buffer.str() << "\n";
        }
        else
        {
            dlog << "Could not read generated assembly file.\n";
        }
    }

    std::string cmd =
        "nasm -f elf64 " + base + ".asm -o " + base + ".o && "
        "gcc -no-pie " + base + ".o -o " + base;

    if (debug)
    {
        dlog << "Build command: " << cmd << "\n";
    }

    int ret = system(cmd.c_str());

    if (debug)
    {
        dlog << "Build return code: " << ret << "\n";
    }

    if (ret != 0)
    {
        std::cerr << "Error: assembling or linking failed\n";
        if (debug) dlog << "Build failed.\n";
        return 1;
    }

    std::remove((base + ".o").c_str());
    std::remove((base + ".asm").c_str());

    std::cout << "[OK] built " << base << "\n";
    if (debug) dlog << "Build successful. Output: " << base << "\n";
    return 0;
}
