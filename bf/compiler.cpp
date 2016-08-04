#ifdef _WIN32
// Suppress warnings...
#pragma warning(disable : 4348) // from boost spirit
#pragma warning(disable : 4180) // from boost proto
#endif

#include "compiler.h"
#include "generator.h"
#include "instruction_grammar.h"
#include "instruction_visitor.h"

#include <algorithm>
#include <ostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace bf {

// ----- Parse source to AST ---------------------------------------------------
program_t parse(const std::string &source) {
    instruction_g<std::string::const_iterator> grammar;
    skipper_g<std::string::const_iterator>     skipper;
    program_t                                  program;

    // Build program struct from source input.
    auto it = source.begin(), end = source.end();
    const bool success = qi::phrase_parse(it, end, grammar, skipper, program);

    if (!success || it != end)
        throw std::logic_error("Parse unsuccessful!");

    // ----- Check program struct ----------------------------------------------
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

// ----- Generate Brainfuck code from AST --------------------------------------
compiler::compiler() : m_debug_output(false) {}

std::string compiler::compile(const std::string &source) const {
    program_t program = parse(source);
    return generate(program);
}

void compiler::enable_debug_output(bool debug_output) {
    m_debug_output = debug_output;
}

std::string compiler::generate(const program_t &program) const {
    build_t build(program);
    auto return_value = build.bfg.new_var("_return_value");
    // As long as all function calls are inlined, this makes sense.
    instruction_visitor visitor(build, return_value);
    visitor(instruction::function_call_t{"main"});

    if (m_debug_output)
        return build.bfg.get_code();
    else
        return build.bfg.get_minimal_code();
}

// ----- Helper function -------------------------------------------------------
const generator::var_ptr &build_t::get_var(const std::string &variable_name) const {
    for (auto scope_it = scope.rbegin(); scope_it != scope.rend(); ++scope_it) {
        auto it = scope_it->find(variable_name);
        if (it != scope_it->end())
            return it->second;
    }

    throw std::logic_error("Variable not declared in this scope: " + variable_name);
}

} // namespace bf
