#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE compiler
#include <boost/test/unit_test.hpp>

#include "../bf/compiler.h"
#include "../bf/interpreter.h"

template <typename memory_type = unsigned char, std::size_t memory_size = 128>
void bfc_check(const std::string &program, const std::string &description,
        const std::vector<memory_type> &input, const std::vector<memory_type> &expected_output)
{
    bf::interpreter<memory_type, memory_size> test(program);
    test.send_input(input);
    test.run();
    const auto received_output = test.recv_output();

    BOOST_CHECK_MESSAGE(std::equal(expected_output.begin(), expected_output.end(),
                                   received_output.begin(), received_output.end()),
                        "Unexpected result after processing '" + description + "'!");
}

// ----- Example program: Hello world ------------------------------------------
BOOST_AUTO_TEST_CASE(example_hello_world) {
    const std::string source = R"(
        function main() {
            print("Hello world");
        }
    )";

    bf::compiler bfc;
    const std::string program = bfc.compile(source);
    const std::string result = "Hello world";

    bfc_check(program, "Hello world", {}, {result.begin(), result.end()});
}
