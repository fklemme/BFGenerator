#include "compiler.h"

#include <boost/spirit/include/qi.hpp>

#include <iostream> // debug outputs

namespace bf {

namespace qi    = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;

template <typename iterator>
struct grammar : qi::grammar<iterator, std::string(), ascii::space_type> {
    parser() : parser::base_type(start) {
        start = *qi::char_;
    }

    qi::rule<iterator, std::string(), ascii::space_type> start;
};

std::string compiler::compile(const std::string &source) {
    grammar<decltype(source.begin())> g;
    std::string ast;
    const bool success = qi::phrase_parse(source.begin(), source.end(), g, ascii::space, ast);

    std::cout << "Parse result: " << (success ? "true" : "false") << std::endl;
    std::cout << "AST: " << ast << std::endl;

    return "TODO";
}

} // namespace
