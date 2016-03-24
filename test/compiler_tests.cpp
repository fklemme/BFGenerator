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

// ----- Compiler: Scan and print ----------------------------------------------
BOOST_AUTO_TEST_CASE(compiler_scan_and_print) {
    const std::string source = R"(
        function main() {
            var a;
            scan a;
            print a;
        }
    )";

    bf::compiler bfc;
    const std::string program = bfc.compile(source);

    bfc_check(program, "Echo program", {2}, {2});
    bfc_check(program, "Echo program", {5}, {5});
    bfc_check(program, "Echo program", {17}, {17});
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
