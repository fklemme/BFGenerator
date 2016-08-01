#ifdef _WIN32
// Suppress warnings...
#pragma warning(disable : 4348) // from boost spirit
#pragma warning(disable : 4180) // from boost proto
#endif

#include "compiler.h"
#include "generator.h"
#include "instruction_visitor.h"

#define BOOST_RESULT_OF_USE_DECLTYPE
#define BOOST_SPIRIT_USE_PHOENIX_V3

#include <algorithm>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/repository/include/qi_distinct.hpp>
#include <iostream> // for std::cerr in parser
#include <ostream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

// ----- Parser description ----------------------------------------------------
namespace bf {

namespace qi    = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;

template <typename iterator>
struct skipper : qi::grammar<iterator> {
    skipper() : skipper::base_type(skip) {
        skip      = ascii::space | comment | multiline;
        comment   = qi::lit("//") >> *(qi::char_ - qi::eol) >> qi::eol;
        multiline = qi::lit("/*") >> *(qi::char_ - "*/") >> "*/";
    }

    qi::rule<iterator> skip;
    qi::rule<iterator> comment;
    qi::rule<iterator> multiline;
};

template <typename iterator>
struct grammar : qi::grammar<iterator, program_t(), skipper<iterator>> {
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
    typedef qi::rule<iterator, expression::expression_t(), skipper<iterator>> expression_rule_t;
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
    grammar() : grammar::base_type(program) {
        #define KEYWORD boost::spirit::repository::distinct(qi::char_("a-zA-Z_0-9"))
        program  = *function;
        function = KEYWORD["function"] > function_name
                 > '(' > -(variable_name % ',') > ')'
                 > '{' > *instruction > '}';
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

        // Instructions
        instruction = ( // Semicolon terminated instructions
                        ( function_call_instr
                        | variable_declaration
                        | variable_assignment
                        | print_expression
                        | print_text
                        | scan_variable
                        | return_statement
                        ) > ';')
                    // Non-semicolon terminated instructions
                    | if_else
                    | while_loop
                    | for_loop
                    | instruction_block;

        function_call_instr  = function_name >> '(' > -(expression % ',') > ')';
        variable_declaration = KEYWORD["var"] > variable_name > (('=' > expression) | qi::attr(expression::value_t{0u}));
        variable_assignment  = variable_name >> '=' > expression;
        print_expression     = KEYWORD["print"] >> expression;
        print_text           = KEYWORD["print"] >> qi::lexeme['"' > *(qi::char_ - '"') > '"'];
        scan_variable        = KEYWORD["scan"] > variable_name;
        return_statement     = KEYWORD["return"] > expression;
        if_else              = KEYWORD["if"] > '(' > expression > ')' > instruction > -(qi::lexeme["else"] > instruction);
        while_loop           = KEYWORD["while"] > '(' > expression > ')' > instruction;
        for_loop             = KEYWORD["for"] > '(' > -for_initialization > ';' > for_expression > ';' > -for_post_loop > ')' > instruction;
        for_initialization   = variable_declaration | variable_assignment;
        for_expression       = expression | qi::attr(expression::value_t{1u});
        for_post_loop        = variable_assignment.alias();
        instruction_block    = '{' > *instruction > '}';

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
        function_call_expr.name("function call expr");     // debug(function_call_expr);
        variable.name("variable");                         // debug(variable);
        parenthesized.name("parenthesized expression");    // debug(parenthesized);

        instruction.name("instruction");                   // debug(instruction);
        function_call_instr.name("function call instr");   // debug(function_call_instr);
        variable_declaration.name("variable declaration"); // debug(variable_declaration);
        variable_assignment.name("variable assignment");   // debug(variable_assignment);
        print_expression.name("print expression");         // debug(print_expression);
        print_text.name("print text");                     // debug(print_text);
        scan_variable.name("scan variable");               // debug(scan_variable);
        return_statement.name("return statement");         // debug(return_statement);
        if_else.name("if / else");                         // debug(if_else);
        while_loop.name("while loop");                     // debug(while_loop);
        for_loop.name("for loop");                         // debug(for_loop);
        for_initialization.name("for initialization");     // debug(for_initialization);
        for_expression.name("for expression");             // debug(for_expression);
        for_post_loop.name("for post loop");               // debug(for_post_loop);
        instruction_block.name("instruction block");       // debug(instruction_block);

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

    qi::rule<iterator, program_t(),   skipper<iterator>> program;
    qi::rule<iterator, function_t(),  skipper<iterator>> function;
    qi::rule<iterator, std::string(), skipper<iterator>> function_name;
    qi::rule<iterator, std::string(), skipper<iterator>> variable_name;

    qi::rule<iterator, expression::expression_t(),                                     skipper<iterator>> expression;
    qi::rule<iterator, expression::expression_t(),                                     skipper<iterator>> expression_12;
    qi::rule<iterator, expression::binary_operation_t<expression::operator_t::or_>(),  skipper<iterator>> binary_or;
    qi::rule<iterator, expression::expression_t(),                                     skipper<iterator>> expression_11;
    qi::rule<iterator, expression::binary_operation_t<expression::operator_t::and_>(), skipper<iterator>> binary_and;
    qi::rule<iterator, expression::expression_t(),                                     skipper<iterator>> expression_7;
    qi::rule<iterator, expression::binary_operation_t<expression::operator_t::eq>(),   skipper<iterator>> binary_eq;
    qi::rule<iterator, expression::binary_operation_t<expression::operator_t::neq>(),  skipper<iterator>> binary_neq;
    qi::rule<iterator, expression::expression_t(),                                     skipper<iterator>> expression_6;
    qi::rule<iterator, expression::binary_operation_t<expression::operator_t::lt>(),   skipper<iterator>> binary_lt;
    qi::rule<iterator, expression::binary_operation_t<expression::operator_t::leq>(),  skipper<iterator>> binary_leq;
    qi::rule<iterator, expression::binary_operation_t<expression::operator_t::gt>(),   skipper<iterator>> binary_gt;
    qi::rule<iterator, expression::binary_operation_t<expression::operator_t::geq>(),  skipper<iterator>> binary_geq;
    qi::rule<iterator, expression::expression_t(),                                     skipper<iterator>> expression_4;
    qi::rule<iterator, expression::binary_operation_t<expression::operator_t::add>(),  skipper<iterator>> binary_add;
    qi::rule<iterator, expression::binary_operation_t<expression::operator_t::sub>(),  skipper<iterator>> binary_sub;
    qi::rule<iterator, expression::expression_t(),                                     skipper<iterator>> expression_3;
    qi::rule<iterator, expression::binary_operation_t<expression::operator_t::mul>(),  skipper<iterator>> binary_mul;
    qi::rule<iterator, expression::expression_t(),                                     skipper<iterator>> expression_2;
    qi::rule<iterator, expression::unary_operation_t<expression::operator_t::not_>(),  skipper<iterator>> unary_not;
    qi::rule<iterator, expression::expression_t(),                                     skipper<iterator>> simple;
    qi::rule<iterator, expression::value_t(),                                          skipper<iterator>> value;
    qi::rule<iterator, expression::function_call_t(),                                  skipper<iterator>> function_call_expr;
    qi::rule<iterator, expression::variable_t(),                                       skipper<iterator>> variable;
    qi::rule<iterator, expression::parenthesized_expression_t(),                       skipper<iterator>> parenthesized;

    qi::rule<iterator, instruction::instruction_t(),          skipper<iterator>> instruction;
    qi::rule<iterator, instruction::function_call_t(),        skipper<iterator>> function_call_instr;
    qi::rule<iterator, instruction::variable_declaration_t(), skipper<iterator>> variable_declaration;
    qi::rule<iterator, instruction::variable_assignment_t(),  skipper<iterator>> variable_assignment;
    qi::rule<iterator, instruction::print_expression_t(),     skipper<iterator>> print_expression;
    qi::rule<iterator, instruction::print_text_t(),           skipper<iterator>> print_text;
    qi::rule<iterator, instruction::scan_variable_t(),        skipper<iterator>> scan_variable;
    qi::rule<iterator, instruction::return_statement_t,       skipper<iterator>> return_statement;
    qi::rule<iterator, instruction::if_else_t(),              skipper<iterator>> if_else;
    qi::rule<iterator, instruction::while_loop_t(),           skipper<iterator>> while_loop;
    qi::rule<iterator, instruction::for_loop_t(),             skipper<iterator>> for_loop;
    qi::rule<iterator, instruction::instruction_t(),          skipper<iterator>> for_initialization;
    qi::rule<iterator, expression::expression_t(),            skipper<iterator>> for_expression;
    qi::rule<iterator, instruction::instruction_t(),          skipper<iterator>> for_post_loop;
    qi::rule<iterator, instruction::instruction_block_t(),    skipper<iterator>> instruction_block;
};

// ----- Parse source to AST ---------------------------------------------------
program_t parse(const std::string &source) {
    grammar<std::string::const_iterator> gram;
    skipper<std::string::const_iterator> skip;
    program_t program;

    // Build program struct from source input.
    auto begin = source.begin(), end = source.end();
    const bool success = qi::phrase_parse(begin, end, gram, skip, program);

    if (!success || begin != end)
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
