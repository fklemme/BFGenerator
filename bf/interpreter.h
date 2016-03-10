#pragma once

#include <deque>
#include <stdexcept>
#include <iterator>
#include <map>
#include <string>
#include <vector>

namespace bf {

template <typename memory_type = unsigned char, std::size_t memory_size = 128>
class interpreter {
public:
    interpreter(const std::string &program)
        : m_program(program), m_instruction_pointer(0),
        m_memory(memory_size), m_stack_pointer(0)
    {
        // Find matching brackets (loops)
        std::vector<std::size_t> loop_stack;
        for (std::size_t ip = 0; ip < m_program.size(); ++ip) {
            if (m_program.at(ip) == '[')
                loop_stack.push_back(ip);
            if (m_program.at(ip) == ']') {
                m_loop_forward.emplace(loop_stack.back(), ip);
                m_loop_back.emplace(ip, loop_stack.back());
                loop_stack.pop_back();
            }
        }
        // TODO: Check if loop_stack is completely unwinded.
    }

    void send_input(const std::vector<memory_type> &input) {
        std::copy(input.begin(), input.end(), std::back_inserter(m_input_buffer));
    }

    std::vector<memory_type> recv_output() const {
        std::vector<memory_type> result;
        std::swap(m_output_buffer, result);
        return result;
    }

    void run() {
        while (m_instruction_pointer < m_program.size()) {
            char op = m_program.at(m_instruction_pointer);
            switch (op) {
            case '>': ++m_stack_pointer;
                      break;
            case '<': --m_stack_pointer;
                      break;
            case '+': ++m_memory.at(m_stack_pointer);
                      break;
            case '-': --m_memory.at(m_stack_pointer);
                      break;
            case '.': m_output_buffer.push_back(m_memory.at(m_stack_pointer));
                      break;
            case ',': if (m_input_buffer.empty())
                          throw std::runtime_error("Waiting for input...");
                      m_memory.at(m_stack_pointer) = m_input_buffer.front();
                      m_input_buffer.pop_front();
                      break;
            case '[': if (m_memory.at(m_stack_pointer) == 0)
                          m_instruction_pointer = m_loop_forward.at(m_instruction_pointer);
                      break;
            case ']': if (m_memory.at(m_stack_pointer) != 0)
                          m_instruction_pointer = m_loop_back.at(m_instruction_pointer);
                      break;
            default:  break; // No Brainfuck operation
            }
            ++m_instruction_pointer;
        }
    }

    // Debug and testing
    const std::vector<memory_type> get_memory() const {
        return m_memory;
    }

    std::size_t get_stack_pointer() const {
        return m_stack_pointer;
    }

private:
    const std::string                  m_program;
    std::size_t                        m_instruction_pointer;
    std::vector<memory_type>           m_memory;
    std::size_t                        m_stack_pointer;
    std::map<std::size_t, std::size_t> m_loop_forward;
    std::map<std::size_t, std::size_t> m_loop_back;
    std::deque<memory_type>            m_input_buffer;
    mutable std::vector<memory_type>   m_output_buffer;
};

} // namespace
