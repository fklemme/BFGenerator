#pragma once

#include "ast_types.h"
#include "error_handler.h"
#include "skipper_grammar.h"

#define BOOST_RESULT_OF_USE_DECLTYPE
#define BOOST_SPIRIT_USE_PHOENIX_V3

#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/repository/include/qi_distinct.hpp>
#include <type_traits>

namespace bf {

namespace qi    = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;

template <typename iterator>
struct expression_g : qi::grammar<iterator, expression::expression_t(), skipper_g<iterator>> {
    // TODO: Descripe necessity of AST rotation here!

    // Helper: Get operator precedence of arbitrary binary operation.
    class precedence_visitor : public boost::static_visitor<int> {
    public:
        // Return value represents C operator precedence.
        // See: http://en.cppreference.com/w/c/language/operator_precedence
        int operator()(const expression::binary_operation_t<expression::operator_t::or_>&)  const {return 12;}
        int operator()(const expression::binary_operation_t<expression::operator_t::and_>&) const {return 11;}
        int operator()(const expression::binary_operation_t<expression::operator_t::eq>&)   const {return 7;}
        int operator()(const expression::binary_operation_t<expression::operator_t::neq>&)  const {return 7;}
        int operator()(const expression::binary_operation_t<expression::operator_t::lt>&)   const {return 6;}
        int operator()(const expression::binary_operation_t<expression::operator_t::leq>&)  const {return 6;}
        int operator()(const expression::binary_operation_t<expression::operator_t::gt>&)   const {return 6;}
        int operator()(const expression::binary_operation_t<expression::operator_t::geq>&)  const {return 6;}
        int operator()(const expression::binary_operation_t<expression::operator_t::add>&)  const {return 4;}
        int operator()(const expression::binary_operation_t<expression::operator_t::sub>&)  const {return 4;}
        int operator()(const expression::binary_operation_t<expression::operator_t::mul>&)  const {return 3;}

        template <typename other_expression_t>
        int operator()(const other_expression_t&) const {
            return 0; // Placeholder. No binary operation.
        }
    };

    // Helper: Get left/right hand side of arbitrary binary operation.
    enum class side_t {left, right};
    class child_visitor : public boost::static_visitor<expression::expression_t&> {
    public:
        child_visitor(side_t side) : m_side(side) {}

        template <typename binary_operation_t,
            typename std::enable_if<std::is_base_of<expression::binary_operation_base, binary_operation_t>::value>::type* = nullptr>
        expression::expression_t &operator()(binary_operation_t &attr) const {
            if (m_side == side_t::left)
                return attr.lhs;
            else
                return attr.rhs;
        }

        template <typename other_expression_t,
            typename std::enable_if<!std::is_base_of<expression::binary_operation_base, other_expression_t>::value>::type* = nullptr>
        expression::expression_t &operator()(other_expression_t &attr) const {
            assert(false); // child_visitor is never used for non-binary operations!
            return *(expression::expression_t*) nullptr;
        }

    private:
        side_t m_side;
    };

    // Rotate/Rearrange nodes in AST so that operators are applied in correct order later on.
    static expression::expression_t rotate(expression::expression_t attr, expression::expression_t rhs) {
        const child_visitor left (side_t::left);
        const child_visitor right(side_t::right);

        // Rotate nodes #1
        boost::apply_visitor(right, attr) = boost::apply_visitor(left, rhs); // attr.rhs = rhs.lhs;

        // If the new right child of 'attr' is still a binary operation with the same precedence, keep rotating. (recursive)
        const auto &new_rhs = boost::apply_visitor(right, attr);
        const int attr_precedence = boost::apply_visitor(precedence_visitor(), attr);
        const int nrhs_precedence = boost::apply_visitor(precedence_visitor(), new_rhs);
        if (attr_precedence == nrhs_precedence)
            attr = rotate(attr, new_rhs);

        // Rotate nodes #2
        boost::apply_visitor(left, rhs) = attr; // rhs.lhs = attr;

        return rhs;
    }

    // On binary operation: Check if rotation of nodes is necessary and assign result.
    using expression_rule_t = qi::rule<iterator, expression::expression_t(), skipper_g<iterator>>;
    static void check_rotate(expression::expression_t attr, typename expression_rule_t::context_type &context) {
        const auto &attr_rhs = boost::apply_visitor(child_visitor(side_t::right), attr);
        const int attr_precedence = boost::apply_visitor(precedence_visitor(), attr);
        const int rhs_precedence  = boost::apply_visitor(precedence_visitor(), attr_rhs);

        if (attr_precedence == rhs_precedence)
            boost::fusion::at_c<0>(context.attributes) = rotate(attr, attr_rhs);
        else // No rotation, pass through.
            boost::fusion::at_c<0>(context.attributes) = attr;
    };

    // ----- Parser grammar ----------------------------------------------------
    expression_g() : expression_g::base_type(expression) {
        #define KEYWORD boost::spirit::repository::distinct(qi::alnum | '_')
        #define KEYWORDS KEYWORD[qi::lit("function") | "var" | "print" | "scan" | "if" | "else" | "while" | "for"]
        function_name = qi::lexeme[((qi::alpha | '_') >> *(qi::alnum | '_')) - KEYWORDS];
        variable_name = qi::lexeme[((qi::alpha | '_') >> *(qi::alnum | '_')) - KEYWORDS];

        // Expressions named after C operator precedence.
        // See: http://en.cppreference.com/w/c/language/operator_precedence
        expression = expression_12.alias();

        // 12: Logical OR
        expression_12 = binary_or     [check_rotate]       // Rotate if necessary
                      | expression_11 [qi::_val = qi::_1]; // Pass through
        binary_or     = expression_11 >> qi::lexeme["||"] > expression_12;

        // 11: Logical AND
        expression_11 = binary_and   [check_rotate]       // Rotate if necessary
                      | expression_7 [qi::_val = qi::_1]; // Pass through
        binary_and    = expression_7 >> qi::lexeme["&&"] > expression_11;

        // 7: For relational == and != respectively
        expression_7 = binary_eq    [check_rotate]       // Rotate if necessary
                     | binary_neq   [check_rotate]       // Rotate if necessary
                     | expression_6 [qi::_val = qi::_1]; // Pass through
        binary_eq    = expression_6 >> qi::lexeme["=="] > expression_7;
        binary_neq   = expression_6 >> qi::lexeme["!="] > expression_7;

        // 6: For relational operators < and <= respectively
        //    For relational operators > and >= respectively
        expression_6 = binary_leq[check_rotate] | binary_lt[check_rotate] // check "equal" version first
                     | binary_geq[check_rotate] | binary_gt[check_rotate]
                     | expression_4[qi::_val = qi::_1]; // Pass through
        binary_lt    = expression_4 >>            '<'   > expression_6;
        binary_leq   = expression_4 >> qi::lexeme["<="] > expression_6;
        binary_gt    = expression_4 >>            '>'   > expression_6;
        binary_geq   = expression_4 >> qi::lexeme[">="] > expression_6;

        // 4: Addition and subtraction
        expression_4  = binary_add   [check_rotate]       // Rotate if necessary
                      | binary_sub   [check_rotate]       // Rotate if necessary
                      | expression_3 [qi::_val = qi::_1]; // Pass through
        binary_add    = expression_3 >> '+' > expression_4;
        binary_sub    = expression_3 >> '-' > expression_4;

        // 3: Multiplication
        expression_3 = binary_mul   [check_rotate]       // Rotate if necessary
                     | expression_2 [qi::_val = qi::_1]; // Pass through
        binary_mul   = expression_2 >> '*' > expression_3;

        // 2: Logical NOT
        expression_2 = unary_not | simple;
        unary_not    = '!' > simple;

        // Lowest expression level
        simple             = value | function_call_expr | variable | parenthesized;
        value              = qi::uint_ | (qi::lit('\'') > qi::char_ > '\'');
        function_call_expr = function_name >> '(' > -(expression % ',') > ')';
        variable           = variable_name;
        parenthesized      = '(' > expression > ')';

        function_name.name("function name");               // debug(function_name);
        variable_name.name("variable name");               // debug(variable_name);

        expression.name("expression");                     // debug(expression);
        expression_12.name("12: Logical OR");              // debug(expression_12);
        binary_or.name("binary or");                       // debug(binary_or);
        expression_11.name("11: Logical AND");             // debug(expression_11);
        binary_and.name("binary and");                     // debug(binary_and);
        expression_7.name("7: Relational eq/neq");         // debug(expression_7);
        binary_eq.name("binary eq");                       // debug(binary_eq);
        binary_neq.name("binary neq");                     // debug(binary_neq);
        expression_6.name("6: Relational lt/leq/gt/geq");  // debug(expression_6);
        binary_lt.name("binary lt");                       // debug(binary_lt);
        binary_leq.name("binary leq");                     // debug(binary_leq);
        binary_gt.name("binary gt");                       // debug(binary_gt);
        binary_geq.name("binary geq");                     // debug(binary_geq);
        expression_4.name("4: Addition and subtraction");  // debug(expression_4);
        binary_add.name("binary add");                     // debug(binary_add);
        binary_sub.name("binary sub");                     // debug(binary_sub);
        expression_3.name("3: Multiplication");            // debug(expression_3);
        binary_mul.name("binary mul");                     // debug(binary_mul);
        expression_2.name("2: Logical NOT");               // debug(expression_2);
        unary_not.name("unary not");                       // debug(unary_not);
        simple.name("simple");                             // debug(simple);
        value.name("value");                               // debug(value);
        function_call_expr.name("function call expr");     // debug(function_call_expr);
        variable.name("variable");                         // debug(variable);
        parenthesized.name("parenthesized expression");    // debug(parenthesized);

        // Print error message on parse failure.
        qi::on_error<qi::fail>(expression, boost::phoenix::bind(on_error, qi::_1, qi::_2, qi::_3, qi::_4));
    }

    error_handler<iterator> on_error;

    qi::rule<iterator, std::string(), skipper_g<iterator>> function_name;
    qi::rule<iterator, std::string(), skipper_g<iterator>> variable_name;

    qi::rule<iterator, expression::expression_t(),                                     skipper_g<iterator>> expression;
    qi::rule<iterator, expression::expression_t(),                                     skipper_g<iterator>> expression_12;
    qi::rule<iterator, expression::binary_operation_t<expression::operator_t::or_>(),  skipper_g<iterator>> binary_or;
    qi::rule<iterator, expression::expression_t(),                                     skipper_g<iterator>> expression_11;
    qi::rule<iterator, expression::binary_operation_t<expression::operator_t::and_>(), skipper_g<iterator>> binary_and;
    qi::rule<iterator, expression::expression_t(),                                     skipper_g<iterator>> expression_7;
    qi::rule<iterator, expression::binary_operation_t<expression::operator_t::eq>(),   skipper_g<iterator>> binary_eq;
    qi::rule<iterator, expression::binary_operation_t<expression::operator_t::neq>(),  skipper_g<iterator>> binary_neq;
    qi::rule<iterator, expression::expression_t(),                                     skipper_g<iterator>> expression_6;
    qi::rule<iterator, expression::binary_operation_t<expression::operator_t::lt>(),   skipper_g<iterator>> binary_lt;
    qi::rule<iterator, expression::binary_operation_t<expression::operator_t::leq>(),  skipper_g<iterator>> binary_leq;
    qi::rule<iterator, expression::binary_operation_t<expression::operator_t::gt>(),   skipper_g<iterator>> binary_gt;
    qi::rule<iterator, expression::binary_operation_t<expression::operator_t::geq>(),  skipper_g<iterator>> binary_geq;
    qi::rule<iterator, expression::expression_t(),                                     skipper_g<iterator>> expression_4;
    qi::rule<iterator, expression::binary_operation_t<expression::operator_t::add>(),  skipper_g<iterator>> binary_add;
    qi::rule<iterator, expression::binary_operation_t<expression::operator_t::sub>(),  skipper_g<iterator>> binary_sub;
    qi::rule<iterator, expression::expression_t(),                                     skipper_g<iterator>> expression_3;
    qi::rule<iterator, expression::binary_operation_t<expression::operator_t::mul>(),  skipper_g<iterator>> binary_mul;
    qi::rule<iterator, expression::expression_t(),                                     skipper_g<iterator>> expression_2;
    qi::rule<iterator, expression::unary_operation_t<expression::operator_t::not_>(),  skipper_g<iterator>> unary_not;
    qi::rule<iterator, expression::expression_t(),                                     skipper_g<iterator>> simple;
    qi::rule<iterator, expression::value_t(),                                          skipper_g<iterator>> value;
    qi::rule<iterator, expression::function_call_t(),                                  skipper_g<iterator>> function_call_expr;
    qi::rule<iterator, expression::variable_t(),                                       skipper_g<iterator>> variable;
    qi::rule<iterator, expression::parenthesized_expression_t(),                       skipper_g<iterator>> parenthesized;
};

} // namespace bf
