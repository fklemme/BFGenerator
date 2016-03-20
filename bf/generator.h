#pragma once

#include <array>
#include <map>
#include <memory>
#include <ostream>
#include <string>
#include <tuple>
#include <vector>

namespace bf {

static const std::string bf_ops = "><+-.,[]";

class var;

class generator {
    friend class var;

public:
    typedef std::shared_ptr<var> var_ptr;

    var_ptr new_var(std::string var_name = "", unsigned init_value = 0, unsigned pref_stack_pos = 0);

    template <unsigned size>
    std::array<var_ptr, size> new_var_array(std::string array_name = "") {
        // Find space to allocate array
        unsigned start_pos = 0;
        for (auto it = m_pos_to_var.begin(); it != m_pos_to_var.end() && it->first < start_pos + size; ++it)
            start_pos = it->first + 1;

        if (array_name.empty())
            array_name = "_" + std::to_string(start_pos);

        std::array<var_ptr, size> res;
        for (unsigned i = 0; i < size; ++i)
            res[i] = new_var(array_name + "_elem_" + std::to_string(i), 0, start_pos + i);

        return res;
    }

    void while_begin(const var&);
    void while_end(const var&);

    void if_begin(const var&);
    void else_begin();
    void if_end();

    void print(const std::string &text);

    friend std::ostream& operator<<(std::ostream&, const generator&);
    std::string get_code() const;
    std::string get_minimal_code() const;

private:
    // Helper function
    std::string move_sp_to(const var&);

    // Output format: sp moves, operations, comment, indentation
    typedef std::tuple<std::string, std::string, std::string, unsigned> output_t;
    std::vector<output_t> m_out;
    unsigned              m_indention = 0;
    unsigned              m_debug_nr = 0;

    // TODO: Is this map really needed?
    std::map<unsigned, var*>          m_pos_to_var;
    unsigned                          m_stackpos = 0;
    std::vector<std::vector<var_ptr>> m_else_if_stack;
};

class var {
    friend class generator;

public:
    ~var();

    void increment();
    void decrement();
    void set(unsigned);
    void add(unsigned);
    void subtract(unsigned);
    void multiply(unsigned);

    void read_input();
    void write_output() const;

    void move(var&);
    void copy(const var&);
    void add(const var&);
    void subtract(const var&);
    void multiply(const var&);
    void bool_not(const var&);

    void lower_than(const var&);
    void lower_equal(const var&);
    void greater_than(const var&);
    void greater_equal(const var&);
    void equal(const var&);
    void not_equal(const var&);

private:
    var(generator&, const std::string &var_name, unsigned stack_pos);
    var(const var&) = delete;

    generator         &m_gen;
    const std::string m_name;
    const unsigned    m_pos;
};

} // namespace
