#include "generator.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace bf {

std::shared_ptr<var> generator::new_var(std::string var_name, unsigned init_value, unsigned pref_stack_pos) {
    // Check variable name for Brainfuck operators
    if (var_name.compare("") != 0) {
        for (char c : var_name)
            if (std::find(bf_ops.begin(), bf_ops.end(), c) != bf_ops.end())
                throw std::logic_error("Variable name must not contain brainfuck operators!"
                                       " (" + var_name + ")");
    }

    // Find free memory (to be optimized?)
    unsigned stack_pos = pref_stack_pos;
    while (m_pos_to_var.find(stack_pos) != m_pos_to_var.end())
        ++stack_pos;

    // Assign variable name, if not set
    if (var_name.compare("") == 0)
        var_name = "_mem_addr_" + std::to_string(stack_pos);

    m_out.emplace_back("", "", // NOP
            "(Debug " + std::to_string(m_debug_nr++) + ")"
            " Declare variable '" + var_name + "' at position " + std::to_string(stack_pos),
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

void generator::if_begin(const var &v) {
    auto if_else = new_var_array<3>("_if_else_" + v.m_name);
    if_else[1]->set(1);
    if_else[2]->copy(v);
    // Do a quick, 'not'-like operation to set if/else values.
    m_out.emplace_back(move_sp_to(*if_else[2]),
            "[<<+>->[-]]", // If (v > 0) change {0, 1, v} to {1, 0, 0}.
            "Initialize if/else values for '" + v.m_name + "'",
            m_indention);
    m_else_if_stack.push_back({if_else[1], if_else[0]}); // Note the inverse order of if/else!

    m_out.emplace_back(move_sp_to(*if_else[0]),
            "[",
            "If '" + if_else[0]->m_name + "' is not 0",
            m_indention++);
}

void generator::else_begin() {
    if (m_else_if_stack.empty())
        throw std::logic_error("Else without if!");

    auto else_if = m_else_if_stack.back();
    assert(else_if.size() == 2); // Double 'else_begin'?
    m_else_if_stack.back().pop_back(); // Pop 'if' from stack, exposing 'else' for 'end_if'

    // Ensure leaving the if
    else_if[1]->set(0);
    m_out.emplace_back(move_sp_to(*else_if[1]),
            "]",
            "End if '" + else_if[1]->m_name + "'",
            --m_indention);

    m_out.emplace_back(move_sp_to(*else_if[0]),
            "[",
            "Else '" + else_if[0]->m_name + "' is not 0",
            m_indention++);
}

void generator::if_end() {
    if (m_else_if_stack.empty())
        throw std::logic_error("End if without if!");
    auto if_or_else = m_else_if_stack.back().back();

    // Ensure leaving the if/else
    if_or_else->set(0);
    m_out.emplace_back(move_sp_to(*if_or_else),
            "]",
            "End if/else '" + if_or_else->m_name + "'",
            --m_indention);
    m_else_if_stack.pop_back();
}

void generator::print(const std::string &text) {
    // Make Brainfuck-free text version for debug commentary
    std::string comment_text = text;
    for (char op : bf_ops)
        std::replace(comment_text.begin(), comment_text.end(), op, '_');
    std::replace(comment_text.begin(), comment_text.end(), '\n', '_');

    m_out.emplace_back("", "", // NOP
            "(Debug " + std::to_string(m_debug_nr++) + ") Print '" + comment_text + "'",
            m_indention);

    // Print text (to be optimized?)
    auto pc = new_var_array<2>("_print");
    for (unsigned char c : text) {
        if (c < 32)
            pc[0]->set(c);
        else {
            unsigned f = (unsigned) std::sqrt(c);
            m_out.emplace_back(move_sp_to(*pc[0]),
                    "[-]>" + std::string(c / f, '+') +
                    "[<" + std::string(f, '+') + ">-]<" + std::string(c % f, '+'),
                    "Operation sequence to set '" + pc[0]->m_name + "'"
                    " to " + std::to_string((unsigned) c),
                    m_indention);
        }
        pc[0]->write_output();
    }
}

std::ostream &operator<<(std::ostream &o, const generator &g) {
    const unsigned indention_factor = 4;

    // Find good coloum width for formating
    std::vector<unsigned> col_sizes(3, 0);
    for (const auto& row : g.m_out) {
        const unsigned indent = std::get<3>(row) * indention_factor;
        col_sizes[0] = std::max(col_sizes[0], (unsigned) std::get<0>(row).size()); // stack pointer move
        col_sizes[1] = std::max(col_sizes[1], (unsigned) std::get<1>(row).size() + indent); // operation
        col_sizes[2] = std::max(col_sizes[2], (unsigned) std::get<2>(row).size()); // comment
    }

    // Print code to stream
    o << std::left;
    for (const auto& row : g.m_out) {
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

std::string generator::move_sp_to(const var &v) {
    int dist = (int) v.m_pos - (int) m_stackpos;
    m_stackpos = v.m_pos;

    if (dist >= 0)
        return std::string(dist, '>');
    else
        return std::string(-dist, '<');
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
            "(Debug " + std::to_string(m_gen.m_debug_nr++) + ")"
            " Multiply '" + m_name + "' by " + std::to_string(value),
            m_gen.m_indention);

    auto temp = m_gen.new_var("_multiply");
    temp->move(*this);
    m_gen.while_begin(*temp);
    {
        this->add(value);
        temp->decrement();
    }
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

void var::move(var &v) {
    if (&v == this)
        return;

    m_gen.m_out.emplace_back("", "", // NOP
            "(Debug " + std::to_string(m_gen.m_debug_nr++) + ")"
            " Move from '" + v.m_name + "' to '" + m_name + "'",
            m_gen.m_indention);

    this->set(0);
    m_gen.while_begin(v);
    {
        this->increment();
        v.decrement();
    }
    m_gen.while_end(v);
}

void var::copy(const var &v) {
    if (&v == this)
        return;

    m_gen.m_out.emplace_back("", "", // NOP
            "(Debug " + std::to_string(m_gen.m_debug_nr++) + ")"
            " Copy from '" + v.m_name + "' to '" + m_name + "'",
            m_gen.m_indention);

    // Break v temporarily
    auto v_ptr = const_cast<var*>(&v);
    auto temp = m_gen.new_var("_copy");
    this->set(0);
    m_gen.while_begin(v);
    {
        this->increment();
        temp->increment();
        v_ptr->decrement();
    }
    m_gen.while_end(v);
    // Restore v
    v_ptr->move(*temp);
}

void var::add(const var &v) {
    m_gen.m_out.emplace_back("", "", // NOP
            "(Debug " + std::to_string(m_gen.m_debug_nr++) + ")"
            " Add '" + v.m_name + "' to '" + m_name + "'",
            m_gen.m_indention);

    auto temp = m_gen.new_var("_add");
    if (&v != this) {
        // Break v temporarily
        auto v_ptr = const_cast<var*>(&v);
        m_gen.while_begin(v);
        {
            this->increment();
            temp->increment();
            v_ptr->decrement();
        }
        m_gen.while_end(v);
        // Restore v
        v_ptr->move(*temp);
    } else {
        temp->move(*this);
        m_gen.while_begin(*temp);
        {
            this->increment();
            this->increment();
            temp->decrement();
        }
        m_gen.while_end(*temp);
    }
}

void var::subtract(const var &v) {
    m_gen.m_out.emplace_back("", "", // NOP
            "(Debug " + std::to_string(m_gen.m_debug_nr++) + ")"
            " Subtract '" + v.m_name + "' from '" + m_name + "'",
            m_gen.m_indention);

    if (&v != this) {
        // Break v temporarily
        auto v_ptr = const_cast<var*>(&v);
        auto temp = m_gen.new_var("_subtract");
        m_gen.while_begin(v);
        {
            this->decrement();
            temp->increment();
            v_ptr->decrement();
        }
        m_gen.while_end(v);
        // Restore v
        v_ptr->move(*temp);
    } else
        this->set(0);
}

void var::multiply(const var &v) {
    m_gen.m_out.emplace_back("", "", // NOP
            "(Debug " + std::to_string(m_gen.m_debug_nr++) + ")"
            " Multiply '" + v.m_name + "' with '" + m_name + "'",
            m_gen.m_indention);

    auto temp = m_gen.new_var("_multiply");
    if (&v != this) {
        temp->move(*this);
        m_gen.while_begin(*temp);
        {
            this->add(v);
            temp->decrement();
        }
        m_gen.while_end(*temp);
    } else {
        temp->move(*this);
        auto temp2 = m_gen.new_var("_multiply_2");
        temp2->copy(*temp);
        m_gen.while_begin(*temp);
        {
            this->add(*temp2);
            temp->decrement();
        }
        m_gen.while_end(*temp);
    }
}

void var::bool_not(const var &v) {
    m_gen.m_out.emplace_back("", "", // NOP
            "(Debug " + std::to_string(m_gen.m_debug_nr++) + ")"
            " Set '" + m_name + "' to not '" + v.m_name + "'",
            m_gen.m_indention);

    // array = {1 (result), a}
    auto array = m_gen.new_var_array<2>("_not");
    array[0]->set(1);
    if (&v != this)
        array[1]->copy(v);
    else
        array[1]->move(*this);

    m_gen.m_out.emplace_back(m_gen.move_sp_to(*array[1]),
            "[<->[-]]", // If (a > 0), set result to 0 and clear a.
            "Operation sequence for 'not'",
            m_gen.m_indention);

    // Move result to *this
    this->move(*array[0]);
}

void var::bool_and(const var &v) {
    m_gen.m_out.emplace_back("", "", // NOP
            "(Debug " + std::to_string(m_gen.m_debug_nr++) + ")"
            " Set '" + m_name + "' to '" + m_name + "' and '" + v.m_name + "'",
            m_gen.m_indention);

    if (&v != this) {
        // array = {0 (result), a, b}
        auto array = m_gen.new_var_array<3>("_and");
        array[0]->set(0);
        array[1]->move(*this);
        array[2]->copy(v);

        m_gen.m_out.emplace_back(m_gen.move_sp_to(*array[1]),
                "[>[<<+>>[-]]<[-]]", // If (a > 0), check if (b > 0)
                // and if true set result to 1, then clear b and a.
                "Operation sequence for 'and'",
                m_gen.m_indention);

        // Move result to *this
        this->move(*array[0]);
    } else {
        // array = {0 (result), a}
        auto array = m_gen.new_var_array<2>("_and");
        array[0]->set(0);
        array[1]->move(*this);

        m_gen.m_out.emplace_back(m_gen.move_sp_to(*array[1]),
                "[<+>[-]]", // If (a > 0), set result to 1 and clear a.
                "Operation sequence for 'and'",
                m_gen.m_indention);

        // Move result to *this
        this->move(*array[0]);
    }
}

void var::bool_or(const var &v) {
    assert(0); // TODO: Implement
}

void var::lower_than(const var &v) {
    assert(&v != this); // TODO: Implement this case?
    m_gen.m_out.emplace_back("", "", // NOP
            "(Debug " + std::to_string(m_gen.m_debug_nr++) + ")"
            " Compare '" + m_name + "' lower than '" + v.m_name + "'",
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
            "<[<->>]>",  // If "else" (a >= b), set result at [0] to 0 and
                         // correct stack pointer position to [3] at the end.
            "Compare operation sequence for 'lower than'",
            m_gen.m_indention);

    // Move result to *this
    this->move(*array[0]);
}

void var::lower_equal(const var &v) {
    // (this <= v) == (this < v + 1)
    auto v_1 = m_gen.new_var("_1_plus_" + v.m_name);
    v_1->copy(v);
    v_1->increment();
    this->lower_than(*v_1);
}

void var::greater_than(const var &v) {
    // (this > v) == (v < this)
    auto v_copy = m_gen.new_var("_copy_" + v.m_name);
    v_copy->copy(v);
    v_copy->lower_than(*this);
    this->move(*v_copy);
}

void var::greater_equal(const var &v) {
    // (this >= v) == (v <= this)
    auto v_copy = m_gen.new_var("_copy_" + v.m_name);
    v_copy->copy(v);
    v_copy->lower_equal(*this);
    this->move(*v_copy);
}

void var::equal(const var &v) {
    assert(&v != this); // TODO: Implement this case?
    m_gen.m_out.emplace_back("", "", // NOP
            "(Debug " + std::to_string(m_gen.m_debug_nr++) + ")"
            " Compare '" + m_name + "' equal to '" + v.m_name + "'",
            m_gen.m_indention);

    // Similar to http://stackoverflow.com/a/13327857
    // array = {0 (result), 1, 0, a, b, 0}
    //    pos: [0]         [1][2][3][4][5]
    auto array = m_gen.new_var_array<6>("_equal");
    array[1]->set(1);
    array[3]->move(*this); // a ^= *this
    array[4]->copy(v);     // b ^= v

    m_gen.m_out.emplace_back(m_gen.move_sp_to(*array[3]),
            "+>+<"         // This is for managing if a = 0 and b = 0.
            "[->-[>]<<]"   // If a is the one which reaches 0 first (a < b),
                           // then pointer will be at [3]. Else it will be at [2].
            "<["           // If "else" (a >= b)...
            "<+>>>"        // ...set result at [0] to 1 at first (expecting a = b)
            "[<<<->>>[-]]" // ...and reset result to 0 if a > 0
            "<]>",         // Correct stack pointer position to [3] at the end.
            "Compare operation sequence for 'equal'",
            m_gen.m_indention);

    // Move result to *this
    this->move(*array[0]);
}

void var::not_equal(const var &v) {
    assert(&v != this); // TODO: Implement this case?
    auto temp = m_gen.new_var("_equal");
    temp->copy(*this);
    temp->equal(v);
    this->bool_not(*temp);
}

var::var(generator &gen, const std::string &var_name, unsigned stack_pos)
    : m_gen(gen), m_name(var_name), m_pos(stack_pos)
{
    m_gen.m_pos_to_var.emplace(m_pos, this);
}

var::~var() {
    m_gen.m_pos_to_var.erase(m_pos);
}

} // namespace
