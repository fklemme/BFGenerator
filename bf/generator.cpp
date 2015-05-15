#include "generator.h"

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace bf {

    std::shared_ptr<var> generator::new_var(std::string var_name, unsigned init_value, unsigned pref_stack_pos) {
        // Check variable name
        if (var_name.compare("") != 0) {
            // FK: No generic lambdas in C++11 :/
            if (std::any_of(m_pos_to_var.begin(), m_pos_to_var.end(),
                        [&var_name](decltype(*m_pos_to_var.begin()) &kv) {return var_name.compare(kv.second->m_name) == 0;}))
                throw std::logic_error("A variable named '" + var_name + "' already exists!");

            for (char c : var_name)
                if (std::find(bf_ops.begin(), bf_ops.end(), c) != bf_ops.end())
                    throw std::logic_error("Variable name must not contain brainfuck operators! (" + bf_ops + ")");
        }

        // Find free memory (to be optimized?)
        unsigned stack_pos = pref_stack_pos;
        while (m_pos_to_var.find(stack_pos) != m_pos_to_var.end())
            ++stack_pos;

        // Assign variable name, if not set
        if (var_name.compare("") == 0)
            var_name = "_" + std::to_string(stack_pos);

        m_out.emplace_back("", "", // NOP
                "(Debug) Declare variable '" + var_name + "' at position " + std::to_string(stack_pos),
                m_indention);
        
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
        if_var().copy(v);
        m_out.emplace_back(move_sp_to(if_var()),
                "[",
                "If '" + v.m_name + "' is not 0",
                m_indention++);
    }

    void generator::if_end(const var& v) {
        // Ensure leaving the 'if'
        if_var().set(0);
        m_out.emplace_back(move_sp_to(if_var()),
                "]",
                "End if '" + v.m_name + "'",
                --m_indention);
    }

    void generator::print(const std::string& text) {
        // Make Brainfuck-free text version for debug commentary
        std::string comment_text = text;
        for (char op : bf_ops)
            std::replace(comment_text.begin(), comment_text.end(), op, '_');
        std::replace(comment_text.begin(), comment_text.end(), '\n', '_');

        m_out.emplace_back("", "", // NOP
                "(Debug) Print '" + comment_text + "'",
                m_indention);

        // Print text (to be optimized?)
        auto pc = new_var("_print_char");
        for (char c : text) {
            pc->set(c);
            pc->write_output();
        }
    }

    std::ostream& operator<<(std::ostream& o, const generator& bf) {
        const unsigned indention_factor = 4;

        // Find good coloum width for formating
        std::vector<unsigned> col_sizes(3, 0);
        for (const auto& row : bf.m_out) {
            const unsigned indent = std::get<3>(row) * indention_factor;
            col_sizes[0] = std::max(col_sizes[0], (unsigned) std::get<0>(row).size()); // stack pointer move
            col_sizes[1] = std::max(col_sizes[1], (unsigned) std::get<1>(row).size() + indent); // operation
            col_sizes[2] = std::max(col_sizes[2], (unsigned) std::get<2>(row).size()); // comment
        }

        // Print code to stream
        o << std::left;
        for (const auto& row : bf.m_out) {
            const unsigned indent = std::get<3>(row) * indention_factor;
            o << std::setw(col_sizes[0]) << std::get<0>(row); // stack pointer move
            o << ' ' << std::setw(col_sizes[1]) << (std::string(indent, ' ') + std::get<1>(row)); // operation
            o << ' ' << std::get<2>(row) << '\n'; // comment
        }

        return o;
    }

    std::string generator::get_code() const {
        std::stringstream ss;
        ss << *this;
        return ss.str();
    }

    std::string generator::get_minimal_code() const {
        auto full_code = get_code();

        // Filter all non-Brainfuck characters
        std::string minimal_code;
        for (char c : full_code) {
            if (std::find(bf_ops.begin(), bf_ops.end(), c) != bf_ops.end()) {
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
        int dist = (int) v.m_pos - (int) m_stackpos;
        m_stackpos = v.m_pos;

        if (dist >= 0)
            return std::string(dist, '>');
        else
            return std::string(-dist, '<');
    }

    var& generator::if_var() {
        if (m_if_var == nullptr)
            m_if_var = new_var("_if_var");
        return *m_if_var;
    }

    void var::increment() {
        m_gen.m_out.emplace_back(m_gen.move_sp_to(*this),
                "+",
                "Increment '" + m_name + "'",
                m_gen.m_indention);
    }

    void var::decrement() {
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

    void var::subtract(unsigned value) {
        m_gen.m_out.emplace_back(m_gen.move_sp_to(*this),
                std::string(value, '-'),
                "Subtract " + std::to_string(value) + " from '" + m_name + "'",
                m_gen.m_indention);
    }

    void var::multiply(unsigned value) {
        m_gen.m_out.emplace_back("", "", // NOP
                "(Debug) Multiply '" + m_name + "' by " + std::to_string(value),
                m_gen.m_indention);

        auto temp = m_gen.new_var("_multiply");
        temp->move(*this);
        m_gen.while_begin(*temp);
        this->add(value);
        temp->decrement();
        m_gen.while_end(*temp);
    }

    void var::read_input() {
        m_gen.m_out.emplace_back(m_gen.move_sp_to(*this),
                ",",
                "Read input to '" + m_name + "'",
                m_gen.m_indention);
    }

    void var::write_output() const {
        m_gen.m_out.emplace_back(m_gen.move_sp_to(*this),
                ".",
                "Write output from '" + m_name + "'",
                m_gen.m_indention);
    }

    void var::move(var& v) {
        m_gen.m_out.emplace_back("", "", // NOP
                "(Debug) Move from '" + v.m_name + "' to '" + m_name + "'",
                m_gen.m_indention);

        this->set(0);
        m_gen.while_begin(v);
        this->increment();
        v.decrement();
        m_gen.while_end(v);
    }

    void var::copy(const var& v) {
        m_gen.m_out.emplace_back("", "", // NOP
                "(Debug) Copy from '" + v.m_name + "' to '" + m_name + "'",
                m_gen.m_indention);

        // Break v temporarily
        auto v_ptr = const_cast<var*>(&v);
        auto temp = m_gen.new_var("_copy");
        this->set(0);
        m_gen.while_begin(v);
        this->increment();
        temp->increment();
        v_ptr->decrement();
        m_gen.while_end(v);
        // Restore v
        v_ptr->move(*temp);
    }

    void var::add(const var& v) {
        m_gen.m_out.emplace_back("", "", // NOP
                "(Debug) Add '" + v.m_name + "' to '" + m_name + "'",
                m_gen.m_indention);

        // Break v temporarily
        auto v_ptr = const_cast<var*>(&v);
        auto temp = m_gen.new_var("_add");
        m_gen.while_begin(v);
        this->increment();
        temp->increment();
        v_ptr->decrement();
        m_gen.while_end(v);
        // Restore v
        v_ptr->move(*temp);
    }

    void var::subtract(const var& v) {
        m_gen.m_out.emplace_back("", "", // NOP
                "(Debug) Subtract '" + v.m_name + "' from '" + m_name + "'",
                m_gen.m_indention);

        // Break v temporarily
        auto v_ptr = const_cast<var*>(&v);
        auto temp = m_gen.new_var("_subtract");
        m_gen.while_begin(v);
        this->decrement();
        temp->increment();
        v_ptr->decrement();
        m_gen.while_end(v);
        // Restore v
        v_ptr->move(*temp);
    }

    void var::multiply(const var& v) {
        m_gen.m_out.emplace_back("", "", // NOP
                "(Debug) Multiply '" + v.m_name + "' with '" + m_name + "'",
                m_gen.m_indention);

        auto temp = m_gen.new_var("_multiply");
        temp->move(*this);
        m_gen.while_begin(*temp);
        this->add(v);
        temp->decrement();
        m_gen.while_end(*temp);
    }

    void var::bool_not(const var& v) {
        m_gen.m_out.emplace_back("", "", // NOP
                "(Debug) Set '" + m_name + "' to not '" + v.m_name + "'",
                m_gen.m_indention);
        this->set(1);
        m_gen.if_begin(v);
        this->set(0);
        m_gen.if_end(v);
    }

    void var::lower_than(const var& v) {
        m_gen.m_out.emplace_back("", "", // NOP
                "(Debug) Compare '" + m_name + "' lower than '" + v.m_name + "'",
                m_gen.m_indention);

        // Similar to http://stackoverflow.com/a/13327857
        // array = {1 (result), 1, 0, a, b, 0}
        //    pos: [0]         [1][2][3][4][5]
        auto array = m_gen.new_var_array<6>("_lower_than");
        array[0]->set(1);
        array[1]->set(1);
        array[3]->move(*this); // a ^= *this
        array[4]->copy(v);     // b ^= v

        m_gen.m_out.emplace_back(m_gen.move_sp_to(*array[3]),
                "+>+<"       // This is for managing if a = 0 and b = 0.
                "[->-[>]<<]" // If a is the one which reaches 0 first (a < b),
                             // then pointer will be at [3]. Else it will be at [2].
                "<[<->->]>", // If "else" (a >= b), set result at [0] to 0 and
                             // correct stack pointer position to [3] at the end.
                "Compare operation sequence for lower than",
                m_gen.m_indention);

        // Move result to *this
        this->move(*array[0]);
    }

    void var::lower_equal(const var& v) {
        // (this <= v) == (this < v + 1)
        auto v_1 = m_gen.new_var("_" + v.m_name + "_plus_1");
        v_1->copy(v);
        v_1->increment();
        this->lower_than(*v_1);
    }

    void var::greater_than(const var& v) {
        // (this > v) == (v < this)
        auto v_copy = m_gen.new_var("_" + v.m_name + "_copy");
        v_copy->copy(v);
        v_copy->lower_than(*this);
        this->move(*v_copy);
    }

    void var::greater_equal(const var& v) {
        // (this >= v) == (v <= this)
        auto v_copy = m_gen.new_var("_" + v.m_name + "_copy");
        v_copy->copy(v);
        v_copy->lower_equal(*this);
        this->move(*v_copy);
    }

    var::var(generator& gen, const std::string& var_name, unsigned stack_pos)
        : m_gen(gen), m_name(var_name), m_pos(stack_pos)
    {
        m_gen.m_pos_to_var.emplace(m_pos, this);
    }

    var::~var() {
        m_gen.m_pos_to_var.erase(m_pos);
    }

}
