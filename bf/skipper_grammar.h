#pragma once

#define BOOST_RESULT_OF_USE_DECLTYPE
#define BOOST_SPIRIT_USE_PHOENIX_V3

#include <boost/spirit/include/qi.hpp>

namespace bf {

namespace qi    = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;

template <typename iterator>
struct skipper_g : qi::grammar<iterator> {
    skipper_g() : skipper_g::base_type(skip) {
        skip      = ascii::space | comment | multiline;
        comment   = qi::lit("//") >> *(qi::char_ - qi::eol) >> qi::eol;
        multiline = qi::lit("/*") >> *(qi::char_ - "*/") >> "*/";
    }

    qi::rule<iterator> skip;
    qi::rule<iterator> comment;
    qi::rule<iterator> multiline;
};

} // namespace bf
