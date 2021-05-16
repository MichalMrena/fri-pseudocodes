#include "cxx_source_parser.hpp"

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/Tooling.h"

namespace fri
{
    auto CxxSourceParser::parse_code
        (std::string const& code) -> TranslationUnit
    {
        return TranslationUnit(std::vector<Class>());
    }
}