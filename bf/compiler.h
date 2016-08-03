#pragma once

#include "ast_types.h"
#include "generator.h"

#include <map>
#include <string>
#include <vector>

namespace bf {

class compiler {
public:
    compiler();

    // Compile source to Brainfuck code.
    std::string compile(const std::string &source) const;

    void enable_debug_output(bool);

private:
    std::string generate(const program_t &program) const;

    bool m_debug_output;
};

struct build_t {
    typedef std::vector<std::map<std::string, generator::var_ptr>> scope_tree_t;

    build_t(const program_t &p) : program(p) {}
    build_t(const build_t&) = delete; // do not copy by accident

    const generator::var_ptr &get_var(const std::string &variable_name) const;

    const program_t          &program;
    generator                bfg;
    scope_tree_t             scope;
    std::vector<std::string> call_stack;
};

} // namespace bf
