#pragma once

#include "ast.h"
#include "compiler.h"

#include <boost/variant.hpp>

namespace bf {

class instruction_visitor : public boost::static_visitor<void> {
public:
    instruction_visitor(build_t&);
    void operator()(const instruction::function_call_t&);
    void operator()(const instruction::variable_declaration_t&);
    void operator()(const instruction::variable_assignment_t&);
    void operator()(const instruction::print_variable_t&);
    void operator()(const instruction::print_text_t&);
    void operator()(const instruction::scan_variable_t&);
    void operator()(const instruction::if_else_t&);
    void operator()(const instruction::while_loop_t&);
    void operator()(const instruction::for_loop_t&);
    void operator()(const instruction::instruction_block_t&);

private:
    build_t &m_build;
};

} // namespace bf
