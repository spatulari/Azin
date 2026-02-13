#include "codegen.hpp"
#include <sstream>
#include <stdexcept>
#include <stdbool.h>



namespace azin
{

// Static State

int CodegenC::indentLevel = 0;


// Indentation Helpers

std::string CodegenC::indent()
{
    return std::string(indentLevel * 4, ' ');
}

void CodegenC::increaseIndent()
{
    indentLevel++;
}

void CodegenC::decreaseIndent()
{
    indentLevel--;
}


std::string CodegenC::mapTypeToC(const std::string& type)
{
    if (type == "int")  return "int";
    if (type == "nore") return "void";

    if (type == "i8")  return "int8_t";
    if (type == "i16") return "int16_t";
    if (type == "i32") return "int32_t";
    if (type == "i64") return "int64_t";

    if (type == "u8")  return "uint8_t";
    if (type == "u16") return "uint16_t";
    if (type == "u32") return "uint32_t";
    if (type == "u64") return "uint64_t";
    if (type == "bool") return "bool";

    throw std::runtime_error("Unknown type: " + type);
}


// Entry Point

std::string CodegenC::generate(const Program& program)
{
    std::stringstream out;

    out << "#include <stdint.h>\n\n";

    for (const auto& fn : program.functions)
        out << generateFunction(fn);

    return out.str();
}


// Function Generation

std::string CodegenC::generateFunction(const FunctionDecl& fn)
{
    std::stringstream out;

    out << mapTypeToC(fn.returnType) << " " << fn.name << "(";

    for (size_t i = 0; i < fn.params.size(); ++i)
    {
        const auto& p = fn.params[i];

        out << mapTypeToC(p.type) << " " << p.name;

        if (i + 1 < fn.params.size())
            out << ", ";
    }

    out << ") {\n";


    increaseIndent();

    for (const auto& stmt : fn.body->statements)
    {
        out << indent() << generateStatement(stmt.get());
    }

    decreaseIndent();

    out << "}\n\n";

    return out.str();
}


// Statement Generation

std::string CodegenC::generateStatement(const Stmt* stmt)
{
    if (auto ret = dynamic_cast<const ReturnStmt*>(stmt))
    {
        return "return " +
               generateExpression(ret->value.get()) +
               ";\n";
    }

    if (auto var = dynamic_cast<const VarDeclStmt*>(stmt))
    {
        return mapTypeToC(var->type) + " " +
            var->name + " = " +
            generateExpression(var->initializer.get()) +
            ";\n";
    }

    if (auto assign = dynamic_cast<const AssignmentStmt*>(stmt))
    {
        return assign->name + " = " +
            generateExpression(assign->value.get()) +
            ";\n";
    }

    if (auto ifstmt = dynamic_cast<const IfStmt*>(stmt))
    {
        std::stringstream out;

        out << "if (" 
            << generateExpression(ifstmt->condition.get())
            << ") {\n";

        increaseIndent();

        for (const auto& s : ifstmt->thenBranch->statements)
            out << indent() << generateStatement(s.get());

        decreaseIndent();

        out << indent() << "}";

        if (ifstmt->elseBranch)
        {
            out << " else {\n";
            increaseIndent();

            for (const auto& s : ifstmt->elseBranch->statements)
                out << indent() << generateStatement(s.get());

            decreaseIndent();

            out << indent() << "}";
        }

        out << "\n";
        return out.str();
    }




    throw std::runtime_error("Unsupported statement in C codegen");
}


// Expression Generation

std::string CodegenC::generateExpression(const Expr* expr)
{
    if (auto lit = dynamic_cast<const LiteralExpr*>(expr))
        return lit->value;

    if (auto bin = dynamic_cast<const BinaryExpr*>(expr))
    {
        return "(" +
               generateExpression(bin->left.get()) +
               " " + bin->op + " " +
               generateExpression(bin->right.get()) +
               ")";
    }
    if (auto var = dynamic_cast<const VarExpr*>(expr))
    {
        return var->name;
    }
    if (auto call = dynamic_cast<const CallExpr*>(expr))
    {
        std::stringstream out;

        out << call->callee << "(";

        for (size_t i = 0; i < call->arguments.size(); i++)
        {
            out << generateExpression(call->arguments[i].get());
            if (i + 1 < call->arguments.size())
                out << ", ";
        }

        out << ")";

        return out.str();
    }


    throw std::runtime_error("Unsupported expression in C codegen");
}

}
