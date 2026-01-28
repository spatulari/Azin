#pragma once
#include <vector>
#include "ast.hpp"

void gen_program(const std::vector<Stmt*>& program, const std::string& out_name);
