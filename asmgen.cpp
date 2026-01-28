#include "asmgen.hpp"
#include <iostream>
#include <unordered_map>
#include <string>
#include <fstream>

static std::ofstream out;

static void gen_stmt(Stmt* s);
static void gen_expr(Expr* e);

static std::vector<std::string> body;

struct AsmVar
{
    int offset; // rbp-relative
};

static int align16(int n)
{
    return (n + 15) & ~15;
}

static std::unordered_map<std::string, AsmVar> vars;
static int stack_offset = 0;

static int string_id = 0;
static std::vector<std::string> data_strings;

static std::string emit_string(const std::string& s)
{
    std::string label = "str_" + std::to_string(string_id++);
    data_strings.push_back(
        label + " db \"" + s + "\", 10, 0"
    );
    return label;
}

static void emit(const std::string& s)
{
    out << s << "\n";
}

static void emit_body(const std::string& s)
{
    body.push_back(s);
}

void gen_program(const std::vector<Stmt*>& program, const std::string& out_name)
{
    data_strings.clear();
    string_id = 0;

    out.open(out_name);
    if (!out)
        throw std::runtime_error("failed to open out.asm");

    body.clear();
    vars.clear();
    stack_offset = 0;

    for (Stmt* s : program)
    {
        gen_stmt(s);
    }

    int aligned = align16(stack_offset);

    // header
    emit("global main");
    emit("extern printf");
    emit("");
    emit("section .data");
    emit("fmt_int db \"%ld\", 10, 0");
    emit("fmt_str db \"%s\", 0");

    for (auto& s : data_strings)
        emit(s);
    emit("");
    emit("section .text");
    emit("main:");
    emit("    push rbp");
    emit("    mov rbp, rsp");

    if (aligned > 0)
        emit("    sub rsp, " + std::to_string(aligned));

    for (auto& line : body)
        out << line << "\n";

    emit("    mov rsp, rbp");
    emit("    pop rbp");
    emit("    xor eax, eax");
    emit("    ret");
    emit("section .note.GNU-stack noalloc noexec nowrite progbits");

    out.close();
}


static void gen_expr(Expr* e)
{
    if (e->type == EXPR_INT)
    {
        emit_body("    mov rax, " + std::to_string(e->int_value));
        return;
    }

    if (e->type == EXPR_VAR)
    {
        auto it = vars.find(e->var_name);
        if (it == vars.end())
            throw std::runtime_error("undeclared variable: " + e->var_name);

        emit_body("    mov rax, QWORD [rbp-" +
                  std::to_string(it->second.offset) + "]");
        return;
    }
    if (e->type == EXPR_STRING)
    {
        std::string label = emit_string(e->string_value);
        emit_body("    lea rax, [rel " + label + "]");
        return;
    }


    if (e->type == EXPR_CALL)
    {
        if (e->func_name == "out" && e->module_name == "io")
        {
            gen_expr(e->arg);

            if (e->arg->type == EXPR_INT || e->arg->type == EXPR_VAR)
            {
                emit_body("    mov rsi, rax");
                emit_body("    lea rdi, [rel fmt_int]");
            }
            else if (e->arg->type == EXPR_STRING)
            {
                emit_body("    mov rsi, rax");
                emit_body("    lea rdi, [rel fmt_str]");
            }
            else
            {
                throw std::runtime_error("unsupported print argument");
            }

            emit_body("    xor eax, eax");
            emit_body("    call printf");
            return;
        }
    }

    throw std::runtime_error("unsupported expression");
}


static void gen_stmt(Stmt* s)
{
    if (s->type == STMT_DECL)
    {
        stack_offset += 8;
        vars[s->var_name] = { stack_offset };

        gen_expr(s->value);
        emit_body("    mov QWORD [rbp-" +
                  std::to_string(stack_offset) + "], rax");
        return;
    }

    if (s->type == STMT_ASSIGN)
    {
        auto it = vars.find(s->var_name);
        if (it == vars.end())
            throw std::runtime_error("assignment to undeclared variable");

        gen_expr(s->value);
        emit_body("    mov QWORD [rbp-" +
                  std::to_string(it->second.offset) + "], rax");
        return;
    }

    if (s->type == STMT_EXPR)
    {
        gen_expr(s->value);
        return;
    }
}

