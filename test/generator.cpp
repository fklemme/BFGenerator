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

BOOST_AUTO_TEST_CASE(var__add_to) {
    std::string program;
    {
        bf::generator bfg;
        auto begin = bfg.new_var();

        auto a = bfg.new_var("a");
        auto b = bfg.new_var("b");
        a->input();
        b->input();
        a->add_to(*b);
        b->output();

        // Ensure correct SP movement
        begin->add(1);

        program = bfg.get_code();
    }

    // 3 + 4 == 7
    bf::interpreter<> test1(program);
    test1.set_input({3, 4});
    test1.run();
    BOOST_CHECK(test1.get_output().at(0) == 7);
    // Ensure correct SP movement
    BOOST_CHECK(test1.get_sp_position() == 0);
    BOOST_CHECK(test1.get_current_mem() == 1);

    // 5 + 8 == 13
    bf::interpreter<> test2(program);
    test2.set_input({5, 8});
    test2.run();
    BOOST_CHECK(test2.get_output().at(0) == 13);
    // Ensure correct SP movement
    BOOST_CHECK(test2.get_sp_position() == 0);
    BOOST_CHECK(test2.get_current_mem() == 1);
}

BOOST_AUTO_TEST_CASE(var__sub_from) {
    std::string program;
    {
        bf::generator bfg;
        auto begin = bfg.new_var();

        auto a = bfg.new_var("a");
        auto b = bfg.new_var("b");
        a->input();
        b->input();
        a->sub_from(*b);
        b->output();

        // Ensure correct SP movement
        begin->add(1);

        program = bfg.get_code();
    }

    // 5 - 3 == 2
    bf::interpreter<> test1(program);
    test1.set_input({3, 5});
    test1.run();
    BOOST_CHECK(test1.get_output().at(0) == 2);
    // Ensure correct SP movement
    BOOST_CHECK(test1.get_sp_position() == 0);
    BOOST_CHECK(test1.get_current_mem() == 1);

    // 20 - 7 == 13
    bf::interpreter<> test2(program);
    test2.set_input({7, 20});
    test2.run();
    BOOST_CHECK(test2.get_output().at(0) == 13);
    // Ensure correct SP movement
    BOOST_CHECK(test2.get_sp_position() == 0);
    BOOST_CHECK(test2.get_current_mem() == 1);
}

BOOST_AUTO_TEST_CASE(var__not_of) {
    std::string program;
    {
        bf::generator bfg;
        auto begin = bfg.new_var();

        auto a = bfg.new_var("a");
        auto b = bfg.new_var("b");
        a->input();
        b->not_of(*a);
        b->output();

        // Ensure correct SP movement
        begin->add(1);

        program = bfg.get_code();
    }

    // !0 == 1
    bf::interpreter<> test1(program);
    test1.set_input({0});
    test1.run();
    BOOST_CHECK(test1.get_output().at(0) == 1);
    // Ensure correct SP movement
    BOOST_CHECK(test1.get_sp_position() == 0);
    BOOST_CHECK(test1.get_current_mem() == 1);

    // !1 == 0
    bf::interpreter<> test2(program);
    test2.set_input({1});
    test2.run();
    BOOST_CHECK(test2.get_output().at(0) == 0);
    // Ensure correct SP movement
    BOOST_CHECK(test2.get_sp_position() == 0);
    BOOST_CHECK(test2.get_current_mem() == 1);

    // !7 == 0
    bf::interpreter<> test3(program);
    test3.set_input({7});
    test3.run();
    BOOST_CHECK(test3.get_output().at(0) == 0);
    // Ensure correct SP movement
    BOOST_CHECK(test3.get_sp_position() == 0);
    BOOST_CHECK(test3.get_current_mem() == 1);
}
