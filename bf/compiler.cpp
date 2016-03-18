#include "compiler.h"
#include "generator.h"

#include <algorithm>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/variant.hpp>
#include <ostream>
#include <stdexcept>
#include <string>
#include <vector>

#include <iostream> // debug outputs

// ----- Program structs -------------------------------------------------------
namespace bf {
namespace instruction {

struct function_call_t {
    std::string function_name;
};

struct print_variable_t {
    std::string variable_name;
};

struct print_text_t {
    std::string text;
};

} // namespace bf::instruction

typedef boost::variant<
    instruction::function_call_t,
    instruction::print_variable_t,
    instruction::print_text_t
> instruction_t;

struct function_t {
    std::string name;
    std::vector<std::string> parameters;
    std::vector<instruction_t> instructions;
};

typedef std::vector<function_t> program_t;

} // namespace bf

BOOST_FUSION_ADAPT_STRUCT(
        bf::instruction::function_call_t,
        (std::string, function_name))

BOOST_FUSION_ADAPT_STRUCT(
        bf::instruction::print_variable_t,
        (std::string, variable_name))

BOOST_FUSION_ADAPT_STRUCT(
        bf::instruction::print_text_t,
        (std::string, text))

BOOST_FUSION_ADAPT_STRUCT(
        bf::function_t,
        (std::string, name)
        (std::vector<std::string>, parameters)
        (std::vector<bf::instruction_t>, instructions))

// ----- Parser grammar --------------------------------------------------------
namespace bf {

namespace qi    = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;

template <typename iterator>
struct grammar : qi::grammar<iterator, program_t(), ascii::space_type> {
    grammar() : grammar::base_type(program) {
        program = *function;
        function = "function" >> function_name
            >> '(' >> -(variable_name % ',') >> ')'
            >> '{' >> *instruction >> '}';
        function_name = qi::lexeme[+qi::alpha];
        variable_name = qi::lexeme[+qi::alpha];

        instruction = function_call
            | print_variable
            | print_text;

        function_call  = function_name >> '(' >> -(variable_name % ',') >> ')' >> ';';
        print_variable = "print" >> variable_name >> ";";
        print_text     = "print" >> qi::lexeme['"' >> *(qi::char_ - '"') >> '"'] >> ';';
    }

    qi::rule<iterator, program_t(),                     ascii::space_type> program;
    qi::rule<iterator, function_t(),                    ascii::space_type> function;
    qi::rule<iterator, std::string(),                   ascii::space_type> function_name;
    qi::rule<iterator, std::string(),                   ascii::space_type> variable_name;
    qi::rule<iterator, instruction_t(),                 ascii::space_type> instruction;

    qi::rule<iterator, instruction::function_call_t(),  ascii::space_type> function_call;
    qi::rule<iterator, instruction::print_variable_t(), ascii::space_type> print_variable;
    qi::rule<iterator, instruction::print_text_t(),     ascii::space_type> print_text;
};

// ----- Compilation algorithms ------------------------------------------------
program_t parse(const std::string &source) {
    // Build program struct from source input.
    grammar<decltype(source.begin())> g;
    program_t program;
    auto begin = source.begin(), end = source.end();
    const bool success = qi::phrase_parse(begin, end, g, ascii::space, program);

    if (!success || begin != end)
        throw std::logic_error("Parse unsuccessful!");

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

    return program;
}

class instruction_visitor : public boost::static_visitor<void> {
public:
    instruction_visitor(const program_t &program) : m_program(program) {}

    void operator()(const instruction::function_call_t &i) {
        // Check if called function exists.
        auto function_it = std::find_if(m_program.begin(), m_program.end(),
                [&i](const function_t &f) {return f.name == i.function_name;});
        if (function_it == m_program.end())
            throw std::logic_error("Function not found: " + i.function_name);

        // Visit all instructions in called function.
        for (const auto &instruction : function_it->instructions)
            boost::apply_visitor(*this, instruction);
    }

    void operator()(const instruction::print_variable_t &i) {
        // TODO
    }

    void operator()(const instruction::print_text_t &i) {
        m_bfg.print(i.text);
    }

    const generator &get_generator() const {
        return m_bfg;
    }

private:
    program_t m_program; // TODO: just const ref?
    generator m_bfg;
};

std::string generate(const program_t &program) {
    // As long as all function calls are inlined, this makes sense.
    instruction_visitor visitor(program);
    instruction::function_call_t call_to_main {"main"};
    instruction_t start = call_to_main;
    boost::apply_visitor(visitor, start);

    return visitor.get_generator().get_code();
}

std::string compiler::compile(const std::string &source) const {
    program_t program = parse(source);
    return generate(program);
}

} // namespace
