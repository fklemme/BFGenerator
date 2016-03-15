#include "compiler.h"

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/spirit/include/qi.hpp>
#include <ostream>
#include <stdexcept>
#include <string>
#include <vector>

#include <iostream> // debug outputs

// ----- Program structs -------------------------------------------------------
namespace bf {

struct function_t {
    std::string name;
    std::vector<std::string> parameters;
    std::vector<std::string> instructions;
};

typedef std::vector<function_t> program_t;

} // namespace

BOOST_FUSION_ADAPT_STRUCT(
        bf::function_t,
        (std::string, name)
        (std::vector<std::string>, parameters)
        (std::vector<std::string>, instructions))

// ----- Parser grammar --------------------------------------------------------
namespace bf {

namespace qi    = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;

template <typename iterator>
struct grammar : qi::grammar<iterator, program_t(), ascii::space_type> {
    grammar() : grammar::base_type(program) {
        program = *function;
        function = "function" >> function_name
            >> "(" >> -(parameter_name % ',') >> ")"
            >> "{" >> *instruction >> "}";
        function_name = qi::lexeme[+qi::alpha];
        parameter_name = qi::lexeme[+qi::alpha];
        instruction = qi::lexeme[*(qi::char_ - ";")] >> ";"; // TODO!
    }

    qi::rule<iterator, program_t(),   ascii::space_type> program;
    qi::rule<iterator, function_t(),  ascii::space_type> function;
    qi::rule<iterator, std::string(), ascii::space_type> function_name;
    qi::rule<iterator, std::string(), ascii::space_type> parameter_name;
    qi::rule<iterator, std::string(), ascii::space_type> instruction;
};

// Debug program_t output
std::ostream &operator<<(std::ostream &o, const program_t &program) {
    for (auto &function : program) {
        if (function.name != program.front().name)
            o << "\n";
        o << "Function name: " << function.name << "\n";
        o << "Parameters:";
        for (auto &parameter : function.parameters)
            o << " " << parameter;
        o << "\nInstructions:";
        for (auto &instruction : function.instructions)
            o << "\n\t" << instruction;
    }

    return o;
}

// ----- Compilation algorithm -------------------------------------------------
std::string compiler::compile(const std::string &source) {
    // Build program struct from source input.
    grammar<decltype(source.begin())> g;
    program_t program;
    const bool success = qi::phrase_parse(source.begin(), source.end(), g, ascii::space, program);

    // ----- Check program struct -----
    std::sort(program.begin(), program.end(),
            [](const function_t &f1, const function_t &f2) {return f1.name < f2.name;});
    // Ensure unique function names.
    if (program.size() > 1) {
        auto before_it = program.begin();
        for (auto it = before_it + 1; it != program.end(); ++before_it, ++it)
            if (before_it->name == it->name)
                throw std::logic_error("Function name used multiply times: " + it->name);
    }
    // TODO...

    // Debug output. TODO: omit
    std::cout << "Parse result: " << (success ? "true" : "false") << std::endl;
    std::cout << program << std::endl;

    return "TODO";
}

} // namespace
