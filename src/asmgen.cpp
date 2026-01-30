#include "asmgen.hpp"
#include <iostream>
#include <unordered_map>
#include <string>
#include <fstream>

static std::ofstream out;

static void gen_stmt(Stmt *s, bool is_global);
static void gen_expr(Expr *e);

static std::vector<std::string> body;

struct AsmVar
{
    int offset;
    VarType type;
};

static int align16(int n)
{
    return (n + 15) & ~15;
}

static std::unordered_map<std::string, AsmVar> locals;
static std::unordered_map<std::string, std::pair<std::string, VarType>> globals;

static int stack_offset = 0;

static int string_id = 0;
static std::vector<std::string> data_strings;

static std::string emit_string(const std::string &s)
{
    std::string label = "str_" + std::to_string(string_id++);
    data_strings.push_back(label + " db \"" + s + "\", 10, 0");
    return label;
}

static void emit(const std::string &s)
{
    out << s << "\n";
}

static void emit_body(const std::string &s)
{
    body.push_back(s);
}

static int evaluate_const(Expr* e) 
{
    if (e->type == EXPR_INT) return e->int_value;
    if (e->type == EXPR_BINOP) {
        int left = evaluate_const(e->left);
        int right = evaluate_const(e->right);
        switch (e->op) 
        {
            case OP_ADD: return left + right;
            case OP_SUB: return left - right;
            case OP_MUL: return left * right;
            case OP_DIV: return left / right;
        }
    }
    throw std::runtime_error("non-constant expression in global initializer");
}

void gen_program(Function* fn, const std::string& out_name)
{
    
    data_strings.clear();
    globals.clear();
    string_id = 0;

    out.open(out_name);
    if (!out)
        throw std::runtime_error("failed to open out.asm");


    body.clear();
    locals.clear();
    stack_offset = 0;

    for (Stmt* s : fn->globals)
    {
        if (s->type == STMT_DECL)
        {
            std::string label = "glob_" + s->var_name;
            globals[s->var_name] = {label, s->var_type};
        }
    }
    // process globals into .data
    for (Stmt* s : fn->globals)
        gen_stmt(s, true);

    // process function body
    for (Stmt* s : fn->body)
    {
        if (s->type != STMT_RETURN)
            gen_stmt(s, false);
    }

    // return value
    if (!fn->body.empty() && fn->body.back()->type == STMT_RETURN)
    {
        if (fn->body.back()->value)
            gen_expr(fn->body.back()->value);
        else
            emit_body("    xor eax, eax");
    }
    else
    {
        emit_body("    xor eax, eax");
    }

    int aligned = align16(stack_offset);

    emit("global main");
    emit("extern printf");
    emit("");

    emit("section .data");
    emit("fmt_int db \"%ld\", 10, 0");
    emit("fmt_str db \"%s\", 0");
    emit("fmt_char db \"%c\", 10, 0");

    for (auto &s : data_strings)
        emit(s);

    for (auto &g : globals)
        emit(g.second.first);

    emit("");
    emit("section .text");
    emit("main:");
    emit("    push rbp");
    emit("    mov rbp, rsp");

    if (aligned > 0)
        emit("    sub rsp, " + std::to_string(aligned));

    for (auto &line : body)
        out << line << "\n";

    emit("    mov rsp, rbp");
    emit("    pop rbp");
    emit("    ret");
    emit("section .note.GNU-stack noalloc noexec nowrite progbits");

    out.close();
}

static void gen_expr(Expr *e)
{
    if (e->type == EXPR_INT)
    {
        emit_body("    mov rax, " + std::to_string(e->int_value));
        return;
    }
    if (e->type == EXPR_CHAR)
    {
        emit_body("    mov rax, " + std::to_string((int)e->char_value));
        return;
    }

    if (e->type == EXPR_VAR)
    {
        if (locals.count(e->var_name))
        {
            auto it = locals[e->var_name];
            emit_body("    mov rax, QWORD [rbp-" + std::to_string(it.offset) + "]");
            return;
        }
        if (globals.count(e->var_name))
        {
            emit_body("    mov rax, [rel glob_" + e->var_name + "]");
            return;
        }
        throw std::runtime_error("undeclared variable: " + e->var_name);
    }

    if (e->type == EXPR_STRING)
    {
        std::string label = emit_string(e->string_value);
        emit_body("    lea rax, [rel " + label + "]");
        return;
    }

    if (e->type == EXPR_BOOL)
    {
        emit_body("    mov rax, " + std::to_string(e->bool_value ? 1 : 0));
        return;
    }

    if (e->type == EXPR_CALL && e->func_name == "out" && e->module_name == "io")
    {
        gen_expr(e->arg);

        emit_body("    mov rsi, rax");

        if (e->arg->type == EXPR_STRING || (e->arg->type == EXPR_VAR && globals.count(e->arg->var_name) && globals[e->arg->var_name].second == TYPE_CHAR_PTR))
            emit_body("    lea rdi, [rel fmt_str]");
        else
            emit_body("    lea rdi, [rel fmt_int]");

        emit_body("    xor eax, eax");
        emit_body("    call printf");
        return;
    }

    if (e->type == EXPR_BINOP)
    {
        gen_expr(e->left);
        emit_body("    push rax");

        gen_expr(e->right);
        emit_body("    mov rbx, rax");

        emit_body("    pop rax");

        switch (e->op)
        {
            case OP_ADD: emit_body("    add rax, rbx"); break;
            case OP_SUB: emit_body("    sub rax, rbx"); break;
            case OP_MUL: emit_body("    imul rax, rbx"); break;
            case OP_DIV:
                emit_body("    cqo");
                emit_body("    idiv rbx");
                break;
        }
        return;
    }

    throw std::runtime_error("unsupported expression");
}

static void gen_stmt(Stmt *s, bool is_global)
{
    if (s->type == STMT_USE)
        return;

    if (s->type == STMT_DECL)
    {
        if (is_global)
        {
            std::string label = "glob_" + s->var_name;
            globals[s->var_name] = {label + ": dq 0", s->var_type};

            if (s->value)
            {
                if (s->var_type == TYPE_INT)
                {
                    int val = evaluate_const(s->value);
                    globals[s->var_name] = {label + " dq " + std::to_string(val), s->var_type};
                }
                else if (s->var_type == TYPE_CHAR_PTR)
                    globals[s->var_name] = {label + " dq " + emit_string(s->value->string_value), s->var_type};
            }
            return;
        }

        stack_offset += 8;
        locals[s->var_name] = {stack_offset, s->var_type};

        gen_expr(s->value);

        if (s->var_type == TYPE_BOOL)
        {
            emit_body("    cmp rax, 0");
            emit_body("    setne al");
            emit_body("    movzx rax, al");
        }

        emit_body("    mov QWORD [rbp-" + std::to_string(stack_offset) + "], rax");
        return;
    }

    if (s->type == STMT_ASSIGN)
    {
        if (locals.count(s->var_name))
        {
            gen_expr(s->value);
            emit_body("    mov QWORD [rbp-" + std::to_string(locals[s->var_name].offset) + "], rax");
            return;
        }
        if (globals.count(s->var_name))
        {
            gen_expr(s->value);
            emit_body("    mov [rel glob_" + s->var_name + "], rax");
            return;
        }
        throw std::runtime_error("assignment to undeclared variable");
    }

    if (s->type == STMT_EXPR)
    {
        gen_expr(s->value);
        return;
    }
}