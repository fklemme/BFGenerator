#include "compiler.h"
#include "generator.h"
#include "scope_exit.h"

#define BOOST_RESULT_OF_USE_DECLTYPE
#define BOOST_SPIRIT_USE_PHOENIX_V3

#include <algorithm>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/scope_exit.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/variant.hpp>
#include <iostream> // for std::cerr in parser
#include <ostream>
#include <stdexcept>
#include <string>
#include <vector>

// ----- Program structs -------------------------------------------------------
namespace bf {

namespace instruction {

struct function_call_t {
    std::string function_name;
};

struct variable_declaration_t {
    std::string variable_name;
    int         init_value;
};

struct print_variable_t {
    std::string variable_name;
};

struct print_text_t {
    std::string text;
};

struct scan_variable_t {
    std::string variable_name;
};

} // namespace bf::instruction

typedef boost::variant<
    instruction::function_call_t,
    instruction::variable_declaration_t,
    instruction::print_variable_t,
    instruction::print_text_t,
    instruction::scan_variable_t
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
        bf::instruction::variable_declaration_t,
        (std::string, variable_name)
        (int,         init_value))

BOOST_FUSION_ADAPT_STRUCT(
        bf::instruction::print_variable_t,
        (std::string, variable_name))

BOOST_FUSION_ADAPT_STRUCT(
        bf::instruction::print_text_t,
        (std::string, text))

BOOST_FUSION_ADAPT_STRUCT(
        bf::instruction::scan_variable_t,
        (std::string, variable_name))

BOOST_FUSION_ADAPT_STRUCT(
        bf::function_t,
        (std::string, name)
        (std::vector<std::string>, parameters)
        (std::vector<bf::instruction_t>, instructions))

// ----- Parser grammar --------------------------------------------------------
namespace bf {

namespace qi    = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;

#define KEYWORDS (qi::lit("function") | "var" | "print" | "scan")
template <typename iterator>
struct grammar : qi::grammar<iterator, program_t(), ascii::space_type> {
    grammar() : grammar::base_type(program) {
        program = *function;
        function = qi::lexeme["function"] > function_name
            > '(' > -(variable_name % ',') > ')'
            > '{' > *instruction > '}';
        function_name = qi::lexeme[+qi::alpha - KEYWORDS];
        variable_name = qi::lexeme[+qi::alpha - KEYWORDS];

        instruction = function_call
            | variable_declaration
            | print_variable
            | print_text
            | scan_variable;

        function_call        = function_name >> '(' > -(variable_name % ',') > ')' > ';';
        variable_declaration = qi::lexeme["var"] > variable_name > (('=' > qi::uint_) | qi::attr(0)) > ';';
        print_variable       = qi::lexeme["print"] >> variable_name > ';';
        print_text           = qi::lexeme["print"] >> qi::lexeme['"' > *(qi::char_ - '"') > '"'] > ';';
        scan_variable        = qi::lexeme["scan"] > variable_name > ';';

        program.name("program");
        function.name("function");
        function_name.name("function name");
        variable_name.name("variable name");
        instruction.name("instruction");
        function_call.name("function call");
        variable_declaration.name("variable declaration");
        print_variable.name("print variable");
        print_text.name("print text");
        scan_variable.name("scan variable");

        auto on_error = [](auto first, auto last, auto err, auto what) {
            std::string before(first, err);
            std::size_t bpos = before.find_last_of('\n');
            if (bpos != std::string::npos)
                before = before.substr(bpos + 1);

            std::string after(err, last);
            std::size_t apos = after.find_first_of('\n');
            if (apos != std::string::npos)
                after = after.substr(0, apos);

            std::cerr << "Error! Expecting " << what << " here:\n"
                << before << after << '\n'
                << std::string(before.size(), ' ') << '^'
                << std::endl;
        };

        qi::on_error<qi::fail>(program, boost::phoenix::bind(on_error, qi::_1, qi::_2, qi::_3, qi::_4));
    }

    qi::rule<iterator, program_t(),                           ascii::space_type> program;
    qi::rule<iterator, function_t(),                          ascii::space_type> function;
    qi::rule<iterator, std::string(),                         ascii::space_type> function_name;
    qi::rule<iterator, std::string(),                         ascii::space_type> variable_name;
    qi::rule<iterator, instruction_t(),                       ascii::space_type> instruction;

    qi::rule<iterator, instruction::function_call_t(),        ascii::space_type> function_call;
    qi::rule<iterator, instruction::variable_declaration_t(), ascii::space_type> variable_declaration;
    qi::rule<iterator, instruction::print_variable_t(),       ascii::space_type> print_variable;
    qi::rule<iterator, instruction::print_text_t(),           ascii::space_type> print_text;
    qi::rule<iterator, instruction::scan_variable_t(),        ascii::space_type> scan_variable;
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

        // Check for recursion (which is not supported).
        auto recursion_it = std::find(m_call_stack.begin(), m_call_stack.end(), i.function_name);
        if (recursion_it != m_call_stack.end()) {
            std::string call_stack_dump;
            for (const auto &function : m_call_stack)
                call_stack_dump += function + ", ";
            call_stack_dump += "(*) " + i.function_name;
            throw std::logic_error("Recursion not supported: " + call_stack_dump);
        }

        // Provide a clean scope for the called function.
        std::vector<std::map<std::string, generator::var_ptr>> scope_backup;
        std::swap(m_scope, scope_backup);
        m_scope.emplace_back();
        // TODO: Copy arguments
        // Restore old scope after the function returns.
        SCOPE_EXIT {std::swap(m_scope, scope_backup);};

        // Just for error report on recursion: Keep track of the call stack.
        m_call_stack.push_back(i.function_name);
        SCOPE_EXIT {m_call_stack.pop_back();};

        // Finally, visit all instructions in the called function.
        for (const auto &instruction : function_it->instructions)
            boost::apply_visitor(*this, instruction);
    }

    void operator()(const instruction::variable_declaration_t &i) {
        auto it = m_scope.back().find(i.variable_name);
        if (it != m_scope.back().end())
            throw std::logic_error("Redeclaration of variable: " + i.variable_name);

        m_scope.back().emplace(i.variable_name, m_bfg.new_var(i.variable_name, i.init_value));
    }

    void operator()(const instruction::print_variable_t &i) {
        get_var(i.variable_name)->write_output();
    }

    void operator()(const instruction::print_text_t &i) {
        m_bfg.print(i.text);
    }

    void operator()(const instruction::scan_variable_t &i) {
        get_var(i.variable_name)->read_input();
    }

    const generator::var_ptr &get_var(const std::string &variable_name) const {
        for (auto scope_it = m_scope.rbegin(); scope_it != m_scope.rend(); ++scope_it) {
            auto it = scope_it->find(variable_name);
            if (it != scope_it->end())
                return it->second;
        }

        throw std::logic_error("Variable not declared in this scope: " + variable_name);
    }

    const generator &get_generator() const {
        return m_bfg;
    }

private:
    program_t                                              m_program; // TODO: just const ref?
    generator                                              m_bfg;
    std::vector<std::map<std::string, generator::var_ptr>> m_scope;
    std::vector<std::string>                               m_call_stack;
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
