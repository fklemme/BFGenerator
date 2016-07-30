#pragma once

#include "ast.h"

namespace bf {

class compiler {
public:
    compiler();

    // Compile source to Brainfuck code.
    std::string compile(const std::string &source) const;

    void enable_debug_output(bool);

private:
    std::string generate(const program_t &program) const;

    bool m_debug_output;
};

} // namespace bf
