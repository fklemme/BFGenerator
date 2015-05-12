#pragma once

#include <array>
#include <map>
#include <memory>
#include <ostream>
#include <string>
#include <tuple>
#include <vector>

namespace bfg {

    class brainfuck {
        friend class var;
        typedef std::shared_ptr<var> var_ptr;

    public:
        var_ptr new_var(std::string var_name = "", unsigned init_value = 0, unsigned pref_stack_pos = 0);

        template <unsigned size>
        std::array<var_ptr, size> new_var_array(std::string array_name = "") {
            unsigned start_pos = 0;
            for (auto it = m_pos_to_var.begin(); it != m_pos_to_var.end() && it->first <= start_pos + size; ++it)
                start_pos = it->first + 1;

            if (array_name.compare("") == 0)
                array_name = std::to_string(start_pos);

            std::array<var_ptr, size> res;
            for (unsigned i = 0; i < size; ++i)
                res[i] = new_var(array_name + "_elem_" + std::to_string(i), 0, start_pos + i);

            return res;
        }

        void while_begin(const var&);
        void while_end(const var&);

        void if_begin(const var&);
        void if_end(const var&);

        void print(const std::string& text);

        friend std::ostream& operator<<(std::ostream& o, const brainfuck& bf);
        std::string minimal_code() const;

    private:
        std::string move_sp_to(const var&);

        // Output format: moves, operations, comment, indentation
        std::vector<std::tuple<std::string, std::string, std::string, unsigned>> m_out;
        unsigned m_indention = 0;
        std::map<std::string, unsigned> m_var_to_pos;
        std::map<unsigned, std::string> m_pos_to_var;
        unsigned m_stackpos = 0;
        std::map<const var* const, var_ptr> m_if_var_backup;
    };

    class var {
        friend class brainfuck;

        var(brainfuck& bf, const std::string& var_name, unsigned stack_pos);
        var(const var&) = delete;

    public:
        ~var();

        void inc();
        void dec();
        void set(unsigned);
        void add(unsigned);
        void sub(unsigned);
        void mult(unsigned);

        void input();
        void output() const;

        void not(const var&);

        void move_to(var&);
        void move_to_both(var&, var&);
        void copy_to(var&) const;
        void add_to(var&) const;

        void lower_than(const var&);
        void lower_equal(const var&);
        void greater_than(const var&);
        void greater_equal(const var&);

    private:
        brainfuck& m_bf;
        const std::string m_name;
        const unsigned m_pos;
    };

}