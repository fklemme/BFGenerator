#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE generator
#include <boost/test/unit_test.hpp>

#include "../bf/generator.h"
#include "../bf/interpreter.h"

BOOST_AUTO_TEST_CASE(var__add) {
    std::string program;
    {
        bf::generator bfg;

        auto a = bfg.new_var("a");
        a->input();
        a->add(3);
        a->output();

        program = bfg.get_code();
    }

    // 2 + 3 == 5
    bf::interpreter<> test1(program);
    test1.set_input({2});
    test1.run();
    BOOST_CHECK(test1.get_output().at(0) == 5);

    // 7 + 3 == 10
    bf::interpreter<> test2(program);
    test2.set_input({7});
    test2.run();
    BOOST_CHECK(test2.get_output().at(0) == 10);
}

BOOST_AUTO_TEST_CASE(var__sub) {
    std::string program;
    {
        bf::generator bfg;

        auto a = bfg.new_var("a");
        a->input();
        a->sub(2);
        a->output();

        program = bfg.get_code();
    }

    // 5 - 2 == 3
    bf::interpreter<> test1(program);
    test1.set_input({5});
    test1.run();
    BOOST_CHECK(test1.get_output().at(0) == 3);

    // 10 - 2 == 8
    bf::interpreter<> test2(program);
    test2.set_input({10});
    test2.run();
    BOOST_CHECK(test2.get_output().at(0) == 8);
}

BOOST_AUTO_TEST_CASE(var__mult) {
    std::string program;
    {
        bf::generator bfg;

        auto a = bfg.new_var("a");
        a->input();
        a->mult(3);
        a->output();

        program = bfg.get_code();
    }

    // 5 * 3 == 15
    bf::interpreter<> test1(program);
    test1.set_input({5});
    test1.run();
    BOOST_CHECK(test1.get_output().at(0) == 15);

    // 8 * 3 == 24
    bf::interpreter<> test2(program);
    test2.set_input({8});
    test2.run();
    BOOST_CHECK(test2.get_output().at(0) == 24);
}

BOOST_AUTO_TEST_CASE(var__copy_to) {
    std::string program;
    {
        bf::generator bfg;
        auto begin = bfg.new_var();

        auto a = bfg.new_var("a");
        auto b = bfg.new_var("b");
        a->input();
        a->copy_to(*b);
        b->output();

        // Ensure correct SP movement
        begin->add(1);

        program = bfg.get_code();
    }

    // Simple copy test
    bf::interpreter<> test1(program);
    test1.set_input({5});
    test1.run();
    BOOST_CHECK(test1.get_output().at(0) == 5);
    // Ensure correct SP movement
    BOOST_CHECK(test1.get_sp_position() == 0);
    BOOST_CHECK(test1.get_current_mem() == 1);
}
