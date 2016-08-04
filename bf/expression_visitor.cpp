#include "expression_visitor.h"
#include "instruction_visitor.h"
#include "scope_exit.h"

#include <algorithm>

namespace bf {

expression_visitor::expression_visitor(compiler::build_t &build, const generator::var_ptr &target_variable) : m_build(build) {
    m_var_stack.push_back(target_variable);
}
    
// ----- Binary or -------------------------------------------------------------
void expression_visitor::operator()(const expression::binary_operation_t<expression::operator_t::or_> &e) {
    if (const expression::value_t *v = boost::get<expression::value_t>(&e.rhs)) {
        if (v->value)
            m_var_stack.back()->set(1);
        else
            boost::apply_visitor(*this, e.lhs);
    } else {
        boost::apply_visitor(*this, e.lhs);
        auto rhs_ptr = m_build.bfg.new_var("_binary_or_rhs");
        m_var_stack.push_back(rhs_ptr);
        boost::apply_visitor(*this, e.rhs);
        m_var_stack.pop_back();
        m_var_stack.back()->bool_or(*rhs_ptr);
    }
}

// ----- Binary and ------------------------------------------------------------
void expression_visitor::operator()(const expression::binary_operation_t<expression::operator_t::and_> &e) {
    if (const expression::value_t *v = boost::get<expression::value_t>(&e.rhs)) {
        if (v->value)
            boost::apply_visitor(*this, e.lhs);
        else
            m_var_stack.back()->set(0);
    } else {
        boost::apply_visitor(*this, e.lhs);
        auto rhs_ptr = m_build.bfg.new_var("_binary_and_rhs");
        m_var_stack.push_back(rhs_ptr);
        boost::apply_visitor(*this, e.rhs);
        m_var_stack.pop_back();
        m_var_stack.back()->bool_and(*rhs_ptr);
    }
}

// ----- Binary equal ----------------------------------------------------------
void expression_visitor::operator()(const expression::binary_operation_t<expression::operator_t::eq> &e) {
    boost::apply_visitor(*this, e.lhs);
    auto rhs_ptr = m_build.bfg.new_var("_binary_eq_rhs");
    m_var_stack.push_back(rhs_ptr);
    boost::apply_visitor(*this, e.rhs);
    m_var_stack.pop_back();
    m_var_stack.back()->equal(*rhs_ptr);
}

// ----- Binary not equal ------------------------------------------------------
void expression_visitor::operator()(const expression::binary_operation_t<expression::operator_t::neq> &e) {
    boost::apply_visitor(*this, e.lhs);
    auto rhs_ptr = m_build.bfg.new_var("_binary_neq_rhs");
    m_var_stack.push_back(rhs_ptr);
    boost::apply_visitor(*this, e.rhs);
    m_var_stack.pop_back();
    m_var_stack.back()->not_equal(*rhs_ptr);
}

// ----- Binary lower than -----------------------------------------------------
void expression_visitor::operator()(const expression::binary_operation_t<expression::operator_t::lt> &e) {
    boost::apply_visitor(*this, e.lhs);
    auto rhs_ptr = m_build.bfg.new_var("_binary_lt_rhs");
    m_var_stack.push_back(rhs_ptr);
    boost::apply_visitor(*this, e.rhs);
    m_var_stack.pop_back();
    m_var_stack.back()->lower_than(*rhs_ptr);
}

// ----- Binary lower equal ----------------------------------------------------
void expression_visitor::operator()(const expression::binary_operation_t<expression::operator_t::leq> &e) {
    boost::apply_visitor(*this, e.lhs);
    auto rhs_ptr = m_build.bfg.new_var("_binary_leq_rhs");
    m_var_stack.push_back(rhs_ptr);
    boost::apply_visitor(*this, e.rhs);
    m_var_stack.pop_back();
    m_var_stack.back()->lower_equal(*rhs_ptr);
}

// ----- Binary greater than ---------------------------------------------------
void expression_visitor::operator()(const expression::binary_operation_t<expression::operator_t::gt> &e) {
    boost::apply_visitor(*this, e.lhs);
    auto rhs_ptr = m_build.bfg.new_var("_binary_gt_rhs");
    m_var_stack.push_back(rhs_ptr);
    boost::apply_visitor(*this, e.rhs);
    m_var_stack.pop_back();
    m_var_stack.back()->greater_than(*rhs_ptr);
}

// ----- Binary greater equal --------------------------------------------------
void expression_visitor::operator()(const expression::binary_operation_t<expression::operator_t::geq> &e) {
    boost::apply_visitor(*this, e.lhs);
    auto rhs_ptr = m_build.bfg.new_var("_binary_geq_rhs");
    m_var_stack.push_back(rhs_ptr);
    boost::apply_visitor(*this, e.rhs);
    m_var_stack.pop_back();
    m_var_stack.back()->greater_equal(*rhs_ptr);
}

// ----- Binary add ------------------------------------------------------------
void expression_visitor::operator()(const expression::binary_operation_t<expression::operator_t::add> &e) {
    boost::apply_visitor(*this, e.lhs);
    if (const expression::value_t *v = boost::get<expression::value_t>(&e.rhs)) {
        m_var_stack.back()->add(v->value);
    } else {
        auto rhs_ptr = m_build.bfg.new_var("_binary_add_rhs");
        m_var_stack.push_back(rhs_ptr);
        boost::apply_visitor(*this, e.rhs);
        m_var_stack.pop_back();
        m_var_stack.back()->add(*rhs_ptr);
    }
}

// ----- Binary subtract -------------------------------------------------------
void expression_visitor::operator()(const expression::binary_operation_t<expression::operator_t::sub> &e) {
    boost::apply_visitor(*this, e.lhs);
    if (const expression::value_t *v = boost::get<expression::value_t>(&e.rhs)) {
        m_var_stack.back()->subtract(v->value);
    } else {
        auto rhs_ptr = m_build.bfg.new_var("_binary_sub_rhs");
        m_var_stack.push_back(rhs_ptr);
        boost::apply_visitor(*this, e.rhs);
        m_var_stack.pop_back();
        m_var_stack.back()->subtract(*rhs_ptr);
    }
}

// ----- Binary multiply -------------------------------------------------------
void expression_visitor::operator()(const expression::binary_operation_t<expression::operator_t::mul> &e) {
    boost::apply_visitor(*this, e.lhs);
    if (const expression::value_t *v = boost::get<expression::value_t>(&e.rhs)) {
        m_var_stack.back()->multiply(v->value);
    } else {
        auto rhs_ptr = m_build.bfg.new_var("_binary_mul_rhs");
        m_var_stack.push_back(rhs_ptr);
        boost::apply_visitor(*this, e.rhs);
        m_var_stack.pop_back();
        m_var_stack.back()->multiply(*rhs_ptr);
    }
}

// ----- Unary not -------------------------------------------------------------
void expression_visitor::operator()(const expression::unary_operation_t<expression::operator_t::not_> &e) {
    boost::apply_visitor(*this, e.expression);
    m_var_stack.back()->bool_not(*m_var_stack.back());
}

// ----- Value -----------------------------------------------------------------
void expression_visitor::operator()(const expression::value_t &e) {
    m_var_stack.back()->set(e.value);
}

// ----- Function call ---------------------------------------------------------
void expression_visitor::operator()(const expression::function_call_t &e) {
    // Check if called function exists.
    auto function_it = std::find_if(m_build.program.begin(), m_build.program.end(),
            [&e](const function_t &f) {return f.name == e.function_name;});
    if (function_it == m_build.program.end())
        throw std::logic_error("Function not found: " + e.function_name);

    // Check if arguments are okay.
    if (function_it->parameters.size() != e.arguments.size())
        throw std::logic_error("Wrong number of arguments!"
            "Expected: " + std::to_string(function_it->parameters.size())
            + ", provided: " + std::to_string(e.arguments.size()));
    auto params_copy = function_it->parameters;
    std::sort(params_copy.begin(), params_copy.end());
    for (std::size_t i = 1; i < params_copy.size(); ++i) {
        if (params_copy[i - 1] == params_copy[i])
            throw std::logic_error("Duplicate parameter name: " + params_copy[i]);
    }

    // Check for recursion (which is not supported).
    auto recursion_it = std::find(m_build.call_stack.begin(), m_build.call_stack.end(), e.function_name);
    if (recursion_it != m_build.call_stack.end()) {
        std::string call_stack_dump;
        for (const auto &function : m_build.call_stack)
            call_stack_dump += function + ", ";
        call_stack_dump += "(*) " + e.function_name;
        throw std::logic_error("Recursion not supported: " + call_stack_dump);
    }

    // Update call stack.
    m_build.call_stack.push_back(e.function_name);
    SCOPE_EXIT {m_build.call_stack.pop_back();};

    // Provide a clean scope for the called function.
    compiler::scope_tree_t scope;
    scope.emplace_back();

    // Evaluate function arguments.
    for (std::size_t i = 0; i < e.arguments.size(); ++i) {
        const std::string param = function_it->parameters[i];
        // If expression is a simple constant, pass it as init value.
        if (const expression::value_t *v = boost::get<expression::value_t>(&e.arguments[i]))
            scope.back().emplace(param, m_build.bfg.new_var(param, v->value));
        else {
            auto var_ptr = m_build.bfg.new_var(param);
            scope.back().emplace(param, var_ptr);
            expression_visitor visitor(m_build, var_ptr);
            boost::apply_visitor(visitor, e.arguments[i]);
        }
    }

    // Enable new scope, including parameters.
    std::swap(scope, m_build.scope);
    // Restore old scope after the function returns.
    SCOPE_EXIT {std::swap(scope, m_build.scope);};

    // Finally, visit all instructions in the called function.
    instruction_visitor visitor(m_build, m_var_stack.back());
    for (const auto &instruction : function_it->instructions)
        boost::apply_visitor(visitor, instruction);
}

// ----- Variable --------------------------------------------------------------
void expression_visitor::operator()(const expression::variable_t &e) {
    m_var_stack.back()->copy(*m_build.get_var(e.variable_name));
}

// ----- Parenthesized expression ----------------------------------------------
void expression_visitor::operator()(const expression::parenthesized_expression_t &e) {
    boost::apply_visitor(*this, e.expression);
}

} // namespace bf
