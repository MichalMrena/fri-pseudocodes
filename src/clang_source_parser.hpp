#ifndef FRI_CLANG_SOURCE_PARSER_HPP
#define FRI_CLANG_SOURCE_PARSER_HPP

#include "abstract_code.hpp"

#include <string>

namespace fri
{
    /**
     *  @brief Our function that interacts with the clang black magic.
     */
    auto extract_code (std::string const& code) -> TranslationUnit;
}

#endif