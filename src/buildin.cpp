#include "buildin.h"

#include <sstream>

#include "generation.h"

namespace {
    void exit_with(const std::string& err_msg)
    {
        std::cerr << "[Error] " << err_msg << std::endl;
        exit(EXIT_FAILURE);
    }

    std::string print_call(const std::vector<Node::Expr*>& args)
    {
        std::stringstream ss;

        ss << "std::cout";

        for (size_t i = 0; i < args.size(); i++)
        {
            ss << " << " << gen::expr(args[i]);
        }
        
        ss << ";\n";

        return ss.str();
    }

    std::string println_call(const std::vector<Node::Expr*>& args)
    {
        std::stringstream ss;

        ss << "std::cout";

        for (size_t i = 0; i < args.size(); i++)
        {
            ss << " << " << gen::expr(args[i]);
        }
        
        ss << " << std::endl;\n";

        return ss.str();
    }

    std::string itoc_call(const std::vector<Node::Expr*>& args)
    {
        if (args.empty())
            exit_with("function `itoc` require an argument");
        if (args[0]->type != VarType::INT)
            exit_with("itoc argument type must be int");
        if (args.size() > 1)
            exit_with("too many arguments in function call");

        return "(char)(" + gen::expr(args[0]) + "+ '0')";
    }

    std::string ctoi_call(const std::vector<Node::Expr*>& args)
    {
        if (args.empty())
            exit_with("function `ctoi` require an argument");
        if (args[0]->type != VarType::CHAR)
            exit_with("itoc argument type must be char");
        if (args.size() > 1)
            exit_with("too many arguments in function call");

        return "("+ gen::expr(args[0]) + " - '0')";
    }
}

std::optional<std::string> call_func(std::string func, const std::vector<Node::Expr*>& args)
{
    if (func == "print")
        return print_call(args);
    else if (func == "println")
        return println_call(args);
    else if (func == "itoc")
        return itoc_call(args);
    else if (func == "ctoi")
        return ctoi_call(args);
    
    return {};
}
