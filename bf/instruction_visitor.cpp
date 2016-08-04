#include "expression_visitor.h"
#include "instruction_visitor.h"
#include "scope_exit.h"

namespace bf {

instruction_visitor::instruction_visitor(compiler::build_t &build, const generator::var_ptr &return_value)
    : m_build(build), m_return_value(return_value) {}

// ----- Function call ---------------------------------------------------------
void instruction_visitor::operator()(const instruction::function_call_t &i) {
    // TODO: Describe this kind-of wrapper.
    auto return_value = m_build.bfg.new_var("_return_value", 0);
    expression_visitor visitor(m_build, return_value);
    expression::function_call_t function_call{i.function_name, i.arguments};
    visitor(function_call);
}

// ----- Variable declaration --------------------------------------------------
void instruction_visitor::operator()(const instruction::variable_declaration_t &i) {
    auto it = m_build.scope.back().find(i.variable_name);
    if (it != m_build.scope.back().end())
        throw std::logic_error("Redeclaration of variable: " + i.variable_name);

    // If expression is a simple constant, pass it as init value.
    if (const expression::value_t *v = boost::get<expression::value_t>(&i.expression))
        m_build.scope.back().emplace(i.variable_name, m_build.bfg.new_var(i.variable_name, v->value));
    else {
        auto var_ptr = m_build.bfg.new_var(i.variable_name);
        m_build.scope.back().emplace(i.variable_name, var_ptr);
        expression_visitor visitor(m_build, var_ptr);
        boost::apply_visitor(visitor, i.expression);
    }
}

// ----- Variable assignment ---------------------------------------------------
void instruction_visitor::operator()(const instruction::variable_assignment_t &i) {
    expression_visitor visitor(m_build, m_build.get_var(i.variable_name));
    boost::apply_visitor(visitor, i.expression);
}

// ----- Print expression ------------------------------------------------------
void instruction_visitor::operator()(const instruction::print_expression_t &i) {
    auto expression = m_build.bfg.new_var("_expression");
    expression_visitor visitor(m_build, expression);
    boost::apply_visitor(visitor, i.expression);
    expression->write_output();
}

// ----- Print text ------------------------------------------------------------
void instruction_visitor::operator()(const instruction::print_text_t &i) {
    std::string text = i.text;
    // Replace character sequence "\n" with character '\n' and so on...
    // TODO: This can be enhanced for sure.
    auto it = text.begin();
    while ((it = std::find(it, text.end(), '\\')) != text.end()) {
        text.erase(it);
        if (it != text.end())
            switch (*it) {
                case 'n': *it = '\n'; break;
                case 't': *it = '\t'; break;
                default: throw std::logic_error(std::string("Unknown char: '\\") + *it + "'");
            }
    }
    m_build.bfg.print(text);
}

// ----- Scan variable ---------------------------------------------------------
void instruction_visitor::operator()(const instruction::scan_variable_t &i) {
    m_build.get_var(i.variable_name)->read_input();
}

// ----- Return statement ------------------------------------------------------
void instruction_visitor::operator()(const instruction::return_statement_t &i) {
    expression_visitor visitor(m_build, m_return_value);
    boost::apply_visitor(visitor, i.expression);
}

// ----- Conditional statement -------------------------------------------------
void instruction_visitor::operator()(const instruction::if_else_t &i) {
    auto condition = m_build.bfg.new_var("_if_condition");
    expression_visitor visitor(m_build, condition);
    boost::apply_visitor(visitor, i.condition);
        
    m_build.bfg.if_begin(*condition);
    {
        // Provide a new scope for "then" part.
        m_build.scope.emplace_back();
        SCOPE_EXIT {m_build.scope.pop_back();};
        boost::apply_visitor(*this, i.if_instruction);
    }
    if (i.else_instruction) {
        m_build.bfg.else_begin();
        // Provide a new scope for "else" part.
        m_build.scope.emplace_back();
        SCOPE_EXIT {m_build.scope.pop_back();};
        boost::apply_visitor(*this, *i.else_instruction);
    }
    m_build.bfg.if_end();
}

// ----- While loop ------------------------------------------------------------
void instruction_visitor::operator()(const instruction::while_loop_t &i) {
    auto condition = m_build.bfg.new_var("_while_condition");
    expression_visitor visitor(m_build, condition);
    boost::apply_visitor(visitor, i.condition);

    m_build.bfg.while_begin(*condition);
    {
        // Provide a new scope for loop body.
        m_build.scope.emplace_back();
        SCOPE_EXIT {m_build.scope.pop_back();};
        boost::apply_visitor(*this, i.instruction);
    }
    // Re-evaluate condition.
    boost::apply_visitor(visitor, i.condition);
    m_build.bfg.while_end(*condition);
}

// ----- For loop --------------------------------------------------------------
void instruction_visitor::operator()(const instruction::for_loop_t &i) {
    // Provide a new scope for loop header.
    m_build.scope.emplace_back();
    SCOPE_EXIT {m_build.scope.pop_back();};

    if (i.initialization)
        boost::apply_visitor(*this, *i.initialization);

    auto condition = m_build.bfg.new_var("_for_condition");
    expression_visitor visitor(m_build, condition);
    boost::apply_visitor(visitor, i.condition);

    m_build.bfg.while_begin(*condition);
    {
        // Provide a new scope for loop body.
        m_build.scope.emplace_back();
        SCOPE_EXIT {m_build.scope.pop_back();};
        boost::apply_visitor(*this, i.instruction);
    }
    // Post-loop instruction.
    if (i.post_loop)
        boost::apply_visitor(*this, *i.post_loop);

    // Re-evaluate condition.
    boost::apply_visitor(visitor, i.condition);
    m_build.bfg.while_end(*condition);
}

// ----- Instruction block -----------------------------------------------------
void instruction_visitor::operator()(const instruction::instruction_block_t &i) {
    // Provide a new scope for variable names.
    m_build.scope.emplace_back();
    SCOPE_EXIT {m_build.scope.pop_back();};

    for (const auto &instruction : i.instructions)
        boost::apply_visitor(*this, instruction);
}

} // namespace bf
