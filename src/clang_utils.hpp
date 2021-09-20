#ifndef FRI_CLANG_UTILS_HPP
#define FRI_CLANG_UTILS_HPP

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/Tooling.h"

#include <memory>

#include "abstract_code.hpp"
#include "types.hpp"


namespace fri
{
    class ExpressionVisitor;

    auto extract_type        ( clang::PrintingPolicy const&
                             , clang::QualType
                             , ExpressionVisitor& ) -> uptr<Type>;
    auto switch_bin_operator (clang::BinaryOperatorKind) -> BinOpcode;
    auto switch_un_operator  (clang::UnaryOperatorKind)  -> UnOpcode;
}

#endif