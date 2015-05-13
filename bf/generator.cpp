#include "generator.h"

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace bf {

    std::shared_ptr<var> generator::new_var(std::string var_name, unsigned init_value, unsigned pref_stack_pos) {
        // Check variable name
        if (var_name.compare("") != 0) {
            if (m_var_to_pos.find(var_name) != m_var_to_pos.end())
                throw std::logic_error("A variable named '" + var_name + "' already exists!");

            const std::string bf_instructions = "><+-.,[]";
            for (char c : var_name)
                if (std::find(bf_instructions.begin(), bf_instructions.end(), c) != bf_instructions.end())
                    throw std::logic_error("Variable name must not contain brainfuck operators! (" + bf_instructions + ")");
        }

        // Find free memory (to be optimized?)
        unsigned stack_pos = pref_stack_pos;
        while (m_pos_to_var.find(stack_pos) != m_pos_to_var.end())
            ++stack_pos;

        // Assign variable name, if not set
        if (var_name.compare("") == 0)
            var_name = "_" + std::to_string(stack_pos);

        m_out.emplace_back("", "", // NOP
                "Declare variable '" + var_name + "' at position " + std::to_string(stack_pos),
                m_indention);
        m_var_to_pos.emplace(var_name, stack_pos);
        m_pos_to_var.emplace(stack_pos, var_name);
        
        auto new_var = std::shared_ptr<var>(new var(*this, var_name, stack_pos));
        new_var->set(init_value);
        return new_var;
    }

    void generator::while_begin(const var &v) {
        m_out.emplace_back(move_sp_to(v),
                "[",
                "While '" + v.m_name + "' is not 0",
                m_indention++);
    }

    void generator::while_end(const var &v) {
        m_out.emplace_back(move_sp_to(v),
                "]",
                "End while '" + v.m_name + "'",
                --m_indention);
    }

    void generator::if_begin(const var& v) {
        m_out.emplace_back(move_sp_to(v),
                "[",
                "If '" + v.m_name + "' is not 0",
                m_indention++);
    }

    void generator::if_end(const var& v) {
        // Ensure leaving the 'if'
        const auto null = new_var();
        m_out.emplace_back(move_sp_to(*null),
                "]",
                "End if '" + v.m_name + "'",
                --m_indention);
    }

    void generator::print(const std::string& text) {
        const std::string bf_instructions = "><+-.,[]";
        std::string comment_text = text;
        for (char op : bf_instructions)
            std::replace(comment_text.begin(), comment_text.end(), op, '_');
        std::replace(comment_text.begin(), comment_text.end(), '\n', '_');

        m_out.emplace_back("", "", // NOP
                "Print '" + comment_text + "'",
                m_indention);

        auto temp = new_var("_print_char");
        for (unsigned char c : text) {
            temp->set(c);
            temp->output();
        }
    }

    std::ostream& operator<<(std::ostream& o, const generator& bf) {
        const unsigned indention_factor = 4;

        std::vector<unsigned> col_sizes(3, 0);
        for (const auto& row : bf.m_out) {
            const unsigned indent = std::get<3>(row) * indention_factor;
            col_sizes[0] = std::max(col_sizes[0], (unsigned) std::get<0>(row).size()); // sp move
            col_sizes[1] = std::max(col_sizes[1], (unsigned) std::get<1>(row).size() + indent); // operation
            col_sizes[2] = std::max(col_sizes[2], (unsigned) std::get<2>(row).size()); // comment
        }

        o << std::left;
        for (const auto& row : bf.m_out) {
            const unsigned indent = std::get<3>(row) * indention_factor;
            o << std::setw(col_sizes[0]) << std::get<0>(row); // sp move
            o << ' ' << std::setw(col_sizes[1]) << (std::string(indent, ' ') + std::get<1>(row)); // operation
            o << ' ' << std::get<2>(row) << '\n'; // comment
        }

        return o;
    }

    std::string generator::minimal_code() const {
        std::stringstream ss;
        ss << *this;
        auto full_code = ss.str();

        std::string minimal_code, bf_instructions = "><+-.,[]";
        for (char c : full_code) {
            if (std::find(bf_instructions.begin(), bf_instructions.end(), c) != bf_instructions.end()) {
                minimal_code += c;
                if (minimal_code.size() % 80 == 0)
                    minimal_code += '\n';
            }
        }

        // Make it look nice :)
        unsigned open_chars = 80 - (minimal_code.size() % 80);
        if (open_chars < 8)
            minimal_code += std::string(open_chars, '+');
        else
            minimal_code += "[-]" + std::string(open_chars - 6, '+') + "[-]";

        return minimal_code;
    }

    std::string generator::move_sp_to(const var& v) {
        const auto var_pos = m_var_to_pos.at(v.m_name);
        int dist = (int) m_var_to_pos.at(v.m_name) - (int) m_stackpos;
        m_stackpos = var_pos;

        if (dist >= 0)
            return std::string(dist, '>');
        else
            return std::string(-dist, '<');
    }

    var::var(generator& bf, const std::string& var_name, unsigned stack_pos)
        : m_gen(bf), m_name(var_name), m_pos(stack_pos) {}

    var::~var() {
        m_gen.m_var_to_pos.erase(m_name);
        m_gen.m_pos_to_var.erase(m_pos);
    }

    void var::inc() {
        m_gen.m_out.emplace_back(m_gen.move_sp_to(*this),
            "+",
            "Increment '" + m_name + "'",
            m_gen.m_indention);
    }

    void var::dec() {
        m_gen.m_out.emplace_back(m_gen.move_sp_to(*this),
            "-",
            "Decrement '" + m_name + "'",
            m_gen.m_indention);
    }

    void var::set(unsigned value) {
        m_gen.m_out.emplace_back(m_gen.move_sp_to(*this),
            "[-]" + std::string(value, '+'),
            "Set '" + m_name + "' to " + std::to_string(value),
            m_gen.m_indention);
    }

    void var::add(unsigned value) {
        m_gen.m_out.emplace_back(m_gen.move_sp_to(*this),
            std::string(value, '+'),
            "Add " + std::to_string(value) + " to '" + m_name + "'",
            m_gen.m_indention);
    }

    void var::sub(unsigned value) {
        m_gen.m_out.emplace_back(m_gen.move_sp_to(*this),
            std::string(value, '-'),
            "Subtract " + std::to_string(value) + " from '" + m_name + "'",
            m_gen.m_indention);
    }

    void var::mult(unsigned value) {
        m_gen.m_out.emplace_back("", "", // NOP
                "Multiply '" + m_name + "' by " + std::to_string(value),
                m_gen.m_indention);
        auto temp = m_gen.new_var("_temp_mult");
        this->move_to(*temp);
        m_gen.while_begin(*temp);
        this->add(value);
        temp->dec();
        m_gen.while_end(*temp);
    }

    void var::input() {
        m_gen.m_out.emplace_back(m_gen.move_sp_to(*this),
            ",",
            "Read input to '" + m_name + "'",
            m_gen.m_indention);
    }

    void var::output() const {
        m_gen.m_out.emplace_back(m_gen.move_sp_to(*this),
            ".",
            "Write output from '" + m_name + "'",
            m_gen.m_indention);
    }

    void var::negate(const var& v) {
        m_gen.m_out.emplace_back("", "", // NOP
                "Set '" + m_name + "' to not '" + v.m_name + "'",
                m_gen.m_indention);
        this->set(1);
        m_gen.if_begin(v);
        this->set(0);
        m_gen.if_end(v);
    }

    void var::move_to(var& v) {
        m_gen.m_out.emplace_back("", "", // NOP
                "Move '" + m_name + "' to '" + v.m_name + "'",
                m_gen.m_indention);
        v.set(0);
        m_gen.while_begin(*this);
        v.inc();
        this->dec();
        m_gen.while_end(*this);
    }

    void var::move_to_both(var& v1, var& v2) {
        m_gen.m_out.emplace_back("", "", // NOP
                "Move '" + m_name + "' to '" + v1.m_name + "' and to '" + v2.m_name + "'",
                m_gen.m_indention);
        v1.set(0);
        v2.set(0);
        m_gen.while_begin(*this);
        v1.inc();
        v2.inc();
        this->dec();
        m_gen.while_end(*this);
    }

    void var::copy_to(var& v) const {
        auto t_ptr = const_cast<var*>(this);
        m_gen.m_out.emplace_back("", "", // NOP
                "Copy '" + m_name + "' to '" + v.m_name + "'",
                m_gen.m_indention);
        auto temp = m_gen.new_var("_temp_copy_to");
        t_ptr->move_to_both(v, *temp);
        // restore *this
        temp->move_to(*t_ptr);
    }

    void var::add_to(var& v) const {
        auto t_ptr = const_cast<var*>(this);
        m_gen.m_out.emplace_back("", "", // NOP
                "Add '" + m_name + "' to '" + v.m_name + "'",
                m_gen.m_indention);
        auto temp = m_gen.new_var("_temp_add_to");
        // like move_to_both but without clearing targets
        m_gen.while_begin(*this);
        v.inc();
        temp->inc();
        t_ptr->dec();
        m_gen.while_end(*this);
        // restore *this
        temp->move_to(*t_ptr);
    }

    void var::lower_than(const var& v) {
        m_gen.m_out.emplace_back("", "", // NOP
                "Compare '" + m_name + "' lower than '" + v.m_name + "'",
                m_gen.m_indention);

        // Similar to http://stackoverflow.com/a/13327857
        // array = {1 (result), 1, 0, a, b, 0}
        //    pos: [0]         [1][2][3][4][5]
        auto array = m_gen.new_var_array<6>("_temp_lt");
        array[0]->set(1);
        array[1]->set(1);
        this->move_to(*array[3]); // this -> a
        v.copy_to(*array[4]); // v -> b

        m_gen.m_out.emplace_back(m_gen.move_sp_to(*array[3]),
            "+>+<" // This is for managing if a = 0 and b = 0.
            "[->-[>]<<]" // If a is the one which reaches 0 first (a < b), then pointer will be at [3]. Else it will be at [2].
            "<[<->->]>", // If "else" (a >= b), set result at [0] to 0 and correct stack pointer position to [3] at the end.
            "Compare operation sequence for lower than", // comment
            m_gen.m_indention);

        // Move result to *this
        array[0]->move_to(*this);
    }

    void var::lower_equal(const var& v) {
        // (this <= v) == (this < v + 1)
        auto v_1 = m_gen.new_var("_" + v.m_name + "_plus_1");
        v.copy_to(*v_1);
        v_1->inc();
        this->lower_than(*v_1);
    }

    void var::greater_than(const var& v) {
        auto v_copy = m_gen.new_var("_" + v.m_name + "_copy");
        v.copy_to(*v_copy);
        // (this > v) == (v < this)
        v_copy->lower_than(*this);
        this->set(0);
        v_copy->move_to(*this);
    }

    void var::greater_equal(const var& v) {
        auto v_copy = m_gen.new_var("_" + v.m_name + "_copy");
        v.copy_to(*v_copy);
        // (this >= v) == (v <= this)
        v_copy->lower_equal(*this);
        this->set(0);
        v_copy->move_to(*this);
    }

}
