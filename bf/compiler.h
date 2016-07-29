#pragma once

#include "ast.h"

namespace bf {

class compiler {
public:
    // Compile source to Brainfuck code.
    std::string compile(const std::string &source) const;

private:
    std::string generate(const program_t &program) const;
};

} // namespace bf
