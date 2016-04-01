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

// ----- Syntax tree structs ---------------------------------------------------
namespace bf {

namespace expression {

    enum class operator_t { or, and, eq, neq, lt, leq, gt, geq, add, sub, mul, not_ };

    // Forward declarations
    struct variable_t;
    struct value_t;
    template <operator_t op> struct unary_operation_t;
    template <operator_t op> struct binary_operation_t;
    struct parenthesized_expression_t;

    typedef boost::variant<
        boost::recursive_wrapper<variable_t>,
        boost::recursive_wrapper<value_t>,
        boost::recursive_wrapper<binary_operation_t<operator_t::or>>,
        boost::recursive_wrapper<binary_operation_t<operator_t::and>>,
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
        boost::recursive_wrapper<parenthesized_expression_t>
    > expression_t;

    struct variable_t {
        std::string variable_name;
    };

    struct value_t {
        unsigned value;
    };

    template <operator_t op>
    struct unary_operation_t {
        expression_t expression;
    };
    
    template <operator_t op>
    struct binary_operation_t {
        expression_t lhs;
        expression_t rhs;
    };

    struct parenthesized_expression_t {
        expression_t expression;
    };

} // namespace bf::expression

namespace instruction {

    struct function_call_t {
        std::string function_name;
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

    typedef boost::variant<
        function_call_t,
        variable_declaration_t,
        variable_assignment_t,
        print_variable_t,
        print_text_t,
        scan_variable_t
    > instruction_t;

} // namespace bf::instruction

struct function_t {
    std::string                             name;
    std::vector<std::string>                parameters;
    std::vector<instruction::instruction_t> instructions;
};

typedef std::vector<function_t> program_t;

} // namespace bf

BOOST_FUSION_ADAPT_STRUCT(
        bf::expression::variable_t,
        (std::string, variable_name))

BOOST_FUSION_ADAPT_STRUCT(
        bf::expression::value_t,
        (unsigned, value))

BOOST_FUSION_ADAPT_STRUCT(
        bf::expression::binary_operation_t<bf::expression::operator_t::or>,
        (bf::expression::expression_t, lhs)
        (bf::expression::expression_t, rhs))

BOOST_FUSION_ADAPT_STRUCT(
        bf::expression::binary_operation_t<bf::expression::operator_t::and>,
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
        bf::expression::parenthesized_expression_t,
        (bf::expression::expression_t, expression))

BOOST_FUSION_ADAPT_STRUCT(
        bf::instruction::function_call_t,
        (std::string, function_name))

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
        bf::function_t,
        (std::string,                                 name)
        (std::vector<std::string>,                    parameters)
        (std::vector<bf::instruction::instruction_t>, instructions))

// ----- Parser grammar --------------------------------------------------------
namespace bf {

namespace qi    = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;

template <typename iterator>
struct grammar : qi::grammar<iterator, program_t(), ascii::space_type> {
    // TODO: Descripe circumstances here!
    typedef expression::binary_operation_t<expression::operator_t::add> binary_op_add_t;
    typedef expression::binary_operation_t<expression::operator_t::sub> binary_op_sub_t;
    typedef boost::variant<binary_op_add_t, binary_op_sub_t>            binary_op_term_t;

    // Helper: Get left/right hand side of arbitrary binary operation.
    enum class side_t {left, right};
    class get_child : public boost::static_visitor<expression::expression_t&> {
    public:
        get_child(side_t side) : m_side(side) {}

        template <typename binary_op_t>
        expression::expression_t &operator()(binary_op_t &attr) {
            if (m_side == side_t::left)
                return attr.lhs;
            else
                return attr.rhs;
        }

    private:
        side_t m_side;
    };

    // Rotate/Rearrange nodes in AST so that subtraction operators are applied correctly later on.
    static binary_op_term_t rotate(binary_op_term_t attr, binary_op_term_t rhs) {
        get_child left (side_t::left);
        get_child right(side_t::right);

        // Rotate nodes #1
        boost::apply_visitor(right, attr) = boost::apply_visitor(left, rhs); // attr.rhs = rhs.lhs;

        // If the new right child of 'attr' is still a binary operation, keep rotating. (recursive)
        const auto &attr_rhs = boost::apply_visitor(right, attr);
        if (const binary_op_add_t *attr_rhs_ptr = boost::get<binary_op_add_t>(&attr_rhs))
            attr = rotate(attr, *attr_rhs_ptr);
        else if (const binary_op_sub_t *attr_rhs_ptr = boost::get<binary_op_sub_t>(&attr_rhs))
            attr = rotate(attr, *attr_rhs_ptr);
        // else impossible

        // Rotate nodes #2
        boost::apply_visitor(left, rhs) = attr; // rhs.lhs = attr;

        return rhs;
    }

    // On binary subtraction operator: Check if rotation of nodes is necessary.
    typedef qi::rule<iterator, expression::expression_t(), ascii::space_type> expression_rule_t;
    static void on_binary_sub(const binary_op_sub_t &attr, typename expression_rule_t::context_type &context) {
        if (const binary_op_add_t *rhs = boost::get<binary_op_add_t>(&attr.rhs))
            boost::fusion::at_c<0>(context.attributes) = rotate(attr, *rhs);
        else if (const binary_op_sub_t *rhs = boost::get<binary_op_sub_t>(&attr.rhs))
            boost::fusion::at_c<0>(context.attributes) = rotate(attr, *rhs);
        else // No rotation, pass through.
            boost::fusion::at_c<0>(context.attributes) = attr;
    };

    grammar() : grammar::base_type(program) {
        program  = *function;
        function = qi::lexeme["function"] > function_name
                 > '(' > -(variable_name % ',') > ')'
                 > '{' > *instruction > '}';
        #define KEYWORDS (qi::lit("function") | "var" | "print" | "scan")
        function_name = qi::lexeme[((qi::alpha | '_') >> *(qi::alnum | '_')) - KEYWORDS];
        variable_name = qi::lexeme[((qi::alpha | '_') >> *(qi::alnum | '_')) - KEYWORDS];

        // Expressions named after C operator precedence.
        // See: http://en.cppreference.com/w/c/language/operator_precedence
        expression = expression_12.alias();

        // 12: Logical OR
        expression_12 = binary_or | expression_11;
        binary_or     = expression_11 >> qi::lexeme["||"] > expression_12;

        // 11: Logical AND
        expression_11 = binary_and | expression_7;
        binary_and    = expression_7 >> qi::lexeme["&&"] > expression_11;

        // 7: For relational == and != respectively
        expression_7 = binary_eq | binary_neq | expression_6;
        binary_eq    = expression_6 >> qi::lexeme["=="] > expression_7;
        binary_neq   = expression_6 >> qi::lexeme["!="] > expression_7;

        // 6: For relational operators < and <= respectively
        //    For relational operators > and >= respectively
        expression_6 = binary_lt | binary_leq | binary_gt | binary_geq | expression_4;
        binary_lt    = expression_4 >>            '<'   > expression_6;
        binary_leq   = expression_4 >> qi::lexeme["<="] > expression_6;
        binary_gt    = expression_4 >>            '>'   > expression_6;
        binary_geq   = expression_4 >> qi::lexeme[">="] > expression_6;

        // 4: Addition and subtraction
        expression_4  = binary_add   [qi::_val = qi::_1]  // Pass through
                      | binary_sub   [on_binary_sub]      // Check if rotation is necessary
                      | expression_3 [qi::_val = qi::_1]; // Pass through
        binary_add    = expression_3 >> '+' > expression_4;
        binary_sub    = expression_3 >> '-' > expression_4;

        // 3: Multiplication
        expression_3 = binary_mul | expression_2;
        binary_mul   = expression_2 >> '*' > expression_3;

        // 2: Logical NOT
        expression_2 = unary_not | simple;
        unary_not    = '!' > simple;

        // Lowest expression level
        simple        = value | variable | parenthesized;
        value         = qi::uint_;
        variable      = variable_name;
        parenthesized = '(' > expression > ')';

        // Instructions
        instruction = function_call
                    | variable_declaration
                    | variable_assignment
                    | print_variable
                    | print_text
                    | scan_variable;

        function_call        = function_name >> '(' > -(variable_name % ',') > ')' > ';';
        variable_declaration = qi::lexeme["var"] > variable_name > (('=' > expression) | qi::attr(expression::value_t{0u})) > ';';
        variable_assignment  = variable_name >> '=' > expression > ';';
        print_variable       = qi::lexeme["print"] >> variable_name > ';';
        print_text           = qi::lexeme["print"] >> qi::lexeme['"' > *(qi::char_ - '"') > '"'] > ';';
        scan_variable        = qi::lexeme["scan"] > variable_name > ';';

        program.name("program");                           // debug(program);
        function.name("function");                         // debug(function);
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
        variable.name("variable");                         // debug(variable);
        parenthesized.name("parenthesized expression");    // debug(parenthesized);

        instruction.name("instruction");                   // debug(instruction);
        function_call.name("function call");               // debug(function_call);
        variable_declaration.name("variable declaration"); // debug(variable_declaration);
        variable_assignment.name("variable assignment");   // debug(variable_assignment);
        print_variable.name("print variable");             // debug(print_variable);
        print_text.name("print text");                     // debug(print_text);
        scan_variable.name("scan variable");               // debug(scan_variable);

		// Print error message on parse failure.
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

    qi::rule<iterator, program_t(),   ascii::space_type> program;
    qi::rule<iterator, function_t(),  ascii::space_type> function;
    qi::rule<iterator, std::string(), ascii::space_type> function_name;
    qi::rule<iterator, std::string(), ascii::space_type> variable_name;

    qi::rule<iterator, expression::expression_t(),                                    ascii::space_type> expression;
    qi::rule<iterator, expression::expression_t(),                                    ascii::space_type> expression_12;
    qi::rule<iterator, expression::binary_operation_t<expression::operator_t::or>(),  ascii::space_type> binary_or;
    qi::rule<iterator, expression::expression_t(),                                    ascii::space_type> expression_11;
    qi::rule<iterator, expression::binary_operation_t<expression::operator_t::and>(), ascii::space_type> binary_and;
    qi::rule<iterator, expression::expression_t(),                                    ascii::space_type> expression_7;
    qi::rule<iterator, expression::binary_operation_t<expression::operator_t::eq>(),  ascii::space_type> binary_eq;
    qi::rule<iterator, expression::binary_operation_t<expression::operator_t::neq>(), ascii::space_type> binary_neq;
    qi::rule<iterator, expression::expression_t(),                                    ascii::space_type> expression_6;
    qi::rule<iterator, expression::binary_operation_t<expression::operator_t::lt>(),  ascii::space_type> binary_lt;
    qi::rule<iterator, expression::binary_operation_t<expression::operator_t::leq>(), ascii::space_type> binary_leq;
    qi::rule<iterator, expression::binary_operation_t<expression::operator_t::gt>(),  ascii::space_type> binary_gt;
    qi::rule<iterator, expression::binary_operation_t<expression::operator_t::geq>(), ascii::space_type> binary_geq;
    qi::rule<iterator, expression::expression_t(),                                    ascii::space_type> expression_4;
    qi::rule<iterator, expression::binary_operation_t<expression::operator_t::add>(), ascii::space_type> binary_add;
    qi::rule<iterator, expression::binary_operation_t<expression::operator_t::sub>(), ascii::space_type> binary_sub;
    qi::rule<iterator, expression::expression_t(),                                    ascii::space_type> expression_3;
    qi::rule<iterator, expression::binary_operation_t<expression::operator_t::mul>(), ascii::space_type> binary_mul;
    qi::rule<iterator, expression::expression_t(),                                    ascii::space_type> expression_2;
    qi::rule<iterator, expression::unary_operation_t<expression::operator_t::not_>(), ascii::space_type> unary_not;
    qi::rule<iterator, expression::expression_t(),                                    ascii::space_type> simple;
    qi::rule<iterator, expression::value_t(),                                         ascii::space_type> value;
    qi::rule<iterator, expression::variable_t(),                                      ascii::space_type> variable;
    qi::rule<iterator, expression::parenthesized_expression_t(),                      ascii::space_type> parenthesized;

    qi::rule<iterator, instruction::instruction_t(),          ascii::space_type> instruction;
    qi::rule<iterator, instruction::function_call_t(),        ascii::space_type> function_call;
    qi::rule<iterator, instruction::variable_declaration_t(), ascii::space_type> variable_declaration;
    qi::rule<iterator, instruction::variable_assignment_t(),  ascii::space_type> variable_assignment;
    qi::rule<iterator, instruction::print_variable_t(),       ascii::space_type> print_variable;
    qi::rule<iterator, instruction::print_text_t(),           ascii::space_type> print_text;
    qi::rule<iterator, instruction::scan_variable_t(),        ascii::space_type> scan_variable;
};

// ----- Parse source to AST ---------------------------------------------------
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

typedef std::vector<std::map<std::string, generator::var_ptr>> scope_tree_t;

// ----- Evaluate AST expressions ----------------------------------------------
class expression_visitor : public boost::static_visitor<void> {
public:
    expression_visitor(generator &bfg, const scope_tree_t &scope, const generator::var_ptr &var_ptr)
        : m_bfg(bfg), m_scope(scope)
    {
        m_var_stack.push_back(var_ptr);
    }

    void operator()(const expression::variable_t &e) {
        m_var_stack.back()->copy(*get_var(e.variable_name));
    }

    void operator()(const expression::value_t &e) {
        m_var_stack.back()->set(e.value);
    }
    
    // TODO...
    void operator()(const expression::binary_operation_t<expression::operator_t::or> &e) {}
    void operator()(const expression::binary_operation_t<expression::operator_t::and> &e) {}
    void operator()(const expression::binary_operation_t<expression::operator_t::eq> &e) {}
    void operator()(const expression::binary_operation_t<expression::operator_t::neq> &e) {}
    void operator()(const expression::binary_operation_t<expression::operator_t::lt> &e) {}
    void operator()(const expression::binary_operation_t<expression::operator_t::leq> &e) {}
    void operator()(const expression::binary_operation_t<expression::operator_t::gt> &e) {}
    void operator()(const expression::binary_operation_t<expression::operator_t::geq> &e) {}

    void operator()(const expression::binary_operation_t<expression::operator_t::add> &e) {
        boost::apply_visitor(*this, e.lhs);
        if (const expression::value_t *v = boost::get<expression::value_t>(&e.rhs)) {
            m_var_stack.back()->add(v->value);
        } else {
            auto rhs_ptr = m_bfg.new_var("_binary_add_rhs");
            m_var_stack.push_back(rhs_ptr);
            boost::apply_visitor(*this, e.rhs);
            m_var_stack.pop_back();
            m_var_stack.back()->add(*rhs_ptr);
        }
    }

    void operator()(const expression::binary_operation_t<expression::operator_t::sub> &e) {
        boost::apply_visitor(*this, e.lhs);
        if (const expression::value_t *v = boost::get<expression::value_t>(&e.rhs)) {
            m_var_stack.back()->subtract(v->value);
        } else {
            auto rhs_ptr = m_bfg.new_var("_binary_sub_rhs");
            m_var_stack.push_back(rhs_ptr);
            boost::apply_visitor(*this, e.rhs);
            m_var_stack.pop_back();
            m_var_stack.back()->subtract(*rhs_ptr);
        }
    }

    void operator()(const expression::binary_operation_t<expression::operator_t::mul> &e) {
        boost::apply_visitor(*this, e.lhs);
        if (const expression::value_t *v = boost::get<expression::value_t>(&e.rhs)) {
            m_var_stack.back()->multiply(v->value);
        } else {
            auto rhs_ptr = m_bfg.new_var("_binary_mul_rhs");
            m_var_stack.push_back(rhs_ptr);
            boost::apply_visitor(*this, e.rhs);
            m_var_stack.pop_back();
            m_var_stack.back()->multiply(*rhs_ptr);
        }
    }

    void operator()(const expression::unary_operation_t<expression::operator_t::not_> &e) {
        boost::apply_visitor(*this, e.expression);
        m_var_stack.back()->bool_not(*m_var_stack.back());
    }

    void operator()(const expression::parenthesized_expression_t &e) {
        boost::apply_visitor(*this, e.expression);
    }

private:
    const generator::var_ptr &get_var(const std::string &variable_name) const {
        for (auto scope_it = m_scope.rbegin(); scope_it != m_scope.rend(); ++scope_it) {
            auto it = scope_it->find(variable_name);
            if (it != scope_it->end())
                return it->second;
        }

        throw std::logic_error("Variable not declared in this scope: " + variable_name);
    }

    generator                       &m_bfg;
    const scope_tree_t              &m_scope;
    std::vector<generator::var_ptr> m_var_stack;
};

// ----- Evaluate AST instructions ---------------------------------------------
class instruction_visitor : public boost::static_visitor<void> {
public:
    instruction_visitor(const program_t &program) : m_program(program) {}

    // ----- Function call -----------------------------------------------------
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
        // TODO: Copy/handle arguments
        // Restore old scope after the function returns.
        SCOPE_EXIT {std::swap(m_scope, scope_backup);};

        // Just for error report on recursion: Keep track of the call stack.
        m_call_stack.push_back(i.function_name);
        SCOPE_EXIT {m_call_stack.pop_back();};

        // Finally, visit all instructions in the called function.
        for (const auto &instruction : function_it->instructions)
            boost::apply_visitor(*this, instruction);
    }

    // ----- Variable declaration ----------------------------------------------
    void operator()(const instruction::variable_declaration_t &i) {
        auto it = m_scope.back().find(i.variable_name);
        if (it != m_scope.back().end())
            throw std::logic_error("Redeclaration of variable: " + i.variable_name);

        // If expression is a simple constant, pass it as init value.
        if (const expression::value_t *v = boost::get<expression::value_t>(&i.expression))
            m_scope.back().emplace(i.variable_name, m_bfg.new_var(i.variable_name, v->value));
        else {
            auto var_ptr = m_bfg.new_var(i.variable_name);
            m_scope.back().emplace(i.variable_name, var_ptr);
            auto visitor = expression_visitor(m_bfg, m_scope, var_ptr);
            boost::apply_visitor(visitor, i.expression);
        }
    }

    // ----- Variable assignment -----------------------------------------------
    void operator()(const instruction::variable_assignment_t &i) {
        auto visitor = expression_visitor(m_bfg, m_scope, get_var(i.variable_name));
        boost::apply_visitor(visitor, i.expression);
    }

    // ----- Print variable ----------------------------------------------------
    void operator()(const instruction::print_variable_t &i) {
        get_var(i.variable_name)->write_output();
    }

    // ----- Print text --------------------------------------------------------
    void operator()(const instruction::print_text_t &i) {
        m_bfg.print(i.text);
    }

    // ----- Scan variable -----------------------------------------------------
    void operator()(const instruction::scan_variable_t &i) {
        get_var(i.variable_name)->read_input();
    }

    const generator &get_generator() const {
        return m_bfg;
    }

private:
    const generator::var_ptr &get_var(const std::string &variable_name) const {
        for (auto scope_it = m_scope.rbegin(); scope_it != m_scope.rend(); ++scope_it) {
            auto it = scope_it->find(variable_name);
            if (it != scope_it->end())
                return it->second;
        }

        throw std::logic_error("Variable not declared in this scope: " + variable_name);
    }

    program_t                m_program; // TODO: just const ref?
    generator                m_bfg;
    scope_tree_t             m_scope;
    std::vector<std::string> m_call_stack;
};

// ----- Generate Brainfuck code from AST --------------------------------------
std::string generate(const program_t &program) {
    // As long as all function calls are inlined, this makes sense.
    instruction_visitor visitor(program);
    instruction::function_call_t call_to_main {"main"};
    instruction::instruction_t start = call_to_main;
    boost::apply_visitor(visitor, start);

    return visitor.get_generator().get_code();
}

std::string compiler::compile(const std::string &source) const {
    program_t program = parse(source);
    return generate(program);
}

} // namespace
