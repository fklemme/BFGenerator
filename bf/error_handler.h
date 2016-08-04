#pragma once

#include <iostream>
#include <string>

namespace bf {

template <typename iterator>
struct error_handler {
    template <typename what_t>
    void operator()(const iterator &first, const iterator &last, const iterator &err, const what_t& what) const {
        std::string before(first, err);
        std::size_t bpos = before.find_last_of('\n');
        if (bpos != std::string::npos)
            before = before.substr(bpos + 1);

        std::string after(err, last);
        std::size_t apos = after.find_first_of('\n');
        if (apos != std::string::npos)
            after = after.substr(0, apos);

        std::cerr << "Error! Expecting " << what << " here:\n"
            << before << after << '\n'
            << std::string(before.size(), ' ') << '^'
            << std::endl;
    }
};

} // namespace bf
