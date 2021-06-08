#ifndef FRI_CLANG_EXPRESSION_VISITOR_HPP
#define FRI_CLANG_EXPRESSION_VISITOR_HPP

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/Tooling.h"

#include "abstract_code.hpp"

#include <memory>

namespace fri
{
    class ExpressionVisitor : public clang::RecursiveASTVisitor<ExpressionVisitor>
    {
    public:
        auto release_expression   () -> std::unique_ptr<Expression>;

        auto VisitIntegerLiteral  (clang::IntegerLiteral*)  -> bool;
        auto VisitFloatingLiteral (clang::FloatingLiteral*) -> bool;
        auto VisitParenExpr       (clang::ParenExpr*)       -> bool;
        auto VisitBinaryOperator  (clang::BinaryOperator*)  -> bool;
        auto VisitDeclRefExpr     (clang::DeclRefExpr*)     -> bool;

    private:
        std::unique_ptr<Expression> expression_;
    };
}

#endif