#ifndef FRI_CXX_SOURCE_PARSER_HPP
#define FRI_CXX_SOURCE_PARSER_HPP

#include "abstract_code.hpp"

namespace fri
{
    class CxxSourceParser
    {
    public:
        auto parse_code (std::string const& code) -> TranslationUnit;
    };
}

#endif