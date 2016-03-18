#pragma once

#include <string>

namespace bf {

class compiler {
public:
    std::string compile(const std::string &source) const;
};

} // namespace
