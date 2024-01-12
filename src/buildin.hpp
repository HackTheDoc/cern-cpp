#include <map>
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
}

std::optional<std::string> call_func(std::string func, const std::optional<Node::ArgList*>& args = {})
{
    if (func == "print")
        return print_call(args);
    else if (func == "println")
        return println_call(args);
    
    return {};
}
