#pragma once

#include "ast_types.h"
#include "error_handler.h"
#include "expression_grammar.h"

#define BOOST_RESULT_OF_USE_DECLTYPE
#define BOOST_SPIRIT_USE_PHOENIX_V3

#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/repository/include/qi_distinct.hpp>

namespace bf {

namespace qi    = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;

template <typename iterator>
struct instruction_g : qi::grammar<iterator, program_t(), skipper_g<iterator>> {
    instruction_g() : instruction_g::base_type(program) {
        #define KEYWORD boost::spirit::repository::distinct(qi::alnum | '_')
        #define KEYWORDS KEYWORD[qi::lit("function") | "var" | "print" | "scan" | "if" | "else" | "while" | "for"]
        function_name = qi::lexeme[((qi::alpha | '_') >> *(qi::alnum | '_')) - KEYWORDS];
        variable_name = qi::lexeme[((qi::alpha | '_') >> *(qi::alnum | '_')) - KEYWORDS];

        program = *function > qi::eoi;
        function = KEYWORD["function"] > function_name
                 > '(' > -(variable_name % ',') > ')'
                 > '{' > *instruction > '}';

        // Instructions
        // Parentheses used here to prevent bug: http://stackoverflow.com/q/19823413
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

        function_name.name("function name");               // debug(function_name);
        variable_name.name("variable name");               // debug(variable_name);
        program.name("program");                           // debug(program);
        function.name("function");                         // debug(function);

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
        qi::on_error<qi::fail>(program, boost::phoenix::bind(on_error, qi::_1, qi::_2, qi::_3, qi::_4));
    }

    expression_g<iterator>  expression;
    error_handler<iterator> on_error;

    qi::rule<iterator, std::string(), skipper_g<iterator>> function_name;
    qi::rule<iterator, std::string(), skipper_g<iterator>> variable_name;
    qi::rule<iterator, program_t(),   skipper_g<iterator>> program;
    qi::rule<iterator, function_t(),  skipper_g<iterator>> function;

    qi::rule<iterator, instruction::instruction_t(),          skipper_g<iterator>> instruction;
    qi::rule<iterator, instruction::function_call_t(),        skipper_g<iterator>> function_call_instr;
    qi::rule<iterator, instruction::variable_declaration_t(), skipper_g<iterator>> variable_declaration;
    qi::rule<iterator, instruction::variable_assignment_t(),  skipper_g<iterator>> variable_assignment;
    qi::rule<iterator, instruction::print_expression_t(),     skipper_g<iterator>> print_expression;
    qi::rule<iterator, instruction::print_text_t(),           skipper_g<iterator>> print_text;
    qi::rule<iterator, instruction::scan_variable_t(),        skipper_g<iterator>> scan_variable;
    qi::rule<iterator, instruction::return_statement_t,       skipper_g<iterator>> return_statement;
    qi::rule<iterator, instruction::if_else_t(),              skipper_g<iterator>> if_else;
    qi::rule<iterator, instruction::while_loop_t(),           skipper_g<iterator>> while_loop;
    qi::rule<iterator, instruction::for_loop_t(),             skipper_g<iterator>> for_loop;
    qi::rule<iterator, instruction::instruction_t(),          skipper_g<iterator>> for_initialization;
    qi::rule<iterator, expression::expression_t(),            skipper_g<iterator>> for_expression;
    qi::rule<iterator, instruction::instruction_t(),          skipper_g<iterator>> for_post_loop;
    qi::rule<iterator, instruction::instruction_block_t(),    skipper_g<iterator>> instruction_block;
};

} // namespace bf
