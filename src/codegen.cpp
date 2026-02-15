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

std::string CodegenC::generateHeader(const Program& program)
{
    std::stringstream out;

    out << "#pragma once\n\n";

    for (const auto& decl : program.decls)
    {
        if (std::holds_alternative<FunctionDecl>(decl))
        {
            const auto& fn = std::get<FunctionDecl>(decl);

            if (fn.isExtern)
                continue;

            out << mapTypeToC(fn.returnType)
                << " "
                << fn.name
                << "(";

            for (size_t i = 0; i < fn.params.size(); i++)
            {
                const auto& p = fn.params[i];
                out << mapTypeToC(p.type);

                if (i + 1 < fn.params.size())
                    out << ", ";
            }

            out << ");\n";
        }
    }

    return out.str();
}

std::string CodegenC::mapTypeToC(const Type& type)
{
    std::string base;

    if (type.base == "int")      base = "int";
    else if (type.base == "nore") base = "void";
    else if (type.base == "bool") base = "bool";
    else if (type.base == "char") base = "char";

    else if (type.base == "i8")   base = "int8_t";
    else if (type.base == "i16")  base = "int16_t";
    else if (type.base == "i32")  base = "int32_t";
    else if (type.base == "i64")  base = "int64_t";

    else if (type.base == "u8")   base = "uint8_t";
    else if (type.base == "u16")  base = "uint16_t";
    else if (type.base == "u32")  base = "uint32_t";
    else if (type.base == "u64")  base = "uint64_t";

    else
        throw std::runtime_error("Unknown type: " + type.base);

    if (type.isPointer)
        base += "*";

    return base;
}




// Entry Point

std::string CodegenC::generate(const Program& program)
{
    std::stringstream out;

    out << "#include <stdint.h>\n";
    out << "#include <stdbool.h>\n";


    for (const auto& decl : program.decls)
    {
        if (std::holds_alternative<UseDecl>(decl))
        {
            const auto& use = std::get<UseDecl>(decl);

            std::string headerName = use.path;
            headerName.replace(headerName.find(".az"), 3, ".h");

            out << "#include \"" << headerName << "\"\n";
        }
    }

    for (const auto& decl : program.decls)
    {
        if (std::holds_alternative<FunctionDecl>(decl))
        {
            const auto& fn = std::get<FunctionDecl>(decl);
            out << generateFunction(fn);
        }
    }


    return out.str();
}


// Function Generation

std::string CodegenC::generateFunction(const FunctionDecl& fn)
{
    std::stringstream out;

    if (fn.isExtern)
    {
        std::stringstream out;

        out << "extern "
            << mapTypeToC(fn.returnType)
            << " "
            << fn.name
            << "(";

        for (size_t i = 0; i < fn.params.size(); ++i)
        {
            const auto& p = fn.params[i];
            out << mapTypeToC(p.type) << " " << p.name;

            if (i + 1 < fn.params.size())
                out << ", ";
        }

        out << ");\n";

        return out.str();
    }


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
        if (!ret->value)
            return "return;\n";

        return "return " +
            generateExpression(ret->value.get()) +
            ";\n";
    }


    if (auto var = dynamic_cast<const VarDeclStmt*>(stmt))
    {
        if (var->isArray)
        {
            return mapTypeToC(var->type) + " " +
                var->name + "[" +
                std::to_string(var->arraySize) +
                "];\n";
        }

        return mapTypeToC(var->type) + " " +
            var->name + " = " +
            generateExpression(var->initializer.get()) +
            ";\n";
    }

    if (auto assign = dynamic_cast<const AssignmentStmt*>(stmt))
    {
        return generateExpression(assign->target.get()) +
            " = " +
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
    if (auto exprStmt = dynamic_cast<const ExpressionStmt*>(stmt))
    {
        return generateExpression(exprStmt->expression.get()) + ";\n";
    }

    if (auto wh = dynamic_cast<const WhileStmt*>(stmt))
    {
        std::stringstream out;

        out << "while ("
            << generateExpression(wh->condition.get())
            << ") {\n";

        increaseIndent();

        for (const auto& s : wh->body->statements)
            out << indent() << generateStatement(s.get());

        decreaseIndent();

        out << indent() << "}\n";

        return out.str();
    }



    throw std::runtime_error("Unsupported statement in C codegen");
}


// Expression Generation

std::string CodegenC::generateExpression(const Expr* expr)
{
    if (auto lit = dynamic_cast<const LiteralExpr*>(expr))
        return lit->value;



    if (auto unary = dynamic_cast<const UnaryExpr*>(expr))
    {
        return "(" + unary->op +
            generateExpression(unary->operand.get()) +
            ")";
    }

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

        if (!call->moduleName.empty())
        {
            out << call->moduleName << "__";
        }

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

    if (auto str = dynamic_cast<const StringExpr*>(expr))
    {
        return "\"" + str->value + "\"";
    }

    if (auto idx = dynamic_cast<const IndexExpr*>(expr))
    {
        return generateExpression(idx->base.get()) +
            "[" +
            generateExpression(idx->index.get()) +
            "]";
    }




    throw std::runtime_error("Unsupported expression in C codegen");
}

}
