#pragma once

#include "parser.h"

std::optional<std::string> call_func(std::string func, const std::optional<Node::ArgList*>& args = {});
