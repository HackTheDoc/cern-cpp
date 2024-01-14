#pragma once

#include "parser.h"

std::optional<std::string> call_func(std::string func, const std::vector<Node::Expr*>& args = {});
