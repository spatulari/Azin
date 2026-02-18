#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

#include "lexer.hpp"
#include "parser.hpp"
#include "ast.hpp"
#include "codegen.hpp"
#include "semantic.hpp"
#include "module.hpp"


using namespace azin;

#include <filesystem>

std::string removeExtension(const std::string& filename)
{
    std::filesystem::path p(filename);
    return p.stem().string();
}

// File Reading

static std::string readFile(const std::string& path)
{
    std::ifstream file(path, std::ios::in | std::ios::binary);
    if (!file)
        throw std::runtime_error("Cannot open file: " + path);

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// Token Debug Dump

static const char* tokenTypeToString(TokenType type)
{
    switch (type)
    {
        case TokenType::IDENTIFIER: return "IDENTIFIER";
        case TokenType::NUMBER: return "NUMBER";
        case TokenType::STRING: return "STRING";
        case TokenType::CHAR_LITERAL: return "CHAR_LIT";


        case TokenType::RETURN: return "RETURN";
        case TokenType::IF: return "IF";
        case TokenType::ELSE: return "ELSE";
        case TokenType::WHILE: return "WHILE";
        case TokenType::FOR: return "FOR";
        case TokenType::EXTERN: return "EXTERN";

        case TokenType::TYPE_INT: return "TYPE_INT";
        case TokenType::TYPE_NORE: return "TYPE_NORE";
        case TokenType::TYPE_U8: return "TYPE_U8";
        case TokenType::TYPE_U16: return "TYPE_U16";
        case TokenType::TYPE_U32: return "TYPE_U32";
        case TokenType::TYPE_U64: return "TYPE_U64";
        case TokenType::TYPE_I8: return "TYPE_I8";
        case TokenType::TYPE_I16: return "TYPE_I16";
        case TokenType::TYPE_I32: return "TYPE_I32";
        case TokenType::TYPE_I64: return "TYPE_I64";
        case TokenType::TYPE_BOOL: return "TYPE_BOOL";
        case TokenType::TYPE_CHAR: return "TYPE_CHAR";
        case TokenType::TRUE: return "TRUE";
        case TokenType::FALSE: return "FALSE";
        case TokenType::USE_DIRECTIVE: return "USE_DIRECTIVE";

        // TODO: Add the other types if i forget

        case TokenType::LPAREN: return "LPAREN";
        case TokenType::RPAREN: return "RPAREN";
        case TokenType::LBRACKET: return "LBRACKET";
        case TokenType::RBRACKET: return "RBRACKET";
        case TokenType::LBRACE: return "LBRACE";
        case TokenType::RBRACE: return "RBRACE";
        case TokenType::SEMICOLON: return "SEMICOLON";
        case TokenType::COMMA: return "COMMA";
        case TokenType::AT: return "AT";

        case TokenType::EQUAL: return "EQUAL";
        case TokenType::PLUS: return "PLUS";
        case TokenType::MINUS: return "MINUS";
        case TokenType::STAR: return "STAR";
        case TokenType::SLASH: return "SLASH";
        case TokenType::AMPERSAND: return "AMPERSAND";
        case TokenType::PIPE: return "PIPE";

        case TokenType::EQEQ: return "EQEQ";
        case TokenType::NOTEQ: return "NOTEQ";
        case TokenType::LT: return "LT";
        case TokenType::GT: return "GT";
        case TokenType::LTEQ: return "LTEQ";
        case TokenType::GTEQ: return "GTEQ";
        

        case TokenType::END_OF_FILE: return "EOF";
        case TokenType::UNKNOWN: return "UNKNOWN";
    }

    return "INVALID";
}

static void dumpTokens(const std::vector<Token>& tokens)
{
    std::cout << "\n===== TOKEN DUMP BEGIN =====\n";

    for (const auto& token : tokens)
    {
        std::cout
            << "[" << token.line << ":" << token.column << "] "
            << tokenTypeToString(token.type)
            << " -> \"" << token.lexeme << "\"\n";
    }

    std::cout << "===== TOKEN DUMP END =====\n\n";
}

// AST Debug Dump

static void indent(int depth)
{
    for (int i = 0; i < depth; ++i)
        std::cout << "  ";
}

static void dumpExpr(const Expr* expr, int depth);

static void dumpStmt(const Stmt* stmt, int depth)
{
    if (auto var = dynamic_cast<const VarDeclStmt*>(stmt))
    {
        indent(depth);
        std::cout << "VarDeclStmt\n";

        indent(depth + 1);
        std::cout << "Type: " << var->type << "\n";

        indent(depth + 1);
        std::cout << "Name: " << var->name << "\n";

        if (var->initializer)
        {
            indent(depth + 1);
            std::cout << "Initializer:\n";
            dumpExpr(var->initializer.get(), depth + 2);
        }
        else
        {
            indent(depth + 1);
            std::cout << "No Initializer\n";
        }

    }
    else if (auto assign = dynamic_cast<const AssignmentStmt*>(stmt))
    {
        indent(depth);
        std::cout << "AssignmentStmt\n";

        indent(depth + 1);
        std::cout << "Target:\n";
        dumpExpr(assign->target.get(), depth + 2);

        indent(depth + 1);
        std::cout << "Value:\n";
        dumpExpr(assign->value.get(), depth + 2);
    }

    else if (auto ret = dynamic_cast<const ReturnStmt*>(stmt))
    {
        indent(depth);
        std::cout << "ReturnStmt\n";
        dumpExpr(ret->value.get(), depth + 1);
    }
    else if (auto ifs = dynamic_cast<const IfStmt*>(stmt))
    {
        indent(depth);
        std::cout << "IfStmt\n";

        indent(depth + 1);
        std::cout << "Condition:\n";
        dumpExpr(ifs->condition.get(), depth + 2);

        indent(depth + 1);
        std::cout << "Then:\n";
        for (const auto& stmt : ifs->thenBranch->statements)
            dumpStmt(stmt.get(), depth + 2);

        if (ifs->elseBranch)
        {
            indent(depth + 1);
            std::cout << "Else:\n";
            for (const auto& stmt : ifs->elseBranch->statements)
                dumpStmt(stmt.get(), depth + 2);
        }

        return;
    }
    else if (auto exprStmt = dynamic_cast<const ExpressionStmt*>(stmt))
    {
        indent(depth);
        std::cout << "ExpressionStmt\n";

        dumpExpr(exprStmt->expression.get(), depth + 1);
    }


    else
    {
        indent(depth);
        std::cout << "UnknownStmt\n";
    }
}


static void dumpExpr(const Expr* expr, int depth)
{
    if (auto lit = dynamic_cast<const LiteralExpr*>(expr))
    {
        indent(depth);
        std::cout << "LiteralExpr: " << lit->value << "\n";
    }
    else if (auto bin = dynamic_cast<const BinaryExpr*>(expr))
    {
        indent(depth);
        std::cout << "BinaryExpr: " << bin->op << "\n";
        dumpExpr(bin->left.get(), depth + 1);
        dumpExpr(bin->right.get(), depth + 1);
    }
    else if (auto var = dynamic_cast<const VarExpr*>(expr))
    {
        indent(depth);
        std::cout << "VarExpr: " << var->name << "\n";
    }
    else if (auto call = dynamic_cast<const CallExpr*>(expr))
    {
        indent(depth);
        std::cout << "CallExpr: " << call->callee << "\n";

        indent(depth + 1);
        std::cout << "Arguments:\n";

        for (const auto& arg : call->arguments)
            dumpExpr(arg.get(), depth + 2);
    }
    else if (auto idx = dynamic_cast<const IndexExpr*>(expr))
    {
        indent(depth);
        std::cout << "IndexExpr\n";

        indent(depth + 1);
        std::cout << "Base:\n";
        dumpExpr(idx->base.get(), depth + 2);

        indent(depth + 1);
        std::cout << "Index:\n";
        dumpExpr(idx->index.get(), depth + 2);
    }
    else if (auto str = dynamic_cast<const StringExpr*>(expr))
    {
        indent(depth);
        std::cout << "StringExpr: \"" << str->value << "\"\n";
    }



    else
    {
        indent(depth);
        std::cout << "UnknownExpr\n";
    }
}

static void dumpAST(const Program& program)
{
    std::cout << "\n===== AST DUMP BEGIN =====\n";

    for (const auto& decl : program.decls)
    {
        if (std::holds_alternative<FunctionDecl>(decl))
        {
            const auto& fn = std::get<FunctionDecl>(decl);

            std::cout << "FunctionDecl\n";

            indent(1);
            std::cout << "ReturnType: " << fn.returnType << "\n";

            indent(1);
            std::cout << "Name: " << fn.name << "\n";

            indent(1);
            std::cout << "Body:\n";

            if (fn.body)
            {
                for (const auto& stmt : fn.body->statements)
                    dumpStmt(stmt.get(), 2);
            }
            else
            {
                indent(1);
                std::cout << "Extern Function (no body)\n";
            }
        }
        else if (std::holds_alternative<UseDecl>(decl))
        {
            const auto& use = std::get<UseDecl>(decl);

            std::cout << "UseDecl\n";

            indent(1);
            std::cout << "Path: " << use.path << "\n";
        }
    }

    std::cout << "===== AST DUMP END =====\n\n";
}


int main(int argc, char** argv)
{
    std::ofstream debugFile("debug.log", std::ios::out | std::ios::trunc);
    if (!debugFile)
        return 1;

    std::streambuf* coutBuf = std::cout.rdbuf();
    std::streambuf* cerrBuf = std::cerr.rdbuf();

    std::cout.rdbuf(debugFile.rdbuf());

    std::cout << "===== AZC DEBUG SESSION START =====\n";

    try
    {
        if (argc < 2)
            throw std::runtime_error("Usage: azc <file.az>");

        std::string sourcePath = argv[1];
        std::string baseName = removeExtension(sourcePath);
        std::string cFileName = baseName + ".c";

#ifdef _WIN32
        std::string exeFileName = baseName + ".exe";
#else
        std::string exeFileName = baseName;
#endif

        // =========================
        // READ FILE
        // =========================

        std::cout << "Reading file: " << sourcePath << "\n";

        std::string source = readFile(sourcePath);

        std::cout << "File loaded successfully.\n";
        std::cout << "File size: " << source.size() << " bytes\n";

        // =========================
        // LEXER
        // =========================

        std::cout << "\n--- Starting Lexical Analysis ---\n";

        Lexer lexer(source);
        auto tokens = lexer.tokenize();

        std::cout << "Lexical analysis complete.\n";
        std::cout << "Total tokens: " << tokens.size() << "\n";

        dumpTokens(tokens);

        // =========================
        // PARSER
        // =========================

        std::cout << "\n--- Starting Parsing ---\n";

        ModuleLoader loader;
        Program program = loader.loadProgramWithModules(sourcePath);


        std::cout << "Parsing completed successfully.\n";

        // =========================
        // AST DUMP
        // =========================

        dumpAST(program);

        // =========================
        // SEMANTIC ANALYSIS
        // =========================

        std::cout << "\n--- Starting Semantic Analysis ---\n";

        SemanticAnalyzer analyzer;
        analyzer.analyze(program);

        std::cout << "Semantic analysis complete.\n";

        // =========================
        // CODEGEN
        // =========================

        std::cout << "\n--- Starting C Code Generation ---\n";

        std::string cCode = CodegenC::generate(program);

        std::ofstream cFile(cFileName);
        cFile << cCode;
        cFile.close();

        std::cout << "C source written to " << cFileName << "\n";

        // =========================
        // BUILD
        // =========================

        std::string command = "gcc " + cFileName + " -o " + exeFileName;

        std::cout << "Running: " << command << "\n";

        int result = std::system(command.c_str());

        if (result != 0)
            throw std::runtime_error("GCC compilation failed.");

        std::cout << "Compilation successful: " << exeFileName << "\n";
    }
    catch (const std::exception& e)
    {
        // Full debug info to log
        std::cout << "\n!!! EXCEPTION CAUGHT !!!\n";
        std::cout << e.what() << "\n";

        // Clean message to user
        std::cerr << e.what() << "\n";
    }
    catch (...)
    {
        std::cout << "\n!!! UNKNOWN CRITICAL ERROR !!!\n";
        std::cerr << "Unknown critical error\n";
    }


    std::cout << "\n===== AZC DEBUG SESSION END =====\n";

    std::cout.rdbuf(coutBuf);
    std::cerr.rdbuf(cerrBuf);
    debugFile.close();

    return 0;
}

