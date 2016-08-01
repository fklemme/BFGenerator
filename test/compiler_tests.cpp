#ifndef _WIN32
#define BOOST_TEST_DYN_LINK
#endif
#define BOOST_TEST_MODULE compiler
#include <boost/test/unit_test.hpp>

#include "../bf/compiler.h"
#include "../bf/interpreter.h"

template <typename memory_type = unsigned char>
void bfc_check(const std::string &program, const std::string &description,
        const std::vector<memory_type> &input, const std::vector<memory_type> &expected_output)
{
    bf::interpreter<memory_type> test(program);
    test.send_input(input);
    test.run();
    const auto received_output = test.recv_output();

    BOOST_CHECK_MESSAGE(std::equal(expected_output.begin(), expected_output.end(),
                                   received_output.begin(), received_output.end()),
                        "Unexpected result after processing '" + description + "'!");

    BOOST_TEST_MESSAGE("----- Results for '" + description + "' -----");
    std::string output_int;
    for (auto v : received_output)
        output_int += " " + std::to_string(v);
    BOOST_TEST_MESSAGE("Received output (as int):" + output_int);
    BOOST_TEST_MESSAGE("Memory used: " + std::to_string(test.get_memory().size()));
}

// ----- Example program: Hello world ------------------------------------------
BOOST_AUTO_TEST_CASE(example_hello_world) {
    const std::string source = R"(
        function main() {
            print "Hello world";
        }
    )";

    bf::compiler bfc;
    const std::string program = bfc.compile(source);
    const std::string result = "Hello world";

    bfc_check(program, "Hello world", {}, {result.begin(), result.end()});
}

// ----- Compiler: Function call -----------------------------------------------
BOOST_AUTO_TEST_CASE(compiler_function_call) {
    const std::string source = R"(
        function main() {
            test();
        }
        function test() {
            print "test";
        }
    )";

    bf::compiler bfc;
    const std::string program = bfc.compile(source);
    const std::string result = "test";

    bfc_check(program, "Function call 'test()'", {}, {result.begin(), result.end()});
}

// ----- Compiler: Function return value ---------------------------------------
/*
BOOST_AUTO_TEST_CASE(compiler_function_return_value) {
    const std::string source = R"(
        function main() {
            var a = five();
            var b = zero();
            print a;
            print b;
        }
        function five() {
            return 5;
        }
        function zero() {}
    )";

    bf::compiler bfc;
    const std::string program = bfc.compile(source);

    bfc_check(program, "Function return value", {}, {5, 0});
}
*/

// ----- Compiler: Scan and print ----------------------------------------------
BOOST_AUTO_TEST_CASE(compiler_scan_and_print) {
    const std::string source = R"(
        function main() {
            var a;
            scan a;
            print a;
            print a + 2;
        }
    )";

    bf::compiler bfc;
    const std::string program = bfc.compile(source);

    bfc_check(program, "Echo, plus two", {2},  {2,  4});
    bfc_check(program, "Echo, plus two", {5},  {5,  7});
    bfc_check(program, "Echo, plus two", {17}, {17, 19});
}

// ----- Compiler: Scan and print 2 --------------------------------------------
BOOST_AUTO_TEST_CASE(compiler_scan_and_print_2) {
    const std::string source = R"(
        function main() {
            var a;
            scan a;
            print a;
            print 'a' + a;
        }
    )";

    bf::compiler bfc;
    const std::string program = bfc.compile(source);

    bfc_check(program, "Echo, plus 'a'", {2},  {2,  'c'});
    bfc_check(program, "Echo, plus 'a'", {5},  {5,  'f'});
    bfc_check(program, "Echo, plus 'a'", {17}, {17, ('a' + 17)});
}

// ----- Compiler: Arithmetics -------------------------------------------------
BOOST_AUTO_TEST_CASE(compiler_arithmetics) {
    const std::string source = R"(
        function main() {
            var a = 2 + 3;
            var b = 5 - 3;
            var c = 3 * 3;
            a = a * b + c;
            print a;
        }
    )";

    bf::compiler bfc;
    const std::string program = bfc.compile(source);

    bfc_check(program, "Arithmetics", {}, {19});
}

// ----- Compiler: Arithmetics 2 -----------------------------------------------
BOOST_AUTO_TEST_CASE(compiler_arithmetics_2) {
    const std::string source = R"(
        function main() {
            var n10 = 1 + 3 * 3;
            var n8  = (1 + 3) * 2;
            var n12 = (5 - 2 + 1) * (1 + 2);
            var n36 = (n10 - 8) * 2 + n8 + 2 * n12;

            print n10;
            print n8;
            print n12;
            print n36;
        }
    )";

    bf::compiler bfc;
    const std::string program = bfc.compile(source);

    bfc_check(program, "Arithmetics 2", {}, {10, 8, 12, 36});
}

// ----- Compiler: Arithmetics minus -------------------------------------------
BOOST_AUTO_TEST_CASE(compiler_arithmetics_minus) {
    const std::string source = R"(
        function main() {
            var m0 = 2 - 1 - 1;
            var m2 = 2 - (1 - 1);
            var p2 = 2 - 1 + 1;
            var p0 = 2 - (1 + 1);
            var x3 = 5 - 3 - 1 + 5 - 2 - 1;
            var y3  = 5 - 1 - 1 - 1 - 1
                    + 5 - 1 - 1 - 1 - 1
                    + 5 - 1 - 1 - 1 - 1;

            print m0;
            print m2;
            print p2;
            print p0;
            print x3;
            print y3;
        }
    )";

    bf::compiler bfc;
    const std::string program = bfc.compile(source);

    bfc_check(program, "Arithmetics 'minus'", {}, {0, 2, 2, 0, 3, 3});
}

// ----- Compiler: Comparisons -------------------------------------------------
BOOST_AUTO_TEST_CASE(compiler_comparisons) {
    const std::string source = R"(
        function main() {
            var a = 2;
            var b = 5;
            var t1 = a < b;
            var f1 = a > b;
            var f2 = a == b;

            print t1;
            print f1;
            print f2;
        }
    )";

    bf::compiler bfc;
    const std::string program = bfc.compile(source);

    bfc_check(program, "Comparisons", {}, {1, 0, 0});
}

// ----- Compiler: Comparisons 2 -----------------------------------------------
BOOST_AUTO_TEST_CASE(compiler_comparisons_2) {
    const std::string source = R"(
        function main() {
            var a = 2;
            var b = 5;
            var t1 = a < b && !(a > b);
            var f1 = a > b || 0;
            var t2 = a > b && 0 || 1;
            var f2 = a == b && a != b;
            var f3 = 1 > 2 || 5 == 6;

            print t1;
            print f1;
            print t2;
            print f2;
            print f3;
        }
    )";

    bf::compiler bfc;
    const std::string program = bfc.compile(source);

    bfc_check(program, "Comparisons 2", {}, {1, 0, 1, 0, 0});
}

// ----- Compiler: Comparisons operator precedence -----------------------------
BOOST_AUTO_TEST_CASE(compiler_comp_op_precedence) {
    const std::string source = R"(
        function main() {
            var a = 2;
            var b = 5;
            var c = 2;
            var t_implicit = a < b < c;
            var t_explicit = (a < b) < c;
            var f_explicit = a < (b < c);

            print t_implicit;
            print t_explicit;
            print f_explicit;
        }
    )";

    bf::compiler bfc;
    const std::string program = bfc.compile(source);

    bfc_check(program, "Comparisons operator precedence", {}, {1, 1, 0});
}

// ----- Compiler: Conditional statements --------------------------------------
BOOST_AUTO_TEST_CASE(compiler_conditional_statements) {
    const std::string source = R"(
        function main() {
            if (2 < 5)
                print "test";
        }
    )";

    bf::compiler bfc;
    const std::string program = bfc.compile(source);
    const std::string result = "test";

    bfc_check(program, "Conditional statements", {}, {result.begin(), result.end()});
}

// ----- Compiler: Conditional statements 2 --------------------------------------
BOOST_AUTO_TEST_CASE(compiler_conditional_statements_2) {
    const std::string source = R"(
        function main() {
            if (2 > 5)
                print "fail";
            else
                print "test";
        }
    )";

    bf::compiler bfc;
    const std::string program = bfc.compile(source);
    const std::string result = "test";

    bfc_check(program, "Conditional statements 2", {}, {result.begin(), result.end()});
}

// ----- Compiler: Conditionals and scopes -------------------------------------
BOOST_AUTO_TEST_CASE(compiler_conditionals_scopes) {
    const std::string source = R"(
        function main() {
            var tmp = 5;
            if (tmp == 5) {
                var tmp = 7;
                tmp = tmp + 2;

                if (tmp == 9)
                    print "res";
            }

            if (1 != 0)
                var tmp = 27;

            tmp = tmp - 3;
            if (tmp == 2)
                print "ult";
            else
                print "fail";
        }
    )";

    bf::compiler bfc;
    const std::string program = bfc.compile(source);
    const std::string result = "result";

    bfc_check(program, "Conditionals and scopes", {}, {result.begin(), result.end()});
}

// ----- Compiler: While loop --------------------------------------------------
BOOST_AUTO_TEST_CASE(compiler_while_loop) {
    const std::string source = R"(
        function main() {
            var a = 2;
            var b = 7;
            while (a < b) {
                print "x";
                b = b - 1;
            }
        }
    )";

    bf::compiler bfc;
    const std::string program = bfc.compile(source);
    const std::string result(5, 'x');

    bfc_check(program, "While loop", {}, {result.begin(), result.end()});
}

// ----- Compiler: For loop ----------------------------------------------------
BOOST_AUTO_TEST_CASE(compiler_for_loop) {
    const std::string source = R"(
        function main() {
            for (var i = 2; i < 7; i = i + 1)
                print "x";
        }
    )";

    bf::compiler bfc;
    const std::string program = bfc.compile(source);
    const std::string result(5, 'x');

    bfc_check(program, "For loop", {}, {result.begin(), result.end()});
}

// ----- Compiler: For loop 2 --------------------------------------------------
BOOST_AUTO_TEST_CASE(compiler_for_loop_2) {
    const std::string source = R"(
        function main() {
            var i = 5;
            for (; i > 0; i = i - 1) {
                print "y";
                var i = 23;
            }
        }
    )";

    bf::compiler bfc;
    const std::string program = bfc.compile(source);
    const std::string result(5, 'y');

    bfc_check(program, "For loop 2", {}, {result.begin(), result.end()});
}

// ----- Compiler: No main function --------------------------------------------
BOOST_AUTO_TEST_CASE(compiler_no_main_function) {
	const std::string source = R"(
        function non_main() {}
    )";

	bf::compiler bfc;
	BOOST_CHECK_THROW(bfc.compile(source), std::exception);
}

// ----- Compiler: Duplicate function names ------------------------------------
BOOST_AUTO_TEST_CASE(compiler_duplicate_function_names) {
    const std::string source = R"(
        function main() {}
        function main() {}
    )";

    bf::compiler bfc;
    BOOST_CHECK_THROW(bfc.compile(source), std::exception);
}

// ----- Compiler: Duplicate variable names ------------------------------------
BOOST_AUTO_TEST_CASE(compiler_duplicate_variable_names) {
    // TODO: Underscores don't seem to be printed correctly. ASCII/UTF-8 problem?
    const std::string source = R"(
        function main() {
            var decleared_twice;
            var decleared_twice;
        }
    )";

    bf::compiler bfc;
    BOOST_CHECK_THROW(bfc.compile(source), std::exception);
}

// ----- Compiler: Undeclared variable -----------------------------------------
BOOST_AUTO_TEST_CASE(compiler_undeclared_variable) {
    const std::string source = R"(
        function main() {
            undeclared = 5;
        }
    )";

    bf::compiler bfc;
    BOOST_CHECK_THROW(bfc.compile(source), std::exception);
}

// ----- Compiler: Function recursion ------------------------------------------
BOOST_AUTO_TEST_CASE(compiler_function_recursion) {
    const std::string source = R"(
        function main() {
            recursion();
        }
        function recursion() {
            recursion();
        }
    )";

    bf::compiler bfc;
    BOOST_CHECK_THROW(bfc.compile(source), std::exception);
}

// ----- Compiler: Function recursion 2 ----------------------------------------
BOOST_AUTO_TEST_CASE(compiler_function_recursion_2) {
    const std::string source = R"(
        function main() {
            recursion();
        }
        function recursion() {
            var a = recursion();
        }
    )";

    bf::compiler bfc;
    BOOST_CHECK_THROW(bfc.compile(source), std::exception);
}
