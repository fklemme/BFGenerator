#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE generator
#include <boost/test/unit_test.hpp>

#include "../bf/generator.h"
#include "../bf/interpreter.h"

BOOST_AUTO_TEST_CASE(var__add_unsigned) {
    std::string program;
    {
        bf::generator bfg;

        auto a = bfg.new_var("a");
        a->read_input();
        a->add(3);
        a->write_output();

        program = bfg.get_code();
    }

    bf::interpreter<> test1(program);
    test1.set_input({2});
    test1.run();
    BOOST_CHECK_MESSAGE(test1.get_output().at(0) == 5, "2 + 3 == 5");

    bf::interpreter<> test2(program);
    test2.set_input({7});
    test2.run();
    BOOST_CHECK_MESSAGE(test2.get_output().at(0) == 10, "7 + 3 == 10");
}

BOOST_AUTO_TEST_CASE(var__subtract_unsigned) {
    std::string program;
    {
        bf::generator bfg;

        auto a = bfg.new_var("a");
        a->read_input();
        a->subtract(2);
        a->write_output();

        program = bfg.get_code();
    }

    bf::interpreter<> test1(program);
    test1.set_input({5});
    test1.run();
    BOOST_CHECK_MESSAGE(test1.get_output().at(0) == 3, "5 - 2 == 3");

    bf::interpreter<> test2(program);
    test2.set_input({10});
    test2.run();
    BOOST_CHECK_MESSAGE(test2.get_output().at(0) == 8, "10 - 2 == 8");
}

BOOST_AUTO_TEST_CASE(var__multiply_unsigned) {
    std::string program;
    {
        bf::generator bfg;

        auto a = bfg.new_var("a");
        a->read_input();
        a->multiply(3);
        a->write_output();

        program = bfg.get_code();
    }

    bf::interpreter<> test1(program);
    test1.set_input({5});
    test1.run();
    BOOST_CHECK_MESSAGE(test1.get_output().at(0) == 15, "5 * 3 == 15");

    bf::interpreter<> test2(program);
    test2.set_input({8});
    test2.run();
    BOOST_CHECK_MESSAGE(test2.get_output().at(0) == 24, "8 * 3 == 24");
}

BOOST_AUTO_TEST_CASE(var__copy) {
    std::string program;
    {
        bf::generator bfg;
        auto begin = bfg.new_var();

        auto a = bfg.new_var("a");
        auto b = bfg.new_var("b");
        a->read_input();
        b->copy(*a);
        b->write_output();

        // Ensure correct SP movement
        begin->add(1);
        program = bfg.get_code();
    }

    bf::interpreter<> test1(program);
    test1.set_input({5});
    test1.run();
    BOOST_CHECK_MESSAGE(test1.get_output().at(0) == 5, "Simple copy instruction");
    BOOST_CHECK(test1.get_sp_position() == 0);
    BOOST_CHECK(test1.get_current_mem() == 1);
}

BOOST_AUTO_TEST_CASE(var__add) {
    std::string program;
    {
        bf::generator bfg;
        auto begin = bfg.new_var();

        auto a = bfg.new_var("a");
        auto b = bfg.new_var("b");
        a->read_input();
        b->read_input();
        a->add(*b);
        a->write_output();

        // Ensure correct SP movement
        begin->add(1);
        program = bfg.get_code();
    }

    bf::interpreter<> test1(program);
    test1.set_input({3, 4});
    test1.run();
    BOOST_CHECK_MESSAGE(test1.get_output().at(0) == 7, "3 + 4 == 7");
    BOOST_CHECK(test1.get_sp_position() == 0);
    BOOST_CHECK(test1.get_current_mem() == 1);

    bf::interpreter<> test2(program);
    test2.set_input({5, 8});
    test2.run();
    BOOST_CHECK_MESSAGE(test2.get_output().at(0) == 13, "5 + 8 == 13");
    BOOST_CHECK(test2.get_sp_position() == 0);
    BOOST_CHECK(test2.get_current_mem() == 1);
}

BOOST_AUTO_TEST_CASE(var__subtract) {
    std::string program;
    {
        bf::generator bfg;
        auto begin = bfg.new_var();

        auto a = bfg.new_var("a");
        auto b = bfg.new_var("b");
        a->read_input();
        b->read_input();
        a->subtract(*b);
        a->write_output();

        // Ensure correct SP movement
        begin->add(1);
        program = bfg.get_code();
    }

    bf::interpreter<> test1(program);
    test1.set_input({5, 3});
    test1.run();
    BOOST_CHECK_MESSAGE(test1.get_output().at(0) == 2, "5 - 3 == 2");
    BOOST_CHECK(test1.get_sp_position() == 0);
    BOOST_CHECK(test1.get_current_mem() == 1);

    bf::interpreter<> test2(program);
    test2.set_input({20, 7});
    test2.run();
    BOOST_CHECK_MESSAGE(test2.get_output().at(0) == 13, "20 - 7 == 13");
    BOOST_CHECK(test2.get_sp_position() == 0);
    BOOST_CHECK(test2.get_current_mem() == 1);
}

// TODO: var__multiply

BOOST_AUTO_TEST_CASE(var__bool_not) {
    std::string program;
    {
        bf::generator bfg;
        auto begin = bfg.new_var();

        auto a = bfg.new_var("a");
        auto b = bfg.new_var("b");
        a->read_input();
        b->bool_not(*a);
        b->write_output();

        // Ensure correct SP movement
        begin->add(1);
        program = bfg.get_code();
    }

    bf::interpreter<> test1(program);
    test1.set_input({0});
    test1.run();
    BOOST_CHECK_MESSAGE(test1.get_output().at(0) == 1, "!0 == 1");
    BOOST_CHECK(test1.get_sp_position() == 0);
    BOOST_CHECK(test1.get_current_mem() == 1);

    bf::interpreter<> test2(program);
    test2.set_input({1});
    test2.run();
    BOOST_CHECK_MESSAGE(test2.get_output().at(0) == 0, "!1 == 0");
    BOOST_CHECK(test2.get_sp_position() == 0);
    BOOST_CHECK(test2.get_current_mem() == 1);

    bf::interpreter<> test3(program);
    test3.set_input({7});
    test3.run();
    BOOST_CHECK_MESSAGE(test3.get_output().at(0) == 0, "!7 == 0");
    BOOST_CHECK(test3.get_sp_position() == 0);
    BOOST_CHECK(test3.get_current_mem() == 1);
}
