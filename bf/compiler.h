#pragma once

#include <string>

namespace bf {

class compiler {
public:
    // Compile source to Brainfuck code.
    std::string compile(const std::string &source) const;
};

} // namespace bf
