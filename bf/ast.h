#pragma once

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/optional.hpp>
#include <boost/variant.hpp>
#include <string>
#include <vector>

namespace bf {

namespace expression {

    enum class operator_t { or_, and_, eq, neq, lt, leq, gt, geq, add, sub, mul, not_ };

    // Forward declarations
    template <operator_t op> struct binary_operation_t;
    template <operator_t op> struct unary_operation_t;
    struct value_t;
    struct function_call_t;
    struct variable_t;
    struct parenthesized_expression_t;

    typedef boost::variant<
        // First type must not contain member of type expression_t.
        boost::recursive_wrapper<value_t>,
        boost::recursive_wrapper<binary_operation_t<operator_t::or_>>,
        boost::recursive_wrapper<binary_operation_t<operator_t::and_>>,
        boost::recursive_wrapper<binary_operation_t<operator_t::eq>>,
        boost::recursive_wrapper<binary_operation_t<operator_t::neq>>,
        boost::recursive_wrapper<binary_operation_t<operator_t::lt>>,
        boost::recursive_wrapper<binary_operation_t<operator_t::leq>>,
        boost::recursive_wrapper<binary_operation_t<operator_t::gt>>,
        boost::recursive_wrapper<binary_operation_t<operator_t::geq>>,
        boost::recursive_wrapper<binary_operation_t<operator_t::add>>,
        boost::recursive_wrapper<binary_operation_t<operator_t::sub>>,
        boost::recursive_wrapper<binary_operation_t<operator_t::mul>>,
        boost::recursive_wrapper<unary_operation_t<operator_t::not_>>,
        boost::recursive_wrapper<function_call_t>,
        boost::recursive_wrapper<variable_t>,
        boost::recursive_wrapper<parenthesized_expression_t>
    > expression_t;
    
    struct binary_operation_base {}; // For type traits

    template <operator_t op>
    struct binary_operation_t : binary_operation_base {
        expression_t lhs;
        expression_t rhs;
    };

    template <operator_t op>
    struct unary_operation_t {
        expression_t expression;
    };

    struct value_t {
        unsigned value;
    };

    struct function_call_t {
        std::string              function_name;
        std::vector<std::string> variable_names;
    };

    struct variable_t {
        std::string variable_name;
    };

    struct parenthesized_expression_t {
        expression_t expression;
    };

} // namespace bf::expression

namespace instruction {

    struct function_call_t {
        std::string              function_name;
        std::vector<std::string> variable_names;
    };

    struct variable_declaration_t {
        std::string              variable_name;
        expression::expression_t expression;
    };

    struct variable_assignment_t {
        std::string              variable_name;
        expression::expression_t expression;
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

    struct if_else_t;
    struct while_loop_t;
    struct for_loop_t;
    struct instruction_block_t;

    typedef boost::variant<
        function_call_t,
        variable_declaration_t,
        variable_assignment_t,
        print_variable_t,
        print_text_t,
        scan_variable_t,
        boost::recursive_wrapper<if_else_t>,
        boost::recursive_wrapper<while_loop_t>,
        boost::recursive_wrapper<for_loop_t>,
        boost::recursive_wrapper<instruction_block_t>
    > instruction_t;

    struct if_else_t {
        expression::expression_t       condition;
        instruction_t                  if_instruction;
        boost::optional<instruction_t> else_instruction;
    };

    struct while_loop_t {
        expression::expression_t condition;
        instruction_t            instruction;
    };

    struct for_loop_t {
        boost::optional<instruction_t> initialization;
        expression::expression_t       condition;
        boost::optional<instruction_t> post_loop;
        instruction_t                  instruction;
    };

    struct instruction_block_t {
        std::vector<instruction_t> instructions;
    };

} // namespace bf::instruction

struct function_t {
    std::string                             name;
    std::vector<std::string>                parameters;
    std::vector<instruction::instruction_t> instructions;
};

typedef std::vector<function_t> program_t;

} // namespace bf

BOOST_FUSION_ADAPT_STRUCT(
        bf::expression::binary_operation_t<bf::expression::operator_t::or_>,
        (bf::expression::expression_t, lhs)
        (bf::expression::expression_t, rhs))

BOOST_FUSION_ADAPT_STRUCT(
        bf::expression::binary_operation_t<bf::expression::operator_t::and_>,
        (bf::expression::expression_t, lhs)
        (bf::expression::expression_t, rhs))

BOOST_FUSION_ADAPT_STRUCT(
        bf::expression::binary_operation_t<bf::expression::operator_t::eq>,
        (bf::expression::expression_t, lhs)
        (bf::expression::expression_t, rhs))

BOOST_FUSION_ADAPT_STRUCT(
        bf::expression::binary_operation_t<bf::expression::operator_t::neq>,
        (bf::expression::expression_t, lhs)
        (bf::expression::expression_t, rhs))

BOOST_FUSION_ADAPT_STRUCT(
        bf::expression::binary_operation_t<bf::expression::operator_t::lt>,
        (bf::expression::expression_t, lhs)
        (bf::expression::expression_t, rhs))

BOOST_FUSION_ADAPT_STRUCT(
        bf::expression::binary_operation_t<bf::expression::operator_t::leq>,
        (bf::expression::expression_t, lhs)
        (bf::expression::expression_t, rhs))

BOOST_FUSION_ADAPT_STRUCT(
        bf::expression::binary_operation_t<bf::expression::operator_t::gt>,
        (bf::expression::expression_t, lhs)
        (bf::expression::expression_t, rhs))

BOOST_FUSION_ADAPT_STRUCT(
        bf::expression::binary_operation_t<bf::expression::operator_t::geq>,
        (bf::expression::expression_t, lhs)
        (bf::expression::expression_t, rhs))

BOOST_FUSION_ADAPT_STRUCT(
        bf::expression::binary_operation_t<bf::expression::operator_t::add>,
        (bf::expression::expression_t, lhs)
        (bf::expression::expression_t, rhs))

BOOST_FUSION_ADAPT_STRUCT(
        bf::expression::binary_operation_t<bf::expression::operator_t::sub>,
        (bf::expression::expression_t, lhs)
        (bf::expression::expression_t, rhs))

BOOST_FUSION_ADAPT_STRUCT(
        bf::expression::binary_operation_t<bf::expression::operator_t::mul>,
        (bf::expression::expression_t, lhs)
        (bf::expression::expression_t, rhs))

BOOST_FUSION_ADAPT_STRUCT(
        bf::expression::unary_operation_t<bf::expression::operator_t::not_>,
        (bf::expression::expression_t, expression))

BOOST_FUSION_ADAPT_STRUCT(
        bf::expression::value_t,
        (unsigned, value))

BOOST_FUSION_ADAPT_STRUCT(
        bf::expression::function_call_t,
        (std::string,              function_name)
        (std::vector<std::string>, variable_names))

BOOST_FUSION_ADAPT_STRUCT(
        bf::expression::variable_t,
        (std::string, variable_name))

BOOST_FUSION_ADAPT_STRUCT(
        bf::expression::parenthesized_expression_t,
        (bf::expression::expression_t, expression))

BOOST_FUSION_ADAPT_STRUCT(
        bf::instruction::function_call_t,
        (std::string,              function_name)
        (std::vector<std::string>, variable_names))

BOOST_FUSION_ADAPT_STRUCT(
        bf::instruction::variable_declaration_t,
        (std::string,                  variable_name)
        (bf::expression::expression_t, expression))

BOOST_FUSION_ADAPT_STRUCT(
        bf::instruction::variable_assignment_t,
        (std::string,                  variable_name)
        (bf::expression::expression_t, expression))

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
        bf::instruction::if_else_t,
        (bf::expression::expression_t,                    condition)
        (bf::instruction::instruction_t,                  if_instruction)
        (boost::optional<bf::instruction::instruction_t>, else_instruction))

BOOST_FUSION_ADAPT_STRUCT(
        bf::instruction::while_loop_t,
        (bf::expression::expression_t,   condition)
        (bf::instruction::instruction_t, instruction))

BOOST_FUSION_ADAPT_STRUCT(
        bf::instruction::for_loop_t,
        (boost::optional<bf::instruction::instruction_t>, initialization)
        (bf::expression::expression_t,                    condition)
        (boost::optional<bf::instruction::instruction_t>, post_loop)
        (bf::instruction::instruction_t,                  instruction))

BOOST_FUSION_ADAPT_STRUCT(
        bf::instruction::instruction_block_t,
        (std::vector<bf::instruction::instruction_t>, instructions))

BOOST_FUSION_ADAPT_STRUCT(
        bf::function_t,
        (std::string,                                 name)
        (std::vector<std::string>,                    parameters)
        (std::vector<bf::instruction::instruction_t>, instructions))
