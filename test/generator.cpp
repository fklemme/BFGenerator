#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE generator
#include <boost/test/unit_test.hpp>

#include "../bf/generator.h"
#include "../bf/interpreter.h"

template <typename memory_type = unsigned char, std::size_t memory_size = 128>
void bfg_check(const std::string &program, const std::string &message,
        const std::vector<memory_type> &input, const std::vector<memory_type> &output)
{
    bf::interpreter<memory_type, memory_size> test(program);
    test.send_input(input);
    test.run();

    BOOST_CHECK_MESSAGE(std::equal(output.begin(), output.end(), test.recv_output().begin()), message);
    // Ensure correct SP movement
    BOOST_CHECK(test.get_sp_position() == 0);
    BOOST_CHECK(test.get_current_mem() == 1);
}

BOOST_AUTO_TEST_CASE(var__add_unsigned) {
    std::string program;
    {
        bf::generator bfg;
        auto begin = bfg.new_var();

        auto a = bfg.new_var("a");
        a->read_input();
        a->add(3);
        a->write_output();

        // Ensure correct SP movement
        begin->add(1);
        program = bfg.get_code();
    }

    bfg_check(program, "2 + 3 == 5", {2}, {5});
    bfg_check(program, "7 + 3 == 10", {7}, {10});
}

BOOST_AUTO_TEST_CASE(var__subtract_unsigned) {
    std::string program;
    {
        bf::generator bfg;
        auto begin = bfg.new_var();

        auto a = bfg.new_var("a");
        a->read_input();
        a->subtract(2);
        a->write_output();

        // Ensure correct SP movement
        begin->add(1);
        program = bfg.get_code();
    }

    bfg_check(program, "5 - 2 == 3", {5}, {3});
    bfg_check(program, "10 - 2 == 8", {10}, {8});
}

BOOST_AUTO_TEST_CASE(var__multiply_unsigned) {
    std::string program;
    {
        bf::generator bfg;
        auto begin = bfg.new_var();

        auto a = bfg.new_var("a");
        a->read_input();
        a->multiply(3);
        a->write_output();

        // Ensure correct SP movement
        begin->add(1);
        program = bfg.get_code();
    }

    bfg_check(program, "5 * 3 == 15", {5}, {15});
    bfg_check(program, "8 * 3 == 24", {8}, {24});
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

    bfg_check(program, "Simple copy instruction", {5}, {5});
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

    bfg_check(program, "3 + 4 == 7", {3, 4}, {7});
    bfg_check(program, "5 + 8 == 13", {5, 8}, {13});
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

    bfg_check(program, "5 - 3 == 2", {5, 3}, {2});
    bfg_check(program, "20 - 7 == 13", {20, 7}, {13});
}


BOOST_AUTO_TEST_CASE(var__multiply) {
    std::string program;
    {
        bf::generator bfg;
        auto begin = bfg.new_var();

        auto a = bfg.new_var("a");
        auto b = bfg.new_var("b");
        a->read_input();
        b->read_input();
        a->multiply(*b);
        a->write_output();

        // Ensure correct SP movement
        begin->add(1);
        program = bfg.get_code();
    }

    bfg_check(program, "5 * 3 == 15", {5, 3}, {15});
    bfg_check(program, "4 * 8 == 32", {4, 8}, {32});
}

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

    bfg_check(program, "!0 == 1", {0}, {1});
    bfg_check(program, "!1 == 0", {1}, {0});
    bfg_check(program, "!7 == 0", {7}, {0});
}

BOOST_AUTO_TEST_CASE(var__lower_than) {
    std::string program;
    {
        bf::generator bfg;
        auto begin = bfg.new_var();

        auto a = bfg.new_var("a");
        auto b = bfg.new_var("b");
        a->read_input();
        b->read_input();
        a->lower_than(*b);
        a->write_output();

        // Ensure correct SP movement
        begin->add(1);
        program = bfg.get_code();
    }

    bfg_check(program, "(4 < 5) == 1", {4, 5}, {1});
    bfg_check(program, "(5 < 5) == 0", {5, 5}, {0});
    bfg_check(program, "(5 < 4) == 0", {5, 4}, {0});
}

BOOST_AUTO_TEST_CASE(var__lower_equal) {
    std::string program;
    {
        bf::generator bfg;
        auto begin = bfg.new_var();

        auto a = bfg.new_var("a");
        auto b = bfg.new_var("b");
        a->read_input();
        b->read_input();
        a->lower_equal(*b);
        a->write_output();

        // Ensure correct SP movement
        begin->add(1);
        program = bfg.get_code();
    }

    bfg_check(program, "(4 <= 5) == 1", {4, 5}, {1});
    bfg_check(program, "(5 <= 5) == 1", {5, 5}, {1});
    bfg_check(program, "(5 <= 4) == 0", {5, 4}, {0});
}

BOOST_AUTO_TEST_CASE(var__greater_than) {
    std::string program;
    {
        bf::generator bfg;
        auto begin = bfg.new_var();

        auto a = bfg.new_var("a");
        auto b = bfg.new_var("b");
        a->read_input();
        b->read_input();
        a->greater_than(*b);
        a->write_output();

        // Ensure correct SP movement
        begin->add(1);
        program = bfg.get_code();
    }

    bfg_check(program, "(4 > 5) == 0", {4, 5}, {0});
    bfg_check(program, "(5 > 5) == 0", {5, 5}, {0});
    bfg_check(program, "(5 > 4) == 1", {5, 4}, {1});
}

BOOST_AUTO_TEST_CASE(var__greater_equal) {
    std::string program;
    {
        bf::generator bfg;
        auto begin = bfg.new_var();

        auto a = bfg.new_var("a");
        auto b = bfg.new_var("b");
        a->read_input();
        b->read_input();
        a->greater_equal(*b);
        a->write_output();

        // Ensure correct SP movement
        begin->add(1);
        program = bfg.get_code();
    }

    bfg_check(program, "(4 >= 5) == 0", {4, 5}, {0});
    bfg_check(program, "(5 >= 5) == 1", {5, 5}, {1});
    bfg_check(program, "(5 >= 4) == 1", {5, 4}, {1});
}

BOOST_AUTO_TEST_CASE(var__equal) {
    std::string program;
    {
        bf::generator bfg;
        auto begin = bfg.new_var();

        auto a = bfg.new_var("a");
        auto b = bfg.new_var("b");
        a->read_input();
        b->read_input();
        a->equal(*b);
        a->write_output();

        // Ensure correct SP movement
        begin->add(1);
        program = bfg.get_code();
    }

    bfg_check(program, "(2 == 2) == 1", {2, 2}, {1});
    bfg_check(program, "(3 == 4) == 0", {3, 4}, {0});
    bfg_check(program, "(5 == 4) == 0", {5, 4}, {0});
    bfg_check(program, "(0 == 0) == 1", {0, 0}, {1});
}

BOOST_AUTO_TEST_CASE(var__not_equal) {
    std::string program;
    {
        bf::generator bfg;
        auto begin = bfg.new_var();

        auto a = bfg.new_var("a");
        auto b = bfg.new_var("b");
        a->read_input();
        b->read_input();
        a->not_equal(*b);
        a->write_output();

        // Ensure correct SP movement
        begin->add(1);
        program = bfg.get_code();
    }

    bfg_check(program, "(2 != 2) == 0", {2, 2}, {0});
    bfg_check(program, "(3 != 4) == 1", {3, 4}, {1});
    bfg_check(program, "(5 != 4) == 1", {5, 4}, {1});
    bfg_check(program, "(0 != 0) == 0", {0, 0}, {0});
}

BOOST_AUTO_TEST_CASE(example_ggt) {
    std::string program;
    {
        bf::generator bfg;
        auto begin = bfg.new_var();

        auto a = bfg.new_var("a");
        auto b = bfg.new_var("b");
        a->read_input();
        b->read_input();

        auto eq = bfg.new_var("eq");
        eq->copy(*a);
        eq->not_equal(*b);
        bfg.while_begin(*eq); // a != b
        {
            auto lt = bfg.new_var("lt");
            lt->copy(*a);
            lt->lower_than(*b);
            bfg.if_begin(*lt); // a < b
            {
                b->subtract(*a);
            }
            bfg.if_end(*lt);

            auto ge = bfg.new_var("ge");
            ge->bool_not(*lt);
            bfg.if_begin(*ge); // a > b
            {
                a->subtract(*b);
            }
            bfg.if_end(*ge);

            eq->copy(*a);
            eq->not_equal(*b);
        }
        bfg.while_end(*eq);
        a->write_output();

        // Ensure correct SP movement
        begin->add(1);
        program = bfg.get_code();
    }

    bfg_check     (program, "ggt(24,   16)   == 8",   {24,   16},   {8});
    bfg_check     (program, "ggt(128,  42)   == 2",   {128,  42},   {2});
    bfg_check     (program, "ggt(9,    13)   == 1",   {9,    13},   {1});
    bfg_check<int>(program, "ggt(3528, 3780) == 252", {3528, 3780}, {252});
}
