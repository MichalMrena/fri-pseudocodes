#ifndef FRI_CLANG_UTILS_HPP
#define FRI_CLANG_UTILS_HPP

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/Tooling.h"

#include <memory>

#include "abstract_code.hpp"

namespace fri
{
    auto extract_type (clang::QualType qt) -> std::unique_ptr<Type>;
}

#endif