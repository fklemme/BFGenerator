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

    BOOST_TEST_MESSAGE("Memory used to run '" + description + "':"
                       " " + std::to_string(test.get_memory().size()));
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

// ----- Compiler: Duplicate function names ------------------------------------
BOOST_AUTO_TEST_CASE(compiler_duplicate_function_names) {
    const std::string source = R"(
        function main() {}
        function main() {}
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
