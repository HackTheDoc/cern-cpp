#include "buildin.h"

#include <sstream>

#include "generation.h"

namespace {
    std::string print_call(const std::optional<Node::ArgList*>& args)
    {
        std::stringstream ss;

        ss << "std::cout << ";

        if (args.has_value()) 
        {
            ss << gen::expr(args.value()->expr);

            auto narg = args.value()->next_arg;
            while (narg.has_value())
            {
                ss << " << ";
                ss << gen::expr(narg.value()->expr);

                narg = narg.value()->next_arg;
            }
        }
        
        ss << ";\n";

        return ss.str();
    }

    std::string println_call(const std::optional<Node::ArgList*>& args)
    {
        std::stringstream ss;

        ss << "std::cout";

        if (args.has_value()) 
        {
            ss << " << " << gen::expr(args.value()->expr);

            auto narg = args.value()->next_arg;
            while (narg.has_value())
            {
                ss << " << " << gen::expr(narg.value()->expr);

                narg = narg.value()->next_arg;
            }
        }
        
        ss << " << std::endl;\n";

        return ss.str();
    }

    std::string itoc_call(const std::optional<Node::ArgList*>& args)
    {
        return "(char)" + gen::expr(args.value()->expr) + "+ '0'";
    }

    std::string ctoi_call(const std::optional<Node::ArgList*>& args)
    {
        return gen::expr(args.value()->expr) + " - '0'";
    }
}

std::optional<std::string> call_func(std::string func, const std::optional<Node::ArgList*>& args)
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
