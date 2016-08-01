#pragma once

#include "ast.h"
#include "compiler.h"
#include "generator.h"

#include <boost/variant.hpp>

namespace bf {

class expression_visitor : public boost::static_visitor<void> {
public:
    expression_visitor(build_t&, const generator::var_ptr& target_variable);
    
    void operator()(const expression::binary_operation_t<expression::operator_t::or_>&);
    void operator()(const expression::binary_operation_t<expression::operator_t::and_>&);
    void operator()(const expression::binary_operation_t<expression::operator_t::eq>&);
    void operator()(const expression::binary_operation_t<expression::operator_t::neq>&);
    void operator()(const expression::binary_operation_t<expression::operator_t::lt>&);
    void operator()(const expression::binary_operation_t<expression::operator_t::leq>&);
    void operator()(const expression::binary_operation_t<expression::operator_t::gt>&);
    void operator()(const expression::binary_operation_t<expression::operator_t::geq>&);
    void operator()(const expression::binary_operation_t<expression::operator_t::add>&);
    void operator()(const expression::binary_operation_t<expression::operator_t::sub>&);
    void operator()(const expression::binary_operation_t<expression::operator_t::mul>&);
    void operator()(const expression::unary_operation_t<expression::operator_t::not_>&);
    void operator()(const expression::value_t&);
    void operator()(const expression::function_call_t&);
    void operator()(const expression::variable_t&);
    void operator()(const expression::parenthesized_expression_t&);

private:
    build_t                         &m_build;
    std::vector<generator::var_ptr> m_var_stack;
};

} // namespace bf
