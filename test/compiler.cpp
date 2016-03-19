#define BOOST_TEST_DYN_LINK
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
            var a = 2 + 5;
            var b = 3 * 3;
            a = a + b;
            print a;
        }
    )";

    bf::compiler bfc;
    const std::string program = bfc.compile(source);

    bfc_check(program, "Arithmetics", {}, {16});
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
    const std::string source = R"(
        function main() {
            var test;
            var test;
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
